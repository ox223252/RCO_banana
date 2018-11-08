#ifndef MANAGEMENT_H
#define MANAGEMENT_H

#include "action.h"
#include "../struct/structAction.h"
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

int initAction ( void );
void gestionAction(Action* listAction, Robot* robot, int indiceAction);
int updateActionEnCours ( Action* listAction, int nbAction, Robot* robot );

#endif //MANAGEMENT_H
