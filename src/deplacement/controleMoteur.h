#ifndef __CONTROLEMOTEUR_H__
#define __CONTROLEMOTEUR_H__

#include <stdint.h>

#include "../lib/roboclaw/roboclaw.h"
#include "../struct/structRobot.h"

void envoiOrdreMoteur ( struct roboclaw* rc, Robot* robot, int16_t limitSpeed );

#endif //__CONTROLEMOTEUR_H__
