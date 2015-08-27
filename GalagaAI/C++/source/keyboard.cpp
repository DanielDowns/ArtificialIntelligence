#include "stdafx.h"
#include "opencv2/core/core.hpp"
#include <iostream>
#include <Windows.h>
#include <algorithm>
#include <math.h>

#define WINVER 0x0500

#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

std::string moveMode = "";
int fireKey = 0x42;

void MoveLeft(){
	if (moveMode == "right"){
		keybd_event(VK_RIGHT, 0, KEYEVENTF_KEYUP, 0);
	}

	if (moveMode != "left"){
		moveMode = "left";
		keybd_event(VK_LEFT, 0, 0, 0);

		DBOUT("\nmoving left...\n");
	}
}

void MoveRight(){
	if (moveMode == "left"){
		keybd_event(VK_LEFT, 0, KEYEVENTF_KEYUP, 0);
	}

	if (moveMode != "right"){
		moveMode = "right";
		keybd_event(VK_RIGHT, 0, 0, 0);

		DBOUT("\nmoving right...\n");
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
	DBOUT("\nstopping...\n");
}

void fireShot(){
	INPUT ip;
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;
	ip.ki.dwExtraInfo = 0;

	// Press the "A" key
	ip.ki.wVk = fireKey; // virtual-key code for the "a" key
	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));
//	DBOUT("firing!\n")
}