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

    return { lp, rp };
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

    return { leftsidePt, rightsidePt };
}

int main() {
    std::vector<int> pxL_temp = { 36, 62 };
    std::vector<int> pxR_temp = { 518, 545 };

    int returnCode = 0;
    int count = 0;

    cv::VideoCapture video = cv::VideoCapture(VIDEO_FILENAME);
    if (!video.isOpened()) {
        std::cerr << "Failed to load the video" << '\n';
        returnCode |= ERROR_LOADING_VIDEO;
    }
    if (returnCode) return returnCode;

    while (1) {
        std::stringstream ss;

        cv::Mat videoFrame, video_ori;
        
        video >> videoFrame;
        video_ori = videoFrame.clone();
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
        
        // ::cerr << "pxL_temp = " << pxL_temp[0] << "," << pxL_temp[1] << " | " << "pxL = " << pxL[0] << "," << pxL[1] << std::endl;
        if ((abs(int((pxL[0] + pxL[1]) / 2) - int((pxL_temp[0] + pxL_temp[1]) / 2))) >= 50) {
            pxL = pxL_temp;
            // std::cerr << "abs = " << abs(abs(int((pxL[0] + pxL[1]) / 2)) - abs(int((pxL_temp[0] + pxL_temp[1]) / 2))) << std::endl;
        }
        pxL_temp = pxL;

        std::cerr << "pxL_temp_center = " << ((pxL_temp[0] + pxL_temp[1]) / 2) << std::endl;

        //if ((abs(int((pxR[0] + pxR[1]) / 2) - int((pxR_temp[0] + pxR_temp[1]) / 2))) >= 25) {
        //    pxR = pxR_temp;
        //}
        //pxR_temp = pxR;

        if ((pxL_temp[0] + pxL_temp[1] ) / 2 > 130) {
            // ::cerr << "pxL_temp = " << pxL_temp[0] << "," << pxL_temp[1] << " | " << "pxL = " << pxL[0] << "," << pxL[1] << std::endl;
            if ((abs(int((pxR[0] + pxR[1]) / 2) - int((pxR_temp[0] + pxR_temp[1]) / 2))) >= 100) {
                pxR = pxR_temp;
            }
            pxR_temp = pxR;
        }
        
        else if ((pxL_temp[0] + pxL_temp[1]) / 2 < 20 ){
            if ((abs(int((pxR[0] + pxR[1]) / 2) - int((pxR_temp[0] + pxR_temp[1]) / 2))) >= 60) {
                pxR = pxR_temp;
            }
            pxR_temp = pxR;
        }
        else {
            if ((abs(int((pxR[0] + pxR[1]) / 2) - int((pxR_temp[0] + pxR_temp[1]) / 2))) >= 20) {
                pxR = pxR_temp;
            }
            pxR_temp = pxR;
        }
        if (pxR_temp == pxR) {
            count++;
            if (count > 40) {
                std::cerr << "count = " << count << std::endl;
                ptsR = find_edges(roiR, false);
                pxR = filterX(ptsR, width, videoFrame.cols - 1, false);
                count = 0;
            }
        }


        // std::cout << pxL[0] << ' ' << pxL[1] << " | ";
        // std::cout << pxR[0] << ' ' << pxR[1] << '\n';

        // Display
        cv::drawMarker(videoFrame, cv::Point(pxL[0], SCAN_OFFSET), YELLOW,
            cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
        cv::drawMarker(videoFrame, cv::Point(pxL[1], SCAN_OFFSET), BLUE,
            cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
        cv::putText(videoFrame, cv::format("(%d)", pxL[0]),
            cv::Point(pxL[0] - 30, SCAN_OFFSET - 20), cv::FONT_HERSHEY_SIMPLEX,
            0.3, YELLOW, 1, cv::LINE_AA);
        cv::putText(videoFrame, cv::format("(%d)", pxL[1]),
            cv::Point(pxL[1] - 20, SCAN_OFFSET + 30), cv::FONT_HERSHEY_SIMPLEX,
            0.3, BLUE, 1, cv::LINE_AA);
        
        cv::drawMarker(videoFrame, cv::Point(pxR[0], SCAN_OFFSET), YELLOW,
            cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
        cv::drawMarker(videoFrame, cv::Point(pxR[1], SCAN_OFFSET), BLUE,
            cv::MARKER_TILTED_CROSS, 10, 2, cv::LINE_AA);
        cv::putText(videoFrame, cv::format("(%d)", pxR[0]),
            cv::Point(pxR[0] - 30, SCAN_OFFSET - 20), cv::FONT_HERSHEY_SIMPLEX,
            0.3, YELLOW, 1, cv::LINE_AA);
        cv::putText(videoFrame, cv::format("(%d)", pxR[1]),
            cv::Point(pxR[1] - 20, SCAN_OFFSET + 30), cv::FONT_HERSHEY_SIMPLEX,
            0.3, BLUE, 1, cv::LINE_AA);

        cv::line(videoFrame, cv::Point(0, SCAN_OFFSET),
            cv::Point(videoFrame.cols, SCAN_OFFSET), BLUE);
        cv::line(video_ori, cv::Point(0, SCAN_OFFSET),
            cv::Point(video_ori.cols, SCAN_OFFSET), BLUE);
        cv::imshow("video", videoFrame);
        cv::imshow("grayFrame", grayFrame);

        /*// image 저장
        if (count % 30 == 0) {
            std::cerr << "count = " << count << "divide 30 = " << count/30 << std::endl;
            ss << count/30 << ".jpg";

            std::string filename = ss.str();
            cv::imwrite(filename, video_ori);
        }
        count++;
        */
        int k = cv::waitKey(1);
        if (k == 27 || k == ' ') break;
    }
    std::cerr << "count = " << count << std::endl;
    video.release();
    cv::destroyAllWindows();
    return returnCode;
}