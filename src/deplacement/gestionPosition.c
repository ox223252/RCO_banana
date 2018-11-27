#include "gestionPosition.h"

#define dX ( robot->cible.yCible - robot->yRobot )
#define dY ( robot->cible.yCible - robot->yRobot )

static float minimumErreur2Angles ( float angle1, float angle2 );

static float _gestionPosition_erreurAnglePre = 0.;
static float _gestionPosition_distanceCiblePre = 0.;
static float _gestionPosition_tempsEcoule;
static float _gestionPosition_pourcentageVitesse;
static struct timeval _gestionPosition_now;
static struct timeval _gestionPosition_pre;

/// \brief return the length.
double pytagor ( double x, double y );
#define pytagor(x,y) ( sqrt ( ( x ) * ( x ) + ( y ) * ( y ) ) )

void premierAppel ( Robot* robot )
{
	premierAppelTenirAngle ( robot );
	_gestionPosition_distanceCiblePre = pytagor ( dX, dY );
	robot->orientationVisee = acos ( ( robot->cible.xCible - robot->xRobot ) / _gestionPosition_distanceCiblePre ) * 360. / ( 2. * M_PI );

	if ( robot->cible.yCible < robot->yRobot )
	{
		robot->orientationVisee = -1. * robot->orientationVisee;
	}
	premierAppelTenirAngle ( robot );
}

void premierAppelTenirAngle ( Robot* robot )
{
	gettimeofday ( &_gestionPosition_pre,  NULL );
	_gestionPosition_erreurAnglePre = minimumErreur2Angles ( robot->orientationRobot,robot->orientationVisee );
}

int calculDeplacement ( Robot* robot )
{
	/*
	Le robot sait où il est, il connait la position de la cible :
	il suffit de calculer la distance de la cible et la consigne d'orientation
	*/

	float distanceCible;
	float erreurAngle;

	distanceCible = pytagor ( dX, dY );
	if ( distanceCible <= robot->cible.precision )
	{
		return 1;
	}
	robot->orientationVisee = acos ( ( robot->cible.xCible - robot->xRobot ) / distanceCible ) * 360. / ( 2. * M_PI );

	if ( robot->cible.yCible < robot->yRobot )
	{
		robot->orientationVisee = -1. * robot->orientationVisee;
	}

	erreurAngle = minimumErreur2Angles ( robot->orientationRobot,robot->orientationVisee );

	if ( robot->cible.sens == 1 )
	{
		//marche arriere
		robot->vitesseGaucheToSend = -1. * distanceCible;
		robot->vitesseDroiteToSend = -1. * distanceCible;
	}
	else
	{
		robot->vitesseGaucheToSend = 1. * distanceCible;
		robot->vitesseDroiteToSend = 1. * distanceCible;
	}

	robot->vitesseDroiteToSend += 10.*erreurAngle;
	robot->vitesseGaucheToSend -= 10.*erreurAngle;

	gettimeofday ( &_gestionPosition_now,  NULL );
	_gestionPosition_tempsEcoule = ( _gestionPosition_now. tv_sec * 1000000 + _gestionPosition_now. tv_usec ) - ( _gestionPosition_pre. tv_sec * 1000000 + _gestionPosition_pre. tv_usec );

	//1000000 sec = 1000000 acc
	//x sec
	if ( ( _gestionPosition_pourcentageVitesse + _gestionPosition_tempsEcoule * robot->cible.acc ) < robot->cible.vitesseMax )
	{
		_gestionPosition_pourcentageVitesse += _gestionPosition_tempsEcoule * robot->cible.acc;
	}
	robot->vitesseGaucheToSend *= ( _gestionPosition_pourcentageVitesse / 100 );
	robot->vitesseDroiteToSend *= ( _gestionPosition_pourcentageVitesse / 100 );

	return 0;
}

int tenirAngle ( Robot* robot )
{
	float erreurAngle;
	erreurAngle = minimumErreur2Angles ( robot->orientationRobot,robot->orientationVisee );

	if ( fabs ( erreurAngle ) <= robot->cible.precision )
	{
		return 1;
	}
	gettimeofday ( &_gestionPosition_now,  NULL );
	_gestionPosition_tempsEcoule = ( _gestionPosition_now. tv_sec * 1000000 + _gestionPosition_now. tv_usec ) - ( _gestionPosition_pre. tv_sec * 1000000 + _gestionPosition_pre. tv_usec );


	robot->vitesseDroiteToSend = 10.*erreurAngle;
	robot->vitesseGaucheToSend = 10.*erreurAngle;
	
	robot->vitesseGaucheToSend *= ( robot->cible.vitesseMax / 100 );
	robot->vitesseDroiteToSend *= ( robot->cible.vitesseMax / 100 );
	return 0;
}

static float minimumErreur2Angles ( float angle1, float angle2 )
{
	float complementAngle1, complementAngle2;
	float erreur1,erreur2,erreur3,erreur4;
	float returnValue = 0;

	if ( angle1 > 0 )
	{
		complementAngle1 = angle1 - 360.;
	}
	else
	{
		complementAngle1 = angle1 + 360.;
	}

	if ( angle2 > 0 )
	{
		complementAngle2 = angle2 - 360.;
	}
	else
	{
		complementAngle2 = angle2 + 360.;
	}

	erreur1 = angle1 - angle2;
	erreur2 = angle1 - complementAngle2;
	erreur3 = complementAngle1 - angle2;
	erreur4 = complementAngle1 - complementAngle2;

	if ( ( fabs ( erreur1 ) <= fabs ( erreur2 ) ) &&
		( fabs ( erreur1 ) <= fabs ( erreur3 ) ) &&
		( fabs ( erreur1 ) <= fabs ( erreur4 ) ) )
	{
		returnValue = erreur1;
	}

	if ( ( fabs ( erreur2 ) <= fabs ( erreur1 ) ) &&
		( fabs ( erreur2 ) <= fabs ( erreur3 ) ) &&
		( fabs ( erreur2 ) <= fabs ( erreur4 ) ) )
	{
		returnValue = erreur2;
	}

	if ( ( fabs ( erreur3 ) <= fabs ( erreur1 ) ) &&
		( fabs ( erreur3 ) <= fabs ( erreur2 ) ) &&
		( fabs ( erreur3 ) <= fabs ( erreur4 ) ) )
	{
		returnValue = erreur3;
	}

	if ( ( fabs ( erreur4 ) <= fabs ( erreur1 ) ) &&
		( fabs ( erreur4 ) <= fabs ( erreur2 ) ) &&
		( fabs ( erreur4 ) <= fabs ( erreur3 ) ) )
	{
		returnValue = erreur4;
	}

	return returnValue;
}
