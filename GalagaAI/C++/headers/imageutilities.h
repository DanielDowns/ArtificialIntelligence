#ifndef IMAGEUTILITIES_H
#define IMAGEUTILITIES_H

#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <Windows.h>

struct XYposition{
	float X;
	float Y;
};

struct Threat{
	std::string type;
	float xCoord;
	float yCoord;
	float impactPoint;
	float timeTilImpact;
};

XYposition templateMatch(cv::Mat &img, cv::Mat &mytemplate);
std::vector<XYposition> multipleTemplateMatch(cv::Mat &img, std::vector<cv::Mat> tplList);
cv::Mat windowToMat(HWND hwnd);
double distFormu(double x1, double y1, double x2, double y2);

#endif