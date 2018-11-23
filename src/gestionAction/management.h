#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include "action.h"
#include "../struct/structAction.h"
#include "../struct/structRobot.h"
#include "../deplacement/gestionPosition.h"

int initAction ( ActionFlag *flag );
void gestionAction ( Action* listAction, Robot* robot, int indiceAction );
int updateActionEnCours ( Action* listAction, int nbAction, Robot* robot );

#endif //MANAGEMENT_H
