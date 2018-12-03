#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include "../lib/log/log.h"

#include "odometrie.h"

static struct
{
	struct timeval date;
	int32_t left;
	int32_t right;
	float angle;
}
_odometrie_old;


int calculPosition ( struct roboclaw* rc, Robot* robot )
{
	static struct timeval now;
	static float tempsEcoule;
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
	// save olds values
	_odometrie_old.left = robot->codeurGauche;
	_odometrie_old.right = robot->codeurDroit;
	_odometrie_old.angle = robot->orientationRobot;

	// request new ones
	roboclaw_encoders ( rc, 0x80, &( robot->codeurGauche ), &( robot->codeurDroit ) );

	// get new date
	gettimeofday ( &now, NULL );

	// calc moves
	deltaComptG = robot->codeurGauche - _odometrie_old.left;
	deltaComptD = robot->codeurDroit - _odometrie_old.right;

	// calc robot angle
	robot->orientationRobot += ( robot->coeffAngleG * deltaComptG ) - ( robot->coeffAngleD * deltaComptD );

	if ( robot->orientationRobot > 180. )
	{
		_odometrie_old.angle -= 360.;
		robot->orientationRobot -=360.;
	}
	else if ( robot->orientationRobot < -180. )
	{
		_odometrie_old.angle += 360.;
		robot->orientationRobot += 360.;
	}

	// calc delta angle
	dAngle = ( robot->orientationRobot + _odometrie_old.angle ) / 2.;

	// calc real move
	dDeplacement = ( ( robot->coeffLongD * deltaComptD ) + robot->coeffLongG * deltaComptG ) / 2.;
	robot->distanceParcourue += dDeplacement;

	// calc X / Y
	robot->xRobot += dDeplacement * cos ( ( dAngle * ( M_PI / 180. ) ) );
	robot->yRobot += dDeplacement * sin ( ( dAngle * ( M_PI / 180. ) ) );

	// calc delta time
	tempsEcoule = ( ( now.tv_sec * 1000000 + now.tv_usec ) - ( _odometrie_old.date.tv_sec * 1000000 + _odometrie_old.date.tv_usec ) ) / 1000000. ;

	// save olds value
	memcpy ( &_odometrie_old.date,  &now, sizeof ( struct timeval ) );

	// calc speed
	if ( tempsEcoule )
	{
		robot->vitesseGauche = ( deltaComptG * robot->coeffAngleG ) / tempsEcoule;
		robot->vitesseDroite = ( deltaComptD * robot->coeffAngleD ) / tempsEcoule;
		robot->vitesseAngulaire = dAngle / tempsEcoule;
	}
	else
	{
		robot->vitesseGauche = 0.;
		robot->vitesseDroite = 0.;
		robot->vitesseAngulaire = 0.;
	}

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
	robot->vitesseGauche = 0.;
	robot->vitesseDroite = 0.;

	if ( roboclaw_encoders ( rc, 0x80, &(robot->codeurGauche), &(robot->codeurDroit) ) != ROBOCLAW_OK )
	{
		errno = ENETUNREACH;
		return ( __LINE__ );
	}

	_odometrie_old.left = robot->codeurGauche;
	_odometrie_old.right = robot->codeurDroit;
	_odometrie_old.angle = robot->orientationRobot;

	gettimeofday ( &_odometrie_old.date,  NULL );

	logDebug ( "Init codeurs : %d %d\n", _odometrie_old.left, _odometrie_old.right );

	return ( 0 );
}
