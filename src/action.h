#ifndef __ACTION_H__
#define _8ACTION_H__

#include <stdint.h>

#define aT(type) ACTION_TYPE_##type

enum {
	ACTION_TYPE_none,
	ACTION_TYPE_servo,
	ACTION_TYPE_dyna,
	ACTION_TYPE_CAPTEUR,
	ACTION_TYPE_MOTEUR,
	ACTION_TYPE_AUTRE,
	ACTION_TYPE_POSITION,
	ACTION_TYPE_ORIENTATION,
	ACTION_TYPE_SEQUENCE,
	ACTION_TYPE_ENTREE,
	ACTION_TYPE_ATTENTE_SERVO,
	ACTION_TYPE_ATTENTE_DYNA,
	ACTION_TYPE_ATTENTE_TEMPS,
	ACTION_TYPE_RETOUR_DEPLACEMENT,
	ACTION_TYPE_RETOUR_ORIENTATION,
	ACTION_TYPE_RETOUR_POSITION,
	ACTION_TYPE_GPIO,
	ACTION_TYPE_RETOUR_GPIO,
	ACTION_TYPE_AND,
	ACTION_TYPE_SET_VALEUR,
	ACTION_TYPE_COURBE,
	ACTION_TYPE_ATTENTE_BLOCAGE,
	ACTION_TYPE_DEPLACEMENT,
	ACTION_TYPE_FIN,
	ACTION_TYPE_set_var,
	ACTION_TYPE_get_var,
	ACTION_TYPE_last
};

int actionManagerInit ( const char * const file );
int actionManagerDeInit ( void );

int actionManagerUpdate ( void );
int actionStartStep ( void );

void actionManagerPrint ( void );
void actionManagerPrintCurrent ( void );

#endif
