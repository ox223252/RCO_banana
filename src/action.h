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

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerInit ( const char * const file );
/// \param [in] file : file name that contain the json
/// \brief parse the json and create needed elements
/// \return 0 if Ok, else see errno for more details
////////////////////////////////////////////////////////////////////////////////
int actionManagerInit ( const char * const file );

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerDeInit ( void );
/// \param [in] file : file name that contain the json
/// \brief free all elements created by action* functions
/// \return 0 if Ok, else see errno for more details
////////////////////////////////////////////////////////////////////////////////
int actionManagerDeInit ( void );



////////////////////////////////////////////////////////////////////////////////
/// \fn int actionStartStep ( void );
/// \brief get Next step to treat and init the FIRST action named "Départ"
/// \return > 0 the current step, < 0 see errno for mer details
////////////////////////////////////////////////////////////////////////////////
int actionStartStep ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerUpdate ( void );
/// \brief manage actions / timeouts / blocking actions
/// \return 0
////////////////////////////////////////////////////////////////////////////////
int actionManagerUpdate ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerCurrentNumber ( uint32_t step );
/// \param [in] step : step for what we want to know size ( provided by
///     actionStartStep )
/// \retrun the number of action remainig for the deffined step
////////////////////////////////////////////////////////////////////////////////
int actionManagerCurrentNumber ( const uint32_t step );

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerExec ( void );
/// \biref make the acts for the currents actions actives
/// \retrun 0 ok, else see errno
////////////////////////////////////////////////////////////////////////////////
int actionManagerExec ( void );



////////////////////////////////////////////////////////////////////////////////
/// \fn void actionManagerPrint ( void );
/// \brief print the full json action
////////////////////////////////////////////////////////////////////////////////
void actionManagerPrint ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn void actionManagerPrintCurrent ( void );
/// \brief print the current actions actives
////////////////////////////////////////////////////////////////////////////////
void actionManagerPrintCurrent ( void );

#endif
