#ifndef __GESTIONPOSITION_H__
#define __GESTIONPOSITION_H__
#include "../struct/structRobot.h"

#include <math.h>
#include <errno.h>
#include "../lib/sharedMem/sharedMem.h"
#include <sys/time.h>
#include <time.h>

#define _AVANCER_DE	0
#define _TOURNER_DE	1

void premierAppelTenirAngle ( Robot* robot , int32_t orientation, int32_t vitesse, int32_t acc, int32_t decel, int32_t * posGauche, int32_t * posDroite);
int calculDeplacement(Robot* robot);
int tenirAngle ( Robot* robot , int32_t posGauche, int32_t posDroite, int32_t tolerance);
void premierAppelMouvement(Robot* robot, int type, int value, int32_t vitesse, int32_t acc, int32_t decel, int32_t * posGauche, int32_t * posDroite);
int setMouvement(Robot* robot, int type, int32_t posGauche, int32_t posDroite, int32_t tolerance);

#endif //__GESTIONPOSITION_H__
