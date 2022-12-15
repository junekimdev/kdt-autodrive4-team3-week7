#include <iostream>

#include "opencv2/opencv.hpp"

// Error codes
constexpr int ERROR_LOADING_VIDEO = 1 << 0;

// constants
constexpr char VIDEO_FILENAME[] = "Sub_project.avi";
constexpr int MIN_VAL = 0;
constexpr int MAX_VAL = 640;
constexpr int SCAN_OFFSET = 400;
constexpr double GAUSIAN_BLUR_SIGMA = 1.;
constexpr int ROI_HEIGHT = 20;
constexpr int ROI_Y = SCAN_OFFSET - (ROI_HEIGHT / 2);
const cv::Scalar BLUE = cv::Scalar(255, 0, 0);
const cv::Scalar YELLOW = cv::Scalar(0, 255, 255);

std::vector<cv::Point2f> find_edges(const cv::Mat &img, bool isLeft = true) {
  cv::Mat img32, blr, dx;
  img.convertTo(img32, CV_32F);
  cv::GaussianBlur(img32, blr, cv::Size(), GAUSIAN_BLUR_SIGMA);
  cv::Sobel(blr, dx, CV_32F, 1, 0);

  double minv, maxv;
  cv::Point minloc, maxloc;

  int y2 = img.rows / 2;     // horizontal center line
  cv::Mat roi = dx.row(y2);  // Line scanning
  cv::minMaxLoc(roi, &minv, &maxv, &minloc, &maxloc);

  // Find false positives
  if (isLeft) {
    if (maxloc.x < minloc.x) minloc.x = MIN_VAL;
  } else {
    if (maxloc.x < minloc.x) maxloc.x = MAX_VAL;
  }

  // std::cout << maxP << ' ' << minP << '\n';
  return {maxloc, minloc};
}

int main() {
  int returnCode = 0;

  cv::VideoCapture video = cv::VideoCapture(VIDEO_FILENAME);
  if (!video.isOpened()) {
    std::cerr << "Failed to load the video" << '\n';
    returnCode |= ERROR_LOADING_VIDEO;
  }
  if (returnCode) return returnCode;

  while (1) {
    cv::Mat videoFrame;
    video >> videoFrame;
    if (videoFrame.empty()) {
      std::cout << "The END of the video; BYE!" << '\n';
      break;
    }
    int width = videoFrame.cols / 2;

    // Convert to Gray
    cv::Mat grayFrame;
    cv::cvtColor(videoFrame, grayFrame, cv::COLOR_BGR2GRAY);

    // Find lanes
    cv::Rect roiRectL(0, ROI_Y, width - 1, ROI_HEIGHT);      // left half
    cv::Rect roiRectR(width, ROI_Y, width - 1, ROI_HEIGHT);  // right half
    cv::Mat roiL = grayFrame(roiRectL);
    cv::Mat roiR = grayFrame(roiRectR);
    std::vector<cv::Point2f> ptsL = find_edges(roiL);
    std::vector<cv::Point2f> ptsR = find_edges(roiR, false);

    // Display
    cv::drawMarker(videoFrame, cv::Point(cvRound(ptsL[0].x), SCAN_OFFSET),
                   YELLOW, cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
    cv::drawMarker(videoFrame, cv::Point(cvRound(ptsL[1].x), SCAN_OFFSET), BLUE,
                   cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
    cv::drawMarker(videoFrame,
                   cv::Point(cvRound(ptsR[0].x) + width, SCAN_OFFSET), YELLOW,
                   cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
    cv::drawMarker(videoFrame,
                   cv::Point(cvRound(ptsR[1].x) + width, SCAN_OFFSET), BLUE,
                   cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);

    cv::line(videoFrame, cv::Point(0, SCAN_OFFSET),
             cv::Point(videoFrame.cols, SCAN_OFFSET), BLUE);
    cv::imshow("video", videoFrame);
    int k = cv::waitKey(1);
    if (k == 27 || k == ' ') break;
  }

  video.release();
  cv::destroyAllWindows();
  return returnCode;
}