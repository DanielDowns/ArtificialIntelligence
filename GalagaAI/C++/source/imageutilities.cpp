#include "imageutilities.h"

#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
#include <opencv2/highgui/highgui.hpp>

#include <ctime>
#include <math.h>
#include <Windows.h>
#include <iostream>
#include <string>

//the game screen if the top of the screen
//is flush with the browser bar
RECT subwindow = { 410, 100, 890, 755 };

XYposition templateMatch(cv::Mat &img, cv::Mat &mytemplate)
{	
	cv::Mat convertImg(img.rows, img.cols, CV_8UC3);
	cv::cvtColor(img, convertImg, CV_BGRA2GRAY);

	cv::Mat result(convertImg.rows - mytemplate.rows + 1, convertImg.cols - mytemplate.cols + 1,
		CV_32FC1); //must be this result type

	cv::matchTemplate(convertImg, mytemplate, result, CV_TM_CCOEFF_NORMED); //crash here
	cv::normalize(result, result, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

	double minVal; double maxVal; 
	cv::Point minLoc; 
	cv::Point maxLoc;

	cv::minMaxLoc(result, &minVal, &maxVal, &minLoc, &maxLoc, cv::Mat());
	XYposition playerData = {
		maxLoc.x - ceil(mytemplate.cols/2), maxLoc.y - ceil(mytemplate.rows/2)
	};

	rectangle(result, maxLoc, cv::Point(maxLoc.x - mytemplate.cols, maxLoc.y - mytemplate.rows),
		cv::Scalar(0, 0, 255), 4, 8, 0);

	return playerData;
}

std::vector<XYposition> multipleTemplateMatch(cv::Mat &img, std::vector<cv::Mat> tplList){

	std::vector<XYposition> matches;

	cv::Mat convertImg(img.rows, img.cols, CV_8UC3);
	cv::cvtColor(img, convertImg, CV_BGRA2GRAY);

	double threshold = 0.8;

	int imgint = convertImg.type();

	for(cv::Mat tpl : tplList){
		int tplint = tpl.type();
		cv::Mat result(convertImg.rows - tpl.rows + 1, convertImg.cols - tpl.cols + 1,
			CV_32FC1); //must be this result type

		int one = 0;

		cv::matchTemplate(convertImg, tpl, result, CV_TM_CCOEFF_NORMED);
		cv::threshold(result, result, threshold, 1., CV_THRESH_TOZERO);

		while (true)
		{
			double minval, maxval;
			cv::Point minloc, maxloc;
			cv::minMaxLoc(result, &minval, &maxval, &minloc, &maxloc);
			if (maxval >= threshold)
			{

				if (one == 1){
					cv::namedWindow("result", CV_WINDOW_AUTOSIZE);
					imshow("result", result);
					cv::waitKey(0);
					one--;
				}

				rectangle(result,
					cv::Point(maxloc.x - 15, maxloc.y - 15), 
					cv::Point(maxloc.x - tpl.cols + 15, maxloc.y + tpl.rows - 15),
					cv::Scalar(0, 0, 255), 
					4,
					8, 
					0);

				cv::floodFill(result, 
					maxloc, 
					cv::Scalar(0), 
					0, 
					cv::Scalar(.1), 
					cv::Scalar(1.)); 

				float DIST_THRESH = 20;
				bool tooClose = false;
				for (XYposition pos : matches){
					float dist = distFormu(pos.X, pos.Y, maxloc.x, maxloc.y);
					if (dist < DIST_THRESH){
						tooClose = true;
						break;
					}
				}

				if (!tooClose){
					XYposition info = {
						maxloc.x - ceil(tpl.cols / 2), maxloc.y - ceil(tpl.rows / 2)
					};
					matches.push_back(info);
				}
			}
			else
				break;
		}
	}

	return matches;
}

cv::Mat windowToMat(HWND hwnd){

	HDC hwindowDC, hwindowCompatibleDC;

	int height, width, srcheight, srcwidth;
	HBITMAP hbwindow;
	cv::Mat src;
	BITMAPINFOHEADER  bi;

	hwindowDC = GetDC(hwnd);
	hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
	SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

	RECT windowsize;    // get the height and width of the screen
	GetClientRect(hwnd, &windowsize);

	srcheight = subwindow.bottom - subwindow.top;// windowsize.bottom;
	srcwidth = subwindow.right - subwindow.left;// windowsize.right;
	height = subwindow.bottom - subwindow.top;//windowsize.bottom;  //change this to whatever size you want to resize to
	width = subwindow.right - subwindow.left;//windowsize.right;

	src.create(height, width, CV_8UC4);

	// create a bitmap
	hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = width;
	bi.biHeight = -height;  //this is the line that makes it draw upside down or not
	bi.biPlanes = 1;
	bi.biBitCount = 32;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrUsed = 0;
	bi.biClrImportant = 0;

	// use the previously created device context with the bitmap
	SelectObject(hwindowCompatibleDC, hbwindow);
	// copy from the window device context to the bitmap device context
	//StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, subwindow.left, 100, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
	GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO *)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

	// avoid memory leak
	DeleteObject(hbwindow); DeleteDC(hwindowCompatibleDC); ReleaseDC(hwnd, hwindowDC);

	return src;
}

double distFormu(double x1, double y1, double x2, double y2){
	//###DONE BUT NOT TESTED

	double xVal = x2 - x1;
	double yVal = y2 - y1;

	xVal = pow(xVal, 2);
	yVal = pow(yVal, 2);

	double val = sqrt(xVal + yVal);
	return val;
}
