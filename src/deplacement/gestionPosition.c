#include "gestionPosition.h"
#include "controleMoteur.h"
#include <stdio.h>
#include <stdlib.h>

#include "../lib/log/log.h"

#define dX ( robot->cible.xCible - robot->xRobot )
#define dY ( robot->cible.yCible - robot->yRobot )

static float minimumErreur2Angles ( float angle1, float angle2 );

//We use this function to calculate the distance between the robot and his target
double pytagor ( double x, double y );
#define pytagor(x,y) ( sqrt ( ( x ) * ( x ) + ( y ) * ( y ) ) )

int calculDeplacement(Robot* robot)
{
	float distanceCible;
	float erreurAngle;


	distanceCible = pytagor ( dX, dY );
	//On commence par calculer la distance de la cible du point de vue du robot.
	//Si celle-ci est inférieure à la tolérance acceptée, alors on estime que le robot est arrivé au point voulu.
	if ( distanceCible <= robot->cible.precision )
	{
		return 1;
	}

	//Si le robot n'est pas encore arrivé sur sa cible, on calcule alors le cap que le robot doit maintenir pour aller droit vers sa cible
	robot->orientationVisee = acos ( ( robot->cible.xCible - robot->xRobot ) / distanceCible ) * 360. / ( 2. * M_PI );

	//Le signe du cap à adopter est différent si la cible est à gauche ou à droite du robot, on doit donc adapter.
	//Pour cela, on regarde les ordonnées du robot et de la cible.
	if ( robot->cible.yCible < robot->yRobot )
	{
		//A droite du robot, l'angle doit être négatif
		robot->orientationVisee = -1. * robot->orientationVisee;
	}

	//Ensuite, en fonction du sens de déplacement du robot (marche avant ou arrière), on modifie le cap souhaité.
	//Ainsi, pour un déplacement en marche arrière, on va chercher à ce que le robot tourne le dos à la cible.
	if(robot->cible.sens == _MARCHE_ARRIERE)
	{
		if(robot->orientationVisee > 0)
		{
			robot->orientationVisee-=180.;
		}else
		{
			robot->orientationVisee+=180.;
		}
	}

	//On calcule ensuite l'erreur entre le cap du robot et le cap souhaité.
	//On cherche à ce que le déplacement soit toujours minimum, on calcule donc tous les angles possibles entre 2 caps.
	//On garde le plus petit écart.
	erreurAngle = minimumErreur2Angles ( robot->orientationVisee, robot->orientationRobot );

	//Ici : gestion de la détection, pour l'instant, c'est pas géré, et heureusement sinon vu le code, ça ferait bien du caca !
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
		//Si le robot ne détecte rien, on peut rouler tranquillement, c'est parti ! 

		//On commence par vérifier que l'erreur en angle (entre le cap à tenir et le cap actuel) n'est pas trop élevé.
		if(abs(erreurAngle) < 5)
		{
			//Afin de s'assurer que le robot ne dérive pas, on va corriger l'erreur sur le cap, celle-ci est faible (inférieur à 5°)
			//Pour cela, on va appliquer un pourcentage sur la vitesse à envoyer au roue dont on veut que le robot tourne.
			//Si on veut que le robot se tourne vers la droite (erreur > 0) on va réduire la vitesse de la roue droite.
			//Inversement si on veut tourner vers la gauche (erreur < 0) 
			if(erreurAngle < 0 )
			{
				robot->vitesseGaucheToSend = (1. + (erreurAngle / 100. )) * robot->cible.vitesseMax ;
				robot->vitesseDroiteToSend = robot->cible.vitesseMax;
			}else
			{
				robot->vitesseGaucheToSend = robot->cible.vitesseMax;
				robot->vitesseDroiteToSend = (1. - (erreurAngle / 100. )) * robot->cible.vitesseMax ;
			}

			//Afin que le robot comprenne qu'il doit réaliser une marche arrière, on lui indique que la distance à parcourir est négative
			if ( robot->cible.sens == _MARCHE_ARRIERE )
			{
				//marche arriere
				distanceCible *= -1.;
			}

			//enfin, on envoie une commande à la carte moteur de réaliser autant de tic codeurs que nécessaire pour arriver à la cible.
			//Cette fonction sera rappelée tant que la cible n'est pas atteinte, le nombre de tics à réaliser sera donc régulièrement mis à jour.
			envoiOrdrePositionMoteurs(robot->cible.acc / robot->coeffLongG, 
			robot->vitesseGaucheToSend / robot->coeffLongG, 
			robot->cible.dec / robot->coeffLongG, 
			robot->codeurGauche + ( distanceCible / robot->coeffLongG),

			robot->cible.acc / robot->coeffLongD, 
			robot->vitesseDroiteToSend / robot->coeffLongD, 
			robot->cible.dec / robot->coeffLongD, 
			robot->codeurDroit + ( distanceCible / robot->coeffLongD));
		}else
		{

			//Si l'erreur est trop élevée, le robot commence par tourner sur lui-même pour se mettre bien en face de la cible.
			//Pour cela, on retrouve le même fonctionnement que pour la fonction tenirAngle.
			robot->vitesseDroiteToSend = robot->cible.vitesseMax;
			robot->vitesseGaucheToSend = robot->cible.vitesseMax;

			envoiOrdrePositionMoteurs(robot->cible.acc / robot->coeffLongG, 
			robot->vitesseGaucheToSend / robot->coeffLongG, 
			robot->cible.dec / robot->coeffLongG, 
			robot->codeurGauche - ( erreurAngle / 2. / robot->coeffAngleG),

			robot->cible.acc / robot->coeffLongD, 
			robot->vitesseDroiteToSend / robot->coeffLongD, 
			robot->cible.dec / robot->coeffLongD, 
			robot->codeurDroit + ( erreurAngle / 2. / robot->coeffAngleD));
		}		
		
	}	

	return 0;
}

void premierAppelTenirAngle ( Robot* robot , int32_t orientation, int32_t vitesse, int32_t acc, int32_t decel, int32_t * posGauche, int32_t * posDroite)
{
	//Afin de tenir un cap, on commence par calculer l'erreur entre le cap actuel et celui désiré.
	float erreurAngle = minimumErreur2Angles (orientation,  robot->orientationRobot );
	erreurAngle /= 2.;
	//On divise cette erreur par deux, en effet on veut que le robot tourne sur lui même.
	//La roue gauche doit donc faire - 1/2 l'erreur d'angle et la roue droite + 1/2 de l'erreur
	//le cumul des deux demi-déplacement fera que le robot tourne sur lui-même de l'angle désiré.
	*posGauche = robot->codeurGauche - ( erreurAngle / robot->coeffAngleG);
	*posDroite = robot->codeurDroit + ( erreurAngle / robot->coeffAngleD);
	envoiOrdrePositionMoteurs(acc, vitesse, decel, *posGauche, acc, vitesse, decel, *posDroite);
}

int tenirAngle ( Robot* robot , int32_t posGauche, int32_t posDroite, int32_t tolerance)
{
	//On vérifie que le nombre de tics du robot correspond à celui attendu
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
		case _AVANCER_DE:
		{
			*posGauche = robot->codeurGauche + ( value / robot->coeffLongG);
			*posDroite = robot->codeurDroit + ( value / (robot->coeffLongD));
			break;
		}
		case _TOURNER_DE:
		{
			value /= 2;
			*posGauche = robot->codeurGauche - ( value / robot->coeffAngleG);
			*posDroite = robot->codeurDroit + ( value / robot->coeffAngleD);
			break;
		}
		default: 
		break;
	}

	envoiOrdrePositionMoteurs(
		acc / robot->coeffLongG, 
		vitesse / robot->coeffLongG, 
		decel / robot->coeffLongG, 
		*posGauche, 
		acc /( robot->coeffLongD), 
		vitesse / (robot->coeffLongD), 
		decel / (robot->coeffLongD), 
		*posDroite);
}

int setMouvement(Robot* robot, int type, int32_t posGauche, int32_t posDroite, int32_t tolerance)
{
	switch (type)
	{
		case _AVANCER_DE:
		{			
			if( abs(robot->codeurGauche - posGauche) < tolerance / robot->coeffLongG && abs(robot->codeurDroit - posDroite) < tolerance /( robot->coeffLongD))
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