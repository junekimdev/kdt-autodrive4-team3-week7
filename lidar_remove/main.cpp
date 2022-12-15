#include <iostream>
#include "opencv2/opencv.hpp"

using namespace std;
using namespace cv;

int main()
{
	Mat src = imread("Sub_project.avi_capture.jpg", IMREAD_GRAYSCALE);
	Mat mask = imread("mask.png", IMREAD_GRAYSCALE);

	if (src.empty()) {
		cerr << "Image laod failed!" << endl;
		return -1;
	}

	Mat img;
	resize(src, img, Size(640, 480));
	Mat dst = Mat(img.size(), CV_8UC1, Scalar(255));
	Mat mask_tt = img(Rect(Point(200, 390), Point(440, 480))).clone();
	Mat mask1 = mask.clone();
	mask = ~mask;

	for (int y = 390; y < 480; y++) {
		for (int x = 200; x < 440; x++) {
			if (mask.at<uchar>(y, x) != 255) {
				dst.at<uchar>(y, x) = img.at<uchar>(y - 90, x);
			}
		}
	}

	img.copyTo(dst, mask);

	imshow("mask", mask);
	imshow("mask1", mask1);
	imshow("dst", dst);
	imwrite("dst.png", dst);
	imwrite("img_640_480.png", img);
	imshow("img", img);
	waitKey();
}