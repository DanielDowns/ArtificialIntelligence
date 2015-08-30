#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <Windows.h>
#include <algorithm>
#include <math.h>

#define WINVER 0x0500

std::string moveMode = "";
int fireKey = 0x42;

void MoveLeft(){
	if (moveMode == "right"){
		keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
	}

	if (moveMode != "left"){
		moveMode = "left";
		keybd_event(VK_LEFT, 0, 0, 0);
	}
}

void MoveRight(){
	if (moveMode == "left"){
		keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
	}

	if (moveMode != "right"){
		moveMode = "right";
		keybd_event(VK_RIGHT, 0, 0, 0);
	}
}

void stopAllMovemeent(){
	if (moveMode == "left"){
		keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
		moveMode = "";
	}
	else if (moveMode == "right"){
		keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
		moveMode = "";
	}
}

void fireShot(){
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press the "b" key
	ip.ki.wVk = fireKey; // virtual-key code for the "b" key
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));
}