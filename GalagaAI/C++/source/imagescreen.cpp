#include "imageutilities.h"
#include "imagescreen.h"

#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <Windows.h>
#include <map>


#define WINVER 0x0500

cv::Mat playerTemplate;
std::vector<cv::Mat> missileTemplate;
std::vector<cv::Mat> bossList;
std::vector<cv::Mat> flyList;
std::vector<cv::Mat> beeList;

//1 is bee, 2 is fly, 3 is boss, 4 is everyone
int phase = 1;
int sweepCount = 0;

int count = 0;

void initialize(){

	cv::Mat mark = cv::imread("scoreMarker.png", CV_LOAD_IMAGE_GRAYSCALE);

	playerTemplate = cv::imread("sprites\\player.png", CV_LOAD_IMAGE_GRAYSCALE);
	missileTemplate.push_back(cv::imread("sprites\\missile.png", CV_LOAD_IMAGE_GRAYSCALE));
	
	std::string bossStrings[4] = { "sprites\\boss\\bossUp.png", 
		"sprites\\boss\\bossDown.png", "sprites\\boss\\bossDownLeft.png", "sprites\\boss\\bossDownRight.png", };
	for (int i = 0; i < 4; i++){
		cv::Mat pic = cv::imread(bossStrings[i], CV_LOAD_IMAGE_GRAYSCALE);
		bossList.push_back(pic);
	}

	std::string flyStrings[5] = { "sprites\\fly\\butterflyDown.png", "sprites\\fly\\butterflyUp.png", "sprites\\fly\\butterflyDownLeft.png", "sprites\\fly\\butterflyDownLeft_2.png",
		 "sprites\\fly\\butterflyDownRight.png" };
	for (int i = 0; i < 5; i++){
		cv::Mat pic = cv::imread(flyStrings[i], CV_LOAD_IMAGE_GRAYSCALE);
		flyList.push_back(pic);
	}

	std::string beeStrings[5] = { "sprites\\bee\\beeUp.png", "sprites\\bee\\beeDownLeft_S.png", "sprites\\bee\\beeDownRight.png", "sprites\\bee\\beeDownLeft.png", "sprites\\bee\\beeDownLeft2.png" };
	for (int i = 0; i < 5; i++){
		cv::Mat pic = cv::imread(beeStrings[i], CV_LOAD_IMAGE_GRAYSCALE);
		beeList.push_back(pic);
	}
}

screenPackage parseScreen(std::map<std::string, std::vector<XYposition>> pastObjects){

	HWND handle = GetForegroundWindow();
	cv::Mat mat;

	if (handle != 0){
		mat = windowToMat(handle);
	}


	XYposition player = findPlayer(mat);

	if (count == 15){
		count = 15;
	}
	else {
		count++;
	}

	std::map<std::string, std::vector<XYposition>> *targetMap = new std::map<std::string, std::vector<XYposition>>();

	if (phase == 3 || phase == 4){
		targetMap = findTargets(mat, targetMap, bossList, "boss");
		if (phase == 3 && targetMap->size() == 0){
			sweepCount++;
			if (sweepCount == 5){
				phase = 4;
				sweepCount = 0;
			}
		}
		else{
			sweepCount = 0;
		}
	}
	if (phase == 2 || phase == 4){
		targetMap = findTargets(mat, targetMap, flyList, "fly");
		if (phase == 2 && targetMap->size() == 0){
			sweepCount++;
			if (sweepCount == 5){
				phase = 3;
				sweepCount = 0;
			}
		}
		else{
			sweepCount = 0;
		}
	}
	if (phase == 1 || phase == 4){
		targetMap = findTargets(mat, targetMap, beeList, "bee");
		if (phase == 1 && targetMap->size() == 0){
			sweepCount++;
			if (sweepCount == 5){
				phase = 2;
				sweepCount = 0;
			}
		}
		else{
			sweepCount = 0;
		}
	}
	targetMap = findTargets(mat, targetMap, missileTemplate, "missile");

	std::vector<Threat> threats = findThreats(*targetMap, pastObjects, player.Y);

	screenPackage package = {
		*targetMap,
		threats,
		player.X
	};

	return package;
}

XYposition findPlayer(cv::Mat mat){
	XYposition val = templateMatch(mat, playerTemplate);
	return val;
}

std::map<std::string, std::vector<XYposition>> * findTargets(cv::Mat mat, std::map<std::string, std::vector<XYposition>> *threatMap,
		std::vector<cv::Mat> threatList, std::string type){

	std::vector<XYposition> threats = multipleTemplateMatch(mat, threatList);
	
	if (threatMap->find(type) != threatMap->end()){
		for (XYposition p : threats){
			std::vector<XYposition> v = threatMap->at(type);
			threatMap->at(type).push_back(p);
		}
	}
	else {
		threatMap->insert(std::pair<std::string, std::vector<XYposition>>(type, threats));
	}
	
	return threatMap;
}

std::vector<Threat> findThreats(std::map<std::string, std::vector<XYposition>> currentObjects,
	std::map<std::string, std::vector<XYposition>> pastObjects, double playerY){

	std::vector<Threat> threats;
	if (pastObjects.size() == 0){
		return threats; 
	}

	//iter returns a pair pointer
	for (std::map<std::string, std::vector<XYposition>>::iterator iter = currentObjects.begin();
		iter != currentObjects.end(); ++iter){
		std::string key = iter->first;

		//continue if not there
		if (pastObjects.find(key) == pastObjects.end()){
			continue;
		}

		std::vector<XYposition> threatList = currentObjects.find(key)->second;
		std::vector<XYposition> pastThreatList = pastObjects.find(key)->second;

		for (XYposition threat : threatList){
			for (XYposition oldThreat : pastThreatList){

				if (key == "missile"){

					double denom = (threat.X - oldThreat.X);
					if (denom == 0){
						denom = .0001;
					}
					double slope = (threat.Y - oldThreat.Y) / denom;
					double math = (playerY - threat.Y) + (slope*threat.X);
					double impPoint = math / slope;

					double yDif = playerY - threat.Y;
					double y = threat.Y - oldThreat.Y;
					double time;
					if (y != 0){
						time = yDif / (y);
					}
					else {
						continue; //0 here means its moving sideways
					}

					if (time < 0){
						continue; //positive means its going up
					}

					if (impPoint < 0 || impPoint > 480){
						continue;
					}

					Threat info = {
						key,
						threat.X,
						threat.Y,
						impPoint,
						time
					};

					threats.push_back(info);
				}
				else{
					double dis = distFormu(threat.X, threat.Y, oldThreat.X, oldThreat.Y);
					if (dis != 0 && dis < 45){

						double yDif = playerY - threat.Y;
						double y = threat.Y - oldThreat.Y;
						double time;
						if (y != 0){
							time = yDif / (y);
						}
						else {
							continue; //0 here means its moving sideways
						}

						if (time < 0){
							continue; //positive means its going up
						}


						double x = threat.X - oldThreat.X;
						double offset;
						if (x != 0){
							offset = time*x;
						}
						else {
							offset = 0;
						}

						double pos = offset + threat.X;
						
						//magic numbers are player line pulled from
						//measuring game
						if (pos < 0 || pos > 480){
							continue;
						}

						Threat info = {
							key,
							threat.X,
							threat.Y,
							pos,
							time
						};

						threats.push_back(info);
					}
				}
			}
		}

	}

	return threats;
}

