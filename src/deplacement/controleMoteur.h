#ifndef __CONTROLEMOTEUR_H__
#define __CONTROLEMOTEUR_H__

#include "../lib/roboclaw/roboclaw.h"
#include "../struct/structRobot.h"

void envoiOrdreMoteur(struct roboclaw* rc, Robot* robot);

#endif //__CONTROLEMOTEUR_H__
