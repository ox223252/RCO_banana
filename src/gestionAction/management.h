#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include "action.h"
#include "../struct/structAction.h"
#include "../struct/structRobot.h"
#include "../deplacement/gestionPosition.h"
#include "../deplacement/detectionBlocage.h"

int initAction ( ActionFlag *flag );
void actionSetFd ( int pca9685 );
void gestionAction ( Action* listAction, Robot* robot, int indiceAction );
int updateActionEnCours ( Action* listAction, int nbAction, Robot* robot );

#endif //MANAGEMENT_H
