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

	robot->orientationRobot += robot->coeffAngleD * deltaComptD - robot->coeffAngleG * deltaComptG;
	if ( robot->orientationRobot > 180 )
	{
		orientationPre -= 360;
		robot->orientationRobot-=360.0;
	}
	else if ( robot->orientationRobot < -180.0 )
	{
		orientationPre += 360.0;
		robot->orientationRobot += 360.0;
	}

	dAngle = ( robot->orientationRobot + orientationPre ) / 2.0;
	dDeplacement = ( robot->coeffLongD * deltaComptD + robot->coeffLongG * deltaComptG ) / 2.0;
	dX = dDeplacement * cos ( ( double )( dAngle * ( M_PI / 180 ) ) );
	dY = dDeplacement * sin ( ( double )( dAngle * ( M_PI / 180 ) ) );

	robot->distanceParcourue+=dDeplacement;

	robot->xRobot += dX;
	robot->yRobot += dY;

	return ( 0 );
}

int initOdometrie ( struct roboclaw* rc, Robot* robot )
{
	if ( !rc ||
		!robot )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	robot->coeffLongG =	-0.0489441484;
	robot->coeffLongD =	0.0489296636;
	robot->coeffAngleG = -0.0108754758;
	robot->coeffAngleD = 0.0108584183;

	if ( roboclaw_encoders ( rc, 0x80, &(robot->codeurGauche), &(robot->codeurDroit) ) != ROBOCLAW_OK )
	{
		errno = ENETUNREACH;
		return ( __LINE__ );
	}

	codeurGauchePrecedent = robot->codeurGauche;
	codeurDroitPrecedent = robot->codeurDroit;
	orientationPre = robot->orientationRobot;

	return ( 0 );
}
