#include "dodgealg.h"
#include "imageutilities.h"
#include <math.h>

#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s;                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

float point_enemy_reward(std::map<std::string, std::vector<XYposition>> pastObjects, float point){

	//###DONE BUT NOT TESTED

	const float OBJECT_SIZE = 40; //#rough pixel size of all enemies, should be swapped to actual
	const float OBJECT_REWARD = 20; //#random value of enemies, should be swapped to actual

	//complete
	for (std::map<std::string, std::vector<XYposition>>::iterator iter = pastObjects.begin();
		iter != pastObjects.end();++iter){
		if (iter->first != "missile"){
			for (XYposition item : iter->second){
				if (point > item.X - (OBJECT_SIZE / 2) && point < item.X + (OBJECT_SIZE/2)){
					return OBJECT_REWARD;
				}
			}
		}
	}
	
	return 0;
}

float point_threat(std::vector<Threat> threats, float point){

	//###DONE BUT NOT TESTED
	const float DANGER_POINT = 500;
	const float LARGE_CONSTANT = 100;
	float highest = 0;
//	DBOUT("\nthreat #:");
	int size = threats.size();
//	DBOUT(size);
//	DBOUT(">\n");

	for (Threat threat : threats){

		if (abs(threat.impactPoint - point) < 20){
		//	if (threat.yCoord > DANGER_POINT){ 
		//		return LARGE_CONSTANT / 3;
		//	}
		//	else{
				return LARGE_CONSTANT;
		//	}
		}
		/*b
		if ((threat.xCoord <= point && point <= threat.impactPoint) || threat.xCoord >= point && point >= threat.impactPoint){
			if (threat.timeTilImpact == 0){
				return LARGE_CONSTANT;;
			}
			float danger = LARGE_CONSTANT / threat.timeTilImpact;
			if (danger > highest){
				highest = danger;	
			}
		} */
	}

	return highest;
}

//not currently used
float point_travel_threat(){
	float sum_threat = 0;

	return sum_threat;
}

float point_distance(float point, float playerX){
	//###DONE BUT NOT TESTED

	return floor(abs(point - playerX) / 50);
}

float point_tactical_value(double point){
	//###DONE BUT NOT TESTED

	const float TACTICAL_MIN = 80;
	const float TACTICAL_MAX = 400;

	if (point > TACTICAL_MIN && point < TACTICAL_MAX){
		return 10;
	}
	else{
		return 0;
	}
}