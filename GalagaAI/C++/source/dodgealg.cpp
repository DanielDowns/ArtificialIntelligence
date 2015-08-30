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

	const float OBJECT_SIZE = 40; //#rough pixel size of all enemies
	const float OBJECT_REWARD = 20; //#random value of enemies

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

	const float DANGER_POINT = 500;
	const float LARGE_CONSTANT = 100;
	float highest = 0;

	int size = threats.size();

	for (Threat threat : threats){

		if (abs(threat.impactPoint - point) < 20){
			if (threat.yCoord > DANGER_POINT){ 
				return LARGE_CONSTANT / 3;
			}
			else{
				return LARGE_CONSTANT;
			}
		}
	}

	return highest;
}

//not currently used
float point_travel_threat(){
	float sum_threat = 0;

	return sum_threat;
}

float point_distance(float point, float playerX){

	return floor(abs(point - playerX) / 50);
}

float point_tactical_value(double point){

	const float TACTICAL_MIN = 80;
	const float TACTICAL_MAX = 400;

	if (point > TACTICAL_MIN && point < TACTICAL_MAX){
		return 10;
	}
	else{
		return 0;
	}
}