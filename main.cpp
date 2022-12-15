#include <iostream>

#include "opencv2/opencv.hpp"

// Error codes
constexpr int ERROR_LOADING_VIDEO = 1 << 0;

// constants
constexpr char VIDEO_FILENAME[] = "Sub_project.avi";
constexpr int SCAN_OFFSET = 400;
constexpr double GAUSIAN_BLUR_SIGMA = 2.;
constexpr int ROI_HEIGHT = 20;
constexpr int ROI_Y = SCAN_OFFSET - (ROI_HEIGHT / 2);
const cv::Scalar BLUE = cv::Scalar(255, 0, 0);
const cv::Scalar YELLOW = cv::Scalar(0, 255, 255);

std::vector<int> filterX(const std::vector<cv::Point>& pts, const int minV,
                         const int maxV, const bool isLeft = true) {
  int lp = pts[0].x, rp = pts[1].x;

  // Offset rightside
  if (!isLeft) {
    lp += minV;
    rp += minV;
  }

  int distance = rp - lp;
  if (distance < 1) {
    lp = isLeft ? minV : maxV;
    rp = isLeft ? minV : maxV;
  }

  return {lp, rp};
}

std::vector<cv::Point> find_edges(const cv::Mat& img,
                                  const bool isLeft = true) {
  cv::Mat img32, blr, dx;
  img.convertTo(img32, CV_32F);
  cv::GaussianBlur(img32, blr, cv::Size(), GAUSIAN_BLUR_SIGMA);
  cv::Sobel(blr, dx, CV_32F, 1, 0);

  double leftsideV, rightsideV;
  cv::Point leftsidePt, rightsidePt;

  int centerY = ROI_HEIGHT / 2;   // horizontal center line
  cv::Mat roi = dx.row(centerY);  // Line scanning
  cv::minMaxLoc(roi, &leftsideV, &rightsideV, &leftsidePt, &rightsidePt);

  return {leftsidePt, rightsidePt};
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
    std::vector<cv::Point> ptsL = find_edges(roiL);
    std::vector<cv::Point> ptsR = find_edges(roiR, false);
    std::vector<int> pxL = filterX(ptsL, 0, width);
    std::vector<int> pxR = filterX(ptsR, width, videoFrame.cols - 1, false);

    std::cout << pxL[0] << ' ' << pxL[1] << " | ";
    std::cout << pxR[0] << ' ' << pxR[1] << '\n';

    // Display
    cv::drawMarker(videoFrame, cv::Point(pxL[0], SCAN_OFFSET), YELLOW,
                   cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
    cv::drawMarker(videoFrame, cv::Point(pxL[1], SCAN_OFFSET), BLUE,
                   cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
    cv::drawMarker(videoFrame, cv::Point(pxR[0], SCAN_OFFSET), YELLOW,
                   cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
    cv::drawMarker(videoFrame, cv::Point(pxR[1], SCAN_OFFSET), BLUE,
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