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

float getSubPixelX(const cv::Mat &roi, const cv::Point &loc) {
  // Get 3 values of [-1, 0, +1] from loc.x
  float pX = roi.at<float>(loc.x - 1);
  float pY = roi.at<float>(loc.x);
  float pZ = roi.at<float>(loc.x + 1);

  // # Calculate value with subpixel accuracy
  // EQ1: y = a*x^2 + b*x + c
  // # Put values of [-1, pX], [0, pY], [1, pZ] in EQ1. (offset by loc.x)
  // a = (pX - 2*pY + pZ) / 2
  // b = (pZ - pX) / 2
  // c = pY
  //
  // # EQ1 has its max(or min) value when (2*a*x + b = 0) <== df/dx
  // EQ2: x = -b / (2*a)
  // # Put values of a,b in EQ2
  // x = (pX - pZ) / (2 * pX - 4 * pY + 2 * pZ)
  //
  // # Undo offset by loc.x and return
  return loc.x + (pX - pZ) / (2 * pX - 4 * pY + 2 * pZ);
}

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

  cv::Point2f maxP(getSubPixelX(roi, maxloc), y2);
  cv::Point2f minP(getSubPixelX(roi, minloc), y2);
  // std::cout << maxP << ' ' << minP << '\n';
  return {maxP, minP};
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
                   cv::Scalar(0,255,255), cv::MARKER_TILTED_CROSS, 10, 2,
                   cv::LINE_AA);
    cv::drawMarker(videoFrame, cv::Point(cvRound(ptsL[1].x), SCAN_OFFSET),
                   cv::Scalar(255, 0, 0), cv::MARKER_TILTED_CROSS, 10, 2,
                   cv::LINE_AA);
    cv::drawMarker(videoFrame,
                   cv::Point(cvRound(ptsR[0].x) + width, SCAN_OFFSET),
        cv::Scalar(0, 255, 255), cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
    cv::drawMarker(
        videoFrame, cv::Point(cvRound(ptsR[1].x) + width, SCAN_OFFSET),
        cv::Scalar(255, 0, 0), cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);

    cv::imshow("video", videoFrame);
    int k = cv::waitKey(1);
    if (k == 27 || k == ' ') break;
  }

  video.release();
  cv::destroyAllWindows();
  return returnCode;
}