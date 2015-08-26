#ifndef DODGEALG_H
#define DODGEALG_H

#include "imageutilities.h"
#include <map>

float point_enemy_reward(std::map<std::string, std::vector<XYposition>> pastObjects, float point);
float point_threat(std::vector<Threat> threats, float point);
float point_travel_threat();
float point_distance(float point, float playX);
float point_tactical_value(double point);

#endif