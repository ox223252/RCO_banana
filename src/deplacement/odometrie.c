#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>

#include "../lib/log/log.h"

#include "odometrie.h"

static struct
{
	int32_t left;
	int32_t right;
	float angle;
}
_odometrie_old;

int calculPosition ( struct roboclaw* rc, Robot* robot )
{
	static float dAngle;
	static float dDeplacement;
	static int32_t deltaComptG;
	static int32_t deltaComptD;

	if ( !rc ||
		!robot )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	_odometrie_old.left = robot->codeurGauche;
	_odometrie_old.right = robot->codeurDroit;
	_odometrie_old.angle = robot->orientationRobot;

	roboclaw_encoders ( rc, 0x80, &(robot->codeurGauche), &(robot->codeurDroit) );

	deltaComptG = robot->codeurGauche - _odometrie_old.left;
	deltaComptD = robot->codeurDroit - _odometrie_old.right;

	robot->orientationRobot += robot->coeffAngleG * deltaComptG - robot->coeffAngleD * deltaComptD;

	if ( robot->orientationRobot > 180. )
	{
		_odometrie_old.angle -= 360.;
		robot->orientationRobot-=360.0;
	}
	else if ( robot->orientationRobot < -180.0 )
	{
		_odometrie_old.angle += 360.0;
		robot->orientationRobot += 360.0;
	}

	dAngle = (robot->orientationRobot + _odometrie_old.angle) / 2.;
	dDeplacement = ( robot->coeffLongD * deltaComptD + robot->coeffLongG * deltaComptG ) / 2.0;

	robot->xRobot += dDeplacement * cos ( ( dAngle * ( M_PI / 180. ) ) );
	robot->yRobot += dDeplacement * sin ( ( dAngle * ( M_PI / 180. ) ) );
	
	robot->distanceParcourue += dDeplacement;

	return ( 0 );
}

int initOdometrie ( struct roboclaw* rc, Robot* robot )
{
	if ( !rc ||	!robot )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	robot->vitesseGaucheDefault = 0.;
	robot->vitesseDroiteDefault = 0.;

	if ( roboclaw_encoders ( rc, 0x80, &(robot->codeurGauche), &(robot->codeurDroit) ) != ROBOCLAW_OK )
	{
		errno = ENETUNREACH;
		return ( __LINE__ );
	}

	_odometrie_old.left = robot->codeurGauche;
	_odometrie_old.right = robot->codeurDroit;
	_odometrie_old.angle = robot->orientationRobot;

	logDebug ( "Init codeurs : %d %d\n", _odometrie_old.left, _odometrie_old.right );

	return ( 0 );
}
