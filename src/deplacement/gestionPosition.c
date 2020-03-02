#include "gestionPosition.h"
#include "controleMoteur.h"

#define dX ( robot->cible.xCible - robot->xRobot )
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
	resetBlocage();
	_gestionPosition_distanceCiblePre = pytagor ( dX, dY );
	robot->orientationVisee = acos ( ( robot->cible.xCible - robot->xRobot ) / _gestionPosition_distanceCiblePre ) * 360. / ( 2. * M_PI );
	printf("Clef : %d \n",robot->memKey);
	if(robot->cible.sens == 0)
	{
		robot->detection->dir = DIR_FORWARD;
	}else
	{
		robot->detection->dir = DIR_BACKWARD;
	}
	
	if ( robot->cible.yCible < robot->yRobot )
	{
		robot->orientationVisee = -1. * robot->orientationVisee;
	}
}

void premierAppelTenirAngle ( Robot* robot , int32_t orientation, int32_t vitesse, int32_t acc, int32_t decel, int32_t * posGauche, int32_t * posDroite)
{
	erreurAngle = minimumErreur2Angles ( robot->orientationRobot, orientation );
	erreurAngle /= 2;
	posGauche = robot->codeurGauche - ( erreurAngle / robot->coeffAngleG)
	posDroite = robot->codeurDroit + ( erreurAngle / robot->coeffAngleD)
	envoiOrdrePositionMoteurs(acc, vitesse, decel, posGauche, acc, speed, decel, posDroite);
}

int calculDeplacement(Robot* robot)
{	

	float distanceCible;
	float erreurAngle;

	distanceCible = pytagor ( dX, dY );

	if ( distanceCible <= robot->cible.precision )
	{
		razAsserv();
		return 1;
	}
	robot->orientationVisee = acos ( ( robot->cible.xCible - robot->xRobot ) / distanceCible ) * 360. / ( 2. * M_PI );

	if ( robot->cible.yCible < robot->yRobot )
	{
		robot->orientationVisee = -1. * robot->orientationVisee;
	}
	if(robot->cible.sens == 1)
	{
		if(robot->orientationVisee > 0)
		{
			robot->orientationVisee-=180.;
		}else
		{
			robot->orientationVisee+=180.;
		}
	}

	erreurAngle = minimumErreur2Angles ( robot->orientationRobot,robot->orientationVisee );

	
	
	if(robot->setDetection == 1)
	{
		if(robot->detection->distance <= 350 && robot->detection->distance >= 180)
		{
			/*robot->vitesseDroiteToSend = 0;
			robot->vitesseGaucheToSend = 0;
			razAsserv();*/
		}else if(robot->detection->distance <= 550 && robot->detection->distance >= 350)
		{
			//_gestionPosition_pourcentageVitesse /= 2;
			
		}else
		{
			/*robot->vitesseGaucheToSend *= ( _gestionPosition_pourcentageVitesse / 100 );
			robot->vitesseDroiteToSend *= ( _gestionPosition_pourcentageVitesse / 100 );
			robot->vitesseDroiteToSend -= 5.0*erreurAngle;
			robot->vitesseGaucheToSend += 5.0*erreurAngle;*/
		}
	}else
	{
		if(erreurAngle > 100)erreurAngle = 100;
		else if(erreurAngle < -100)erreurAngle = -100;

		//posGauche = robot->codeurGauche - ( erreurAngle / robot->coeffAngleG)
		//posDroite = robot->codeurDroit + ( erreurAngle / robot->coeffAngleD)
		if(erreurAngle > 0 )
		{
			robot->vitesseGaucheToSend = robot->cible.vitesseMax - ( robot->cible.vitesseMax / erreurAngle );
			robot->vitesseDroiteToSend = robot->cible.vitesseMax;
		}else
		{
			robot->vitesseGaucheToSend = robot->cible.vitesseMax;
			robot->vitesseDroiteToSend = robot->cible.vitesseMax - ( robot->cible.vitesseMax / erreurAngle );
		}

		if ( robot->cible.sens == 1 )
		{
			//marche arriere
			robot->vitesseGaucheToSend *= -1.;
			robot->vitesseDroiteToSend *= -1.;
		}
		

		envoiOrdrePositionMoteurs(robot->cible.acc, robot->vitesseGaucheToSend, robot->cible.decel, robot->codeurGauche + ( distanceCible / robot->coeffLongG), 
			robot->cible.acc, robot->vitesseDroiteToSend, robot->cible.decel, robot->codeurDroit + ( distanceCible / robot->coeffLongD));
	}	

	return 0;
}

int tenirAngle ( Robot* robot , int32_t posGauche, int32_t posDroite, int32_t tolerance)
{
	if( abs(robot->codeurGauche - posGauche) < tolerance / 2 / robot->coeffAngleG && abs(robot->codeurDroit - posDroite) < tolerance / 2 / robot->coeffAngleD)
	{
		return 1;
	}

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

void premierAppelMouvement(Robot* robot, int type, int value, int32_t vitesse, int32_t acc, int32_t decel, int32_t * posGauche, int32_t * posDroite)
{
	switch (type)
	{
		case _AVANCE_DE:
		{
			posGauche = robot->codeurGauche + ( value / robot->coeffLongG)
			posDroite = robot->codeurDroit + ( value / robot->coeffLongD)
			break;
		}
		case _TOURNER_DE:
		{
			value /= 2;
			posGauche = robot->codeurGauche - ( value / robot->coeffAngleG)
			posDroite = robot->codeurDroit + ( value / robot->coeffAngleD)
			break;
		}
		default: 
		break;
	}
	envoiOrdrePositionMoteurs(acc, vitesse, decel, posGauche, acc, vitesse, decel, posDroite);
}

int setMouvement(Robot* robot, int32_t posGauche, int32_t posDroite, int32_t tolerance)
{
switch (type)
	{
		case _AVANCE_DE:
		{
			if( abs(robot->codeurGauche - posGauche) < tolerance / robot->coeffLongG && abs(robot->codeurDroit - posDroite) < tolerance / robot->coeffLongD)
			{
				return 1;
			}
			break;
		}
		case _TOURNER_DE:
		{
			if( abs(robot->codeurGauche - posGauche) < tolerance / 2 / robot->coeffAngleG && abs(robot->codeurDroit - posDroite) < tolerance / 2 / robot->coeffAngleD)
			{
				return 1;
			}
			break;
		}
		default: 
		break;
	}

	return 0;
}