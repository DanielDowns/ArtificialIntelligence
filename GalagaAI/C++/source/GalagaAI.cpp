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

//http://game-oldies.com/play-online/galaga-coin-op-arcade#
int _tmain(int argc, _TCHAR* argv[])
{
	
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

	moveNode * move = new moveNode(&AI);
	move->nodeName = "move";
	fireNode * fire = new fireNode(&AI);
	fire->nodeName = "fire";
	
	tree.root = &AI;
	tree.runTree(); 
	
	return 0;
}
