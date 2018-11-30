#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include "../lib/log/log.h"

#include "odometrie.h"

static struct
{
	int32_t left;
	int32_t right;
	float angle;
}
_odometrie_old;

static struct timeval _odometrie_now;
static struct timeval _odometrie_pre;
static float _odometrie_tempsEcoule;

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
	gettimeofday(&_odometrie_now, NULL);
	roboclaw_encoders(rc, 0x80, &(robot->codeurGauche), &(robot->codeurDroit) );

	deltaComptG = robot->codeurGauche - _odometrie_old.left;
	deltaComptD = robot->codeurDroit - _odometrie_old.right;

	_odometrie_tempsEcoule = ( _odometrie_now.tv_sec * 1000000 + _odometrie_now.tv_usec ) - ( _odometrie_pre.tv_sec * 1000000 + _odometrie_pre.tv_usec );
	gettimeofday(&_odometrie_pre, NULL);
	robot->vitesseGauche = deltaComptG*robot->coeffAngleG / (_odometrie_tempsEcoule/1000000.);
	robot->vitesseDroite = deltaComptD*robot->coeffAngleD / (_odometrie_tempsEcoule/1000000.);

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
	robot->vitesseAngulaire = dAngle / (_odometrie_tempsEcoule/1000000.);
	
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
	gettimeofday ( &_odometrie_pre,  NULL );

	logDebug ( "Init codeurs : %d %d\n", _odometrie_old.left, _odometrie_old.right );

	return ( 0 );
}
