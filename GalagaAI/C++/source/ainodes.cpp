#include <string>
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

#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}


std::map<std::string, std::vector<XYposition>> pastObjects;
float playerPos = -1000;
float targetPoint;
std::string moveType = "none";

std::string  MyTreeNode::execute(){
	return "";
}

//basic selection that forms branches of tree
SelectorNode::SelectorNode(){
	status = "ready";
	nodeType = "SelectorNode";
}
//basic selection that forms branches of tree
SelectorNode::SelectorNode(SelectorNode * p){
	status = "ready";
	nodeType = "SelectorNode";
	parent = p;

	parent->children.push_back(this);
}

//allows priority execution of children and stops when failed or running 
std::string SelectorNode::traverseChildren(){
	std::string result = "";
	std::vector<MyTreeNode *>::iterator iter;

	for (iter = children.begin(); iter != children.end(); iter++){
		result = (*iter)->execute();
		if (result == "failed" || result == "running"){
			return result;
		}
	}
	return "completed";
}

//action nodes
dodgeNode::dodgeNode(SelectorNode * p){
	status = "ready";
	nodeType = "dodgeNode";
	parent = p;

	parent->children.push_back(this);
} 

std::string dodgeNode::execute(){
	//###DONE BUT NOT TESTED

	std::map<std::string, std::vector<XYposition>> objects = pastObjects;
	screenPackage package = parseScreen(objects);

	std::map<std::string, std::vector<XYposition>> newObjects = package.objects;
	std::vector<Threat> threats = package.threats;
	playerPos = package.playerX;
	DBOUT("\n");

	//playerPos += 20; don't know if this happens in c++
	pastObjects = newObjects;

	if (threats.size() != 0){
//		DBOUT("no threats!\n");
		return "complete";
	}

	float maxP = -1000;
	float threshold = -5;

	std::vector<int> targetPoints;
	const int MAX_LINE = 460;
	const int MIN_LINE = 40;
	std::vector<int> playerLine;
	std::vector<int> leftLine;
	std::vector<int> rightLine;

//	DBOUT("right line--");
	for (int i = MIN_LINE; i <= MAX_LINE; i++){
		playerLine.push_back(i);
		if (i <= playerPos){
			leftLine.push_back(i);
		}
		else{
			rightLine.push_back(i);
		}
	}
	//problem is after here but before other
	std::reverse(leftLine.begin(), leftLine.end()); //problem is here

	int pastPoint = -100000;
	for (int point : rightLine){

		double value;
		if (pastPoint != -100000 && pastPoint < threshold){
			value = pastPoint;
		} 
		else{
			double threatVal = point_threat(threats, point);
			double rewardVal = point_enemy_reward(pastObjects, point);
			double distanceVal = point_distance(point, playerPos);
			double tacticalVal = point_tactical_value(point);

			value = rewardVal - threatVal - distanceVal + tacticalVal;
		}

		pastPoint = value;
//		targetPoints.push_back(value);
//		DBOUT(value);
//		DBOUT(" ");
		if (value > maxP){
			maxP = value;
			targetPoint = point;
		}
	}

//	DBOUT("\nleft line--");
	targetPoints.empty();
	for (int point : leftLine){

		double value;
		if (pastPoint != -100000 && pastPoint < threshold){
			value = pastPoint;
		}
		else{
			double threatVal = point_threat(threats, point);
			double rewardVal = point_enemy_reward(pastObjects, point);
			double distanceVal = point_distance(point, playerPos);
			double tacticalVal = point_tactical_value(point);

			value = rewardVal - threatVal - distanceVal + tacticalVal;
		}

		pastPoint = value;
//		targetPoints.push_back(value);
//		DBOUT(value);
//		DBOUT(" ");
		if (value > maxP){
			maxP = value;
			targetPoint = point;
		}
	}
	if (playerPos == targetPoint){
		return "complete";
	}
	
	return "running"; 
}

fireNode::fireNode(SelectorNode * p){
	status = "ready";
	nodeType = "fireNode";
	parent = p;

	parent->children.push_back(this);
}

std::string fireNode::execute(){

	//###DONE BUT NOT TESTED

	for (std::map<std::string, std::vector<XYposition>>::iterator iter = pastObjects.begin();
		iter != pastObjects.end(); ++iter){
		if (iter->first == "missile"){
			continue;
		}
		for (XYposition target : iter->second){
			if (abs(playerPos - target.X) <= 10){
				fireShot();
				return "complete";
			}
		}
	}

	return "complete";
}


adjustNode::adjustNode(SelectorNode * p){
	status = "ready";
	nodeType = "adjustNode";
	parent = p;

	parent->children.push_back(this);
}

std::string adjustNode::execute(){

	//###DONE BUT NOT TESTED

	std::map<std::string, std::vector<XYposition>> objects = pastObjects;
	//playerPos += 20; dont know if c++ needs this

	float min = 1000000;
	XYposition lock = { -1000, -1000 };

	for (std::map<std::string, std::vector<XYposition>>::iterator iter = objects.begin();
		iter != objects.end(); iter++){
		if (iter->first == "missile"){
			continue;
		}
		for (XYposition target : iter->second){
			float distance = abs(playerPos - target.X);
			if (distance < min){
				min = distance;
				lock = target;
			}
		}
	}

	if (lock.X != -1000 && lock.Y != -1000){
		targetPoint = lock.X;
	}

	return "";
}

moveNode::moveNode(SelectorNode * p){
	status = "ready";
	nodeType = "moveNode";
	parent = p;

	parent->children.push_back(this);
}

std::string moveNode::execute(){
	double dif = abs(targetPoint - playerPos);
	if (playerPos < targetPoint && dif > 5){
		MoveRight();
		moveType = "right";
	}
	else if (targetPoint < playerPos && dif > 5){
		MoveLeft();
		moveType = "left";
	}
	else if (dif < 5 || (moveType == "right" && playerPos < targetPoint) || (moveType == "left" && playerPos > targetPoint)){
		stopAllMovemeent();
		moveType = "none";
	}
	return "";
}


prioritySelectorNode::prioritySelectorNode(SelectorNode * node) : SelectorNode(node){
	nodeType = "prioritySelectorNode";
	parent = node;
}

std::string prioritySelectorNode::execute(){
	return SelectorNode::traverseChildren();
}

concurrentSelectorNode::concurrentSelectorNode(SelectorNode * node){
	nodeType = "concurrentSelectorNode";
	parent = node;
} 

std::string concurrentSelectorNode::execute(){
	std::string result = "";
	std::vector<MyTreeNode *>::iterator iter;
	
	for (iter = children.begin(); iter != children.end(); iter++){
		result = (*iter)->execute();
	}
	return "";
}

runningSelectorNode::runningSelectorNode(SelectorNode * node){
	nodeType = "runningSelectorNode";
	parent = node;
}

std::string runningSelectorNode::runningCheck(){
	std::vector<MyTreeNode *>::iterator iter;

	for (iter = children.begin(); iter != children.end(); iter++){
		if ((*iter)->status == "running"){
			return "running";
		}
	}

	return "-1";
}

behaviorTree::behaviorTree(){
	root = NULL;
}

std::string behaviorTree::resetTree(MyTreeNode * node){
	std::string retString = "";

	if (node->nodeType != "actionNode"){
		std::string nodeStatus = "";

		std::vector<MyTreeNode *>::iterator iter;
		for (iter = node->children.begin(); iter != node->children.end(); iter++){
			std::string kidStatus = resetTree(*iter);
			if (kidStatus == "running"){
				nodeStatus = "running";
			}
		}

		if (nodeStatus != "running" || node == root){
			node->status = "ready";
			retString = "ready";
		}
		else{
			retString = "running";
		}
	}
	else if (node->nodeType == "actionNode"){
		if (node->status != "running"){
			node->status = "ready";
			retString = "ready";
		}
		else{
			retString = "running";
		}
	}

	return retString;
}

void behaviorTree::runTree(){
	while (1){
		root->execute();
		resetTree(root);
	}
}

void move(){
	while (1){
		DBOUT("determining if move is needed...\n");
		double dif = abs(targetPoint - playerPos);
		if (playerPos < targetPoint && dif > 5){
			MoveRight();
		}
		else if (targetPoint < playerPos && dif > 5){
			MoveLeft();
		}
		else if (abs(playerPos - targetPoint) < 5){
			stopAllMovemeent();
		}
	}
}