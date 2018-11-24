#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include "odometrie.h"

int32_t codeurGauchePrecedent;
int32_t codeurDroitPrecedent;

float orientationPre;

int32_t deltaComptG;
int32_t deltaComptD;

float dAngle, dDeplacement, dX, dY;

int calculPosition ( struct roboclaw* rc, Robot* robot )
{
	if ( !rc ||
		!robot )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	codeurGauchePrecedent = robot->codeurGauche;
	codeurDroitPrecedent = robot->codeurDroit;
	orientationPre = robot->orientationRobot;
	roboclaw_encoders ( rc,	0x80, &(robot->codeurGauche), &(robot->codeurDroit) );
	deltaComptG = robot->codeurGauche - codeurGauchePrecedent;
	deltaComptD = robot->codeurDroit - codeurDroitPrecedent;
	robot->orientationRobot += robot->coeffAngleG * deltaComptG - robot->coeffAngleD * deltaComptD;
	if ( robot->orientationRobot > 180. )
	{
		orientationPre -= 360.;
		robot->orientationRobot-=360.0;
	}
	else if ( robot->orientationRobot < -180.0 )
	{
		orientationPre += 360.0;
		robot->orientationRobot += 360.0;
	}

	dAngle = robot->coeffAngleG * deltaComptG - robot->coeffAngleD * deltaComptD;
	dDeplacement = ( robot->coeffLongD * deltaComptD + robot->coeffLongG * deltaComptG ) / 2.0;

	float tuMeCasseLesCouilles = cosf(45 * (double)M_PI  / 180);
	printf("%f %f \n",tuMeCasseLesCouilles, dAngle);
	tuMeCasseLesCouilles = sinf(45 * M_PI / 180.);
	printf("%f %f \n",tuMeCasseLesCouilles, dAngle);
	dX = dDeplacement * cosf ( ( dAngle * ( M_PI / 180. ) ) );
	dY = dDeplacement * sinf ( ( dAngle * ( M_PI / 180. ) ) );
	robot->distanceParcourue+=dDeplacement;
	//printf("%f %f \n",tuMeCasseLesCouilles, dAngle);
	robot->xRobot += dX;
	robot->yRobot += dY;

	return ( 0 );
}

int initOdometrie ( struct roboclaw* rc, Robot* robot )
{
	if ( !rc ||	!robot )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	robot->coeffLongG =	0.0489441484;
	robot->coeffLongD =	-0.0489296636;
	robot->coeffAngleG = 0.0108754758;
	robot->coeffAngleD = -0.0108584183;
	robot->vitesseGaucheDefault = 0.;
	robot->vitesseDroiteDefault = 0.;

	if ( roboclaw_encoders ( rc, 0x80, &(robot->codeurGauche), &(robot->codeurDroit) ) != ROBOCLAW_OK )
	{
		errno = ENETUNREACH;
		return ( __LINE__ );
	}

	codeurGauchePrecedent = robot->codeurGauche;
	codeurDroitPrecedent = robot->codeurDroit;
	orientationPre = robot->orientationRobot;
	printf("Init codeurs : %d %d \n",codeurGauchePrecedent,codeurDroitPrecedent);

	return ( 0 );
}
