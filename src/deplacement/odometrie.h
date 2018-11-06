#ifndef __ODOMETRIE_H__
#define __ODOMETRIE_H__

#include "../lib/roboclaw/roboclaw.h"
#include "../struct/structRobot.h"

int initOdometrie ( struct roboclaw* rc, Robot* robot );
int calculPosition ( struct roboclaw* rc, Robot* robot );

#endif
