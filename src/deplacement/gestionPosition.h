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
#define _MARCHE_AVANT	0
#define _MARCHE_ARRIERE	1


////////////////////////////////////////////////////////////////////////////////
/// \fn int calculDeplacement(Robot* robot);
/// \param [in] robot : la structure décrivant le robot utilisé
/// \return 1 si la position est atteinte, 0 sinon
/// \brief Cette fonction est appelée lorsqu'on souhaite que le robot aille à une position (x;y) précise.
////////////////////////////////////////////////////////////////////////////////
int calculDeplacement(Robot* robot);

////////////////////////////////////////////////////////////////////////////////
/// \fn void premierAppelTenirAngle ( Robot* robot , int32_t orientation, int32_t vitesse, int32_t acc, int32_t decel, int32_t * posGauche, int32_t * posDroite);
/// \param [in] robot : la structure décrivant le robot utilisé
/// \param [in] orientation : Le cap que le robot doit maintenir
/// \param [in] vitesse : explicite (en mm/s)
/// \param [in] acc : accélération du robot (en mm/s²)
/// \param [in] decel : décélération du robot (en mm/s²)
/// \param [in] posGauche : pointeur allant contenir la valeur du nombre de tic codeur à atteindre pour la roue gauche
/// \param [in] posDroite : pointeur allant contenir la valeur du nombre de tic codeur à atteindre pour la roue droite
/// \brief Afin de demander au robot de se tourner en direction d'un cap précis, on appelle cette fonction une première fois. 
/// \brief	La fonction calcule l'erreur sur le cap à corriger en envoie une commande à la carte moteur pour se déplacer comme il faut
////////////////////////////////////////////////////////////////////////////////
void premierAppelTenirAngle ( Robot* robot , int32_t orientation, int32_t vitesse, int32_t acc, int32_t decel, int32_t * posGauche, int32_t * posDroite);

////////////////////////////////////////////////////////////////////////////////
/// \fn int tenirAngle ( Robot* robot , int32_t posGauche, int32_t posDroite, int32_t tolerance);
/// \param [in] robot : la structure décrivant le robot utilisé
/// \param [in] posGauche : valeur du nombre de tic codeur à atteindre pour la roue gauche
/// \param [in] posDroite : valeur du nombre de tic codeur à atteindre pour la roue droite
/// \param [in] tolerance : Tolérance pour déterminer si le cap à atteindre est bon ou non
/// \return 1 si le cap est bon, 0 sinon
/// \brief Cette fonction permet de déterminer si le cap actuel du robot correspond au cap demandé.
////////////////////////////////////////////////////////////////////////////////
int tenirAngle ( Robot* robot , int32_t posGauche, int32_t posDroite, int32_t tolerance);

////////////////////////////////////////////////////////////////////////////////
/// \fn void premierAppelMouvement(Robot* robot, int type, int value, int32_t vitesse, int32_t acc, int32_t decel, int32_t * posGauche, int32_t * posDroite);
/// \param [in] robot : la structure décrivant le robot utilisé
/// \param [in] type : type de mouvement demandé, peut être tournerDe ou avancerDe
/// \param [in] value : Angle (en °) ou distance (en mm/s) demandé
/// \param [in] vitesse : explicite (en mm/s)
/// \param [in] acc : accélération du robot (en mm/s²)
/// \param [in] decel : décélération du robot (en mm/s²)
/// \param [in] posGauche : pointeur allant contenir la valeur du nombre de tic codeur à atteindre pour la roue gauche
/// \param [in] posDroite : pointeur allant contenir la valeur du nombre de tic codeur à atteindre pour la roue droite
/// \brief Afin de demander au robot de se tourner en direction d'un cap précis, on appelle cette fonction une première fois. 
/// \brief La fonction calcule le nombre de tics codeurs que chaque roue va devoir réaliser pour effectuer le déplacement.
/// \brief On envoie ensuite une commande à la carte moteur pour réaliser ce déplacement.
////////////////////////////////////////////////////////////////////////////////
void premierAppelMouvement(Robot* robot, int type, int value, int32_t vitesse, int32_t acc, int32_t decel, int32_t * posGauche, int32_t * posDroite);

////////////////////////////////////////////////////////////////////////////////
/// \fn int setMouvement(Robot* robot, int type, int32_t posGauche, int32_t posDroite, int32_t tolerance);
/// \param [in] robot : la structure décrivant le robot utilisé
/// \param [in] type : type de mouvement demandé, peut être tournerDe ou avancerDe
/// \param [in] posGauche : valeur du nombre de tic codeur à atteindre pour la roue gauche
/// \param [in] posDroite : valeur du nombre de tic codeur à atteindre pour la roue droite
/// \param [in] tolerance : Tolérance pour déterminer si le déplacement à atteindre est bon ou non
/// \return 1 si le déplacement est bon, 0 sinon
/// \brief Cette fonction permet de déterminer si le déplacement effectué est conforme à celui demandé.
////////////////////////////////////////////////////////////////////////////////
int setMouvement(Robot* robot, int type, int32_t posGauche, int32_t posDroite, int32_t tolerance);

#endif //__GESTIONPOSITION_H__
