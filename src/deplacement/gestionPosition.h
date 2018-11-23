#ifndef __GESTIONPOSITION_H__
#define __GESTIONPOSITION_H__
#include "../struct/structRobot.h"
#include <math.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>

void premierAppel(Robot* robot);
void premierAppelTenirAngle(Robot* robot);
int calculDeplacement(Robot* robot);
int tenirAngle(Robot* robot);

#endif //__GESTIONPOSITION_H__
