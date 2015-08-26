#ifndef IMAGESCREEN_H
#define IMAGESCREEN_H

#include "imageutilities.h"
#include <map>;

struct screenPackage{
	std::map<std::string, std::vector<XYposition>> objects;
	std::vector<Threat> threats;
	double playerX;
};


void initialize();
screenPackage parseScreen(std::map<std::string, std::vector<XYposition>> pastObjects);
XYposition findPlayer(cv::Mat mat);
std::vector<Threat> findThreats(std::map<std::string, std::vector<XYposition>> currentObjects,
	std::map<std::string, std::vector<XYposition>> pastObjects, double playerY);
std::map<std::string, std::vector<XYposition>> * findTargets(cv::Mat mat, std::map<std::string,
	std::vector<XYposition>> *threatMap, std::vector<cv::Mat> threatList, std::string type);


#endif