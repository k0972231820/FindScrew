#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <time.h>
#include <fstream>      
#include "opencv2/video/tracking.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/legacy/legacy.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/nonfree/nonfree.hpp"  

using namespace cv;
using namespace std;


void GetH(int x, int y, Mat image, float HSL[3]);
Mat Change(Mat src, Mat dst);
float getDistance(Point2f point1, Point2f point2);
void WriteFile(Mat img);
void ContourNut(Mat img);
void ContourScrew(Mat img);


int minHessian = 500;
int num = 0, isFind = 0;
Mat img;
vector<cv::Point2f> nut;
vector<cv::Point2f> screw;
int xsliderValue, ysliderValue;

int main() {
	Mat img_3;
	VideoCapture video(0);
	//if (!video.isOpened()) {
	//	cout << "Camera Open failed" << endl;
	//	return -1;
	//}

	//cout << video.get(CV_CAP_PROP_FRAME_WIDTH) << " " << video.get(CV_CAP_PROP_FRAME_HEIGHT) << endl;
	video.set(CV_CAP_PROP_FRAME_WIDTH, 1920);
	video.set(CV_CAP_PROP_FRAME_HEIGHT, 1080);
	Mat videoFrame;
	time_t start, mid, temp;
	start = time(NULL);

	/*xsliderValue = 0;
	ysliderValue = 0;
	int sliderMaxValue = 2000;
	namedWindow("frame", 0);
	createTrackbar("x", "frame", &xsliderValue, sliderMaxValue);
	createTrackbar("y", "frame", &ysliderValue, sliderMaxValue);*/

	//while(true){
		//video >> videoFrame;
		//if(videoFrame.empty()){
		//	break;
		//}
	videoFrame = imread("E:\\影像處理圖\\Screw.PNG");
	//ContourNut(videoFrame);
	ContourScrew(videoFrame);

	imshow("frame", videoFrame);
	//waitKey(33);
	//}

	//WriteFile(videoFrame);
	waitKey(0);
	system("pause");
	return 0;
}

void GetH(int x, int y, Mat image, float HSL[3]) {
	float H, S, L;
	Vec3b rgb = image.at<Vec3b>(y, x);
	float B = (float)rgb[0] / 255;
	float G = (float)rgb[1] / 255;
	float R = (float)rgb[2] / 255;
	//cout << (int)image.at<Vec3b>(y,x)[0] << " " << (int)image.at<Vec3b>(y,x)[1] << " " << (int)image.at<Vec3b>(y,x)[2] << endl;
	float max = 0, min = 9999, maxNum = 7, minNum = 7;
	for (int i = 0; i < 3; i++) {
		if (rgb.val[i] > max) {
			max = rgb.val[i];
			maxNum = i;
		}

		if (rgb.val[i] < min) {
			min = rgb.val[i];
			minNum = i;
		}
	}

	max = max / 255;
	min = min / 255;

	L = (max + min) / 2;

	if (L < 0.5) {
		S = (max - min) / (max + min);
	}

	if (L >= 0.5) {
		S = (max - min) / (2 * (max + min));
	}

	if (maxNum == 2) {
		H = (60 * (G - B)) / S;
	}

	else if (maxNum == 1) {
		H = 120 + (60 * (B - R)) / S;
	}

	else {
		H = 240 + (60 * (R - G)) / S;
	}
	HSL[0] = H;
	HSL[1] = S;
	HSL[2] = L;
	//cout << "HSL: " << H << " " << S << " " << L << endl;
}

Mat Change(Mat src, Mat dst) {
	src.copyTo(dst);
	for (int i = 0; i < src.rows; i++) {
		for (int j = 0; j < src.cols; j++) {
			float HSL[3];
			GetH(j, i, src, HSL);
			if (HSL[1] < 0.13) dst.at<Vec3b>(i, j) = Vec3b(255, 255, 255);
			else if (HSL[2] < 0.15) dst.at<Vec3b>(i, j) = Vec3b(255, 255, 255);
			else if (HSL[0] > 200 && HSL[0] <= 240) dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0); // blue
			else if (HSL[0] > 240 && HSL[0] <= 360) dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
			else if (HSL[0] > 60 && HSL[0] <= 130) dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);  // green
			else if (HSL[0] > 130 && HSL[0] <= 200) dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
			else if (HSL[0] > 40 && HSL[0] <= 60) dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0);  // yellow
			else if (HSL[0] >= -40 && HSL[0] < 11) dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0); // red
			else if (HSL[0] >= 11 && HSL[0] <= 40) {
				if (HSL[2] < 0.15) dst.at<Vec3b>(i, j) = Vec3b(255, 255, 255);  // brown wood
				else if (HSL[1] >= 0.43) dst.at<Vec3b>(i, j) = Vec3b(0, 0, 0); // orange 0.48
				else dst.at<Vec3b>(i, j) = Vec3b(255, 255, 255); // wood
			}
			//else dst.at<Vec3b>(i, j) = Vec3b(255,255,255);
		}
	}
	//imshow("test", dst);
	return dst;
}



float getDistance(Point2f point1, Point2f point2)
{
	float distance;
	distance = powf((point1.x - point2.x), 2) + powf((point1.y - point2.y), 2);
	distance = sqrtf(distance);

	return distance;
}


void WriteFile(Mat img) {
	fstream file;
	file.open("E:\\LAB602資料\\FindScrew\\position.txt", ios::out | ios::ate);
	if (!file) {//如果開啟檔案失敗，fp為0；成功，fp為非0
		cout << "Fail to open file: " << endl;
		return;
	}

	unsigned seed;
	seed = (unsigned)time(NULL);
	srand(seed);

	if (nut.size() == 0) file << 0 << " " << 0 << endl;
	else {
		int random = rand() % nut.size();
		file << nut[random].x << " " << nut[random].y << endl;
		circle(img, nut[random], 10, Scalar(0, 255, 0), 3);
	}

	/*if (screw.size() == 0) file << 0 << " " << 0 << endl;
	else {
		int random = rand() % screw.size();
		file << screw[random].x << " " << screw[random].y << endl;
		circle(img, screw[random], 5, Scalar(0,255,0),3);
	}*/

	imshow("choice", img);
	return;
}

void ContourNut(Mat img) {
	Mat temp = img.clone();
	Mat threshold_output, src_gray;
	vector<cv::vector<cv::Point> > contours(10000);
	cv::vector<Vec4i> hierarchy(10000);
	int thresh = 70;
	int max_thresh = 255;
	RNG rng(12345);

	cvtColor(img, src_gray, CV_RGB2GRAY);
	//blur(src_gray, src_gray, Size(3, 3));
	GaussianBlur(src_gray, src_gray, Size(9, 9), 2, 2);
	imshow("GaussianBlur", src_gray);

	/// Detect edges using Threshold
	threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY);
	imshow("threshold", threshold_output);

	/// Find contours
	findContours(threshold_output, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	/// Approximate contours to polygons + get bounding rects and circles
	vector<vector<Point> > contours_poly( contours.size() );
	vector<Rect> boundRect( contours.size() );
	vector<Point2f>center( contours.size() );
	vector<RotatedRect> minRect( contours.size() );

	for( int i = 0; i < contours.size(); i++ )
	   { approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
	     boundRect[i] = boundingRect( Mat(contours_poly[i]) );
	     minRect[i] = minAreaRect( Mat(contours[i]) );
	   }
	
	int time = 0;
	/// Draw polygonal contour + bonding rects + circles
	Mat drawing = Mat::zeros( threshold_output.size(), CV_8UC3 );
	for( int i = 0; i< contours.size(); i++ )
	   {
	     Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
	     drawContours( drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point() );
	     //rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
	     Point2f rect_points[4]; 
	     minRect[i].points( rect_points );
	     for( int j = 0; j < 4; j++ ) {
		     //if ( getDistance(rect_points[0], rect_points[1]) > 40 || getDistance(rect_points[1], rect_points[2]) > 40 )
		         //line( drawing, rect_points[j], rect_points[(j+1)%4], color, 1, 8 );
		     if ( (getDistance(rect_points[0], rect_points[1]) > 10 && getDistance(rect_points[0], rect_points[1]) < 23)
		  	    && (getDistance(rect_points[1], rect_points[2]) > 10 && getDistance(rect_points[1], rect_points[2]) < 23) ) {
		  		Point2f center = Point2f((rect_points[0].x+rect_points[2].x)/2, (rect_points[0].y+rect_points[2].y)/2);
		  		//if (center.x >= 875 && center.x <= 1200 && center.y >= 440 && center.y <= 805) {
		  			line( temp, rect_points[j], rect_points[(j+1)%4], color, 2, 8 );
		  			nut.push_back(center);
		  		//}
		     }
	     }
	   }

	/// Show in a window
	//namedWindow( "Contours", CV_WINDOW_AUTOSIZE );
	imshow( "Contours", drawing );
	imshow("findNut", temp);
}

void ContourScrew(Mat img) {
	Mat temp = img.clone();
	Mat threshold_output, src_gray;
	vector<vector<Point> > contours(10000);
	vector<Vec4i> hierarchy(10000);
	int thresh = 110;
	int max_thresh = 255;
	RNG rng(12345);

	cvtColor(img, src_gray, CV_RGB2GRAY);
	//blur(src_gray, src_gray, Size(3, 3));
	GaussianBlur( src_gray, src_gray, Size(9, 9), 2, 2);
	imshow("GaussianBlur", src_gray);

	
	/// Detect edges using Threshold
	threshold(src_gray, threshold_output, thresh, 255, THRESH_BINARY);
	imshow("threshold", threshold_output);
	//獲取自定義核
	Mat element = getStructuringElement(MORPH_RECT, Size(3, 3)); //第一個引數MORPH_RECT表示矩形的卷積核，當然還可以選擇橢圓形的、交叉型的
	//膨脹操作
	erode(threshold_output, threshold_output, element);
	imshow("threshold2", threshold_output);

	/// Find contours
	findContours(threshold_output, contours, hierarchy, CV_RETR_EXTERNAL/*CV_RETR_CCOMP*/, CV_CHAIN_APPROX_SIMPLE);

	/// Approximate contours to polygons + get bounding rects and circles
	vector<vector<Point> > contours_poly(contours.size());
	vector<Rect> boundRect(contours.size());
	vector<Point2f>center(contours.size());
	vector<RotatedRect> minRect(contours.size());

	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
		boundRect[i] = boundingRect(Mat(contours_poly[i]));
		minRect[i] = minAreaRect(Mat(contours[i]));
	}

	int time = 0;
	/// Draw polygonal contour + bonding rects + circles
	Mat drawing = Mat::zeros(threshold_output.size(), CV_8UC3);
	for (int i = 0; i < contours.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point());
		//rectangle( drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0 );
		Point2f rect_points[4];
		minRect[i].points(rect_points);
		for (int j = 0; j < 4; j++) {
			if ((getDistance(rect_points[0], rect_points[1]) > 2 && getDistance(rect_points[0], rect_points[1]) < 50) &&
				(getDistance(rect_points[1], rect_points[2]) > 60 && getDistance(rect_points[1], rect_points[2]) < 150)) {
				Point2f center = Point2f((rect_points[0].x + rect_points[2].x) / 2, (rect_points[0].y + rect_points[2].y) / 2);
				line(temp, rect_points[j], rect_points[(j + 1) % 4], color, 2, 8);
				screw.push_back(center);
			}
			if ((getDistance(rect_points[0], rect_points[1]) > 60 && getDistance(rect_points[0], rect_points[1]) < 150) &&
				(getDistance(rect_points[1], rect_points[2]) > 2 && getDistance(rect_points[1], rect_points[2]) < 50)) {
				Point2f center = Point2f((rect_points[0].x + rect_points[2].x) / 2, (rect_points[0].y + rect_points[2].y) / 2);
				line(temp, rect_points[j], rect_points[(j + 1) % 4], color, 2, 8);
				screw.push_back(center);
			}
		}
	}

	/// Show in a window
	imshow("Contours", drawing);
	imshow("findScrew", temp);
}


