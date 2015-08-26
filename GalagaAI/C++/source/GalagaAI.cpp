// GalagaAI.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <Windows.h>
#include <algorithm>
#include <math.h>
#include <thread>

#include "ainodes.h"
#include "dodgealg.h"
#include "imagescreen.h"
#include "imageutilities.h"
#include "keyboard.h"

#define WINVER 0x0500
#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

//http://game-oldies.com/play-online/galaga-coin-op-arcade#
int _tmain(int argc, _TCHAR* argv[])
{

	/*  this is timing checker info. run and move the player. output shows how many 
	pixels it moves per check. ideal is ~15, ok is 50. 
	float pastPos = -1000;
	float player = -1000;
	cv::Mat playerTemplate = cv::imread("sprites\\player.png", CV_LOAD_IMAGE_GRAYSCALE);
	std::vector<cv::Mat> bossList;
	std::vector<cv::Mat> flyList;
	std::vector<cv::Mat> beeList;

	std::string bossStrings[4] = { "sprites\\boss\\bossUp.png", "sprites\\boss\\bossDown.png", "sprites\\boss\\bossDownLeft.png", "sprites\\boss\\bossDownRight.png"};
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

	cv::Mat mat; 

	while (1){
		HWND handle = GetForegroundWindow();
		RECT subwindow = { 1370, 100, 1850, 755};


		if (handle != 0){
			mat = windowToMat(handle);
	//		cv::namedWindow("result", CV_WINDOW_AUTOSIZE);
	//		imshow("result", mat);
	//		cv::waitKey(0);
		}

		std::map<std::string, std::vector<XYposition>> *targetMap = new std::map<std::string, std::vector<XYposition>>();

		targetMap = findTargets(mat, targetMap, bossList, "boss");
		targetMap = findTargets(mat, targetMap, flyList, "fly");
		targetMap = findTargets(mat, targetMap, beeList, "bee");


		pastPos = player;
		player = templateMatch(mat, playerTemplate).X;


		DBOUT("\n");
		DBOUT(abs(player - pastPos));
	}

	*/


	
	initialize(); 

	// Pause for 3 seconds
	Sleep(3000);

	behaviorTree tree = behaviorTree();

	concurrentSelectorNode AI = new concurrentSelectorNode(NULL);
	AI.nodeName = "AI";

	prioritySelectorNode moveBranch = new prioritySelectorNode(&AI);
	moveBranch.nodeName = "move";

	dodgeNode * dodge = new dodgeNode(&moveBranch);
	dodge->nodeName = "dodge";
//	adjustNode * adjust = new adjustNode(&moveBranch);
//	adjust->nodeName = "adjust";

	moveNode * move = new moveNode(&AI);
	move->nodeName = "move";
	fireNode * fire = new fireNode(&AI);
	fire->nodeName = "fire";
	
	tree.root = &AI;

//	std::thread moveThread(move);
//	moveThread.detach();
	tree.runTree(); 
	
	return 0;
}
