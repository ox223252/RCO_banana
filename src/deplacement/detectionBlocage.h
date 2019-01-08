#ifndef __DETECTIONBLOCAGE_H__
#define __DETECTIONBLOCAGE_H__
#include "../struct/structRobot.h"

void initDetectionBlocage();
int detectBlocage(Robot* robot, int seuilDetection);
void resetBlocage();
#endif //__DETECTIONBLOCAGE_H__
