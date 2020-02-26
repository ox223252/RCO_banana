#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "lib/log/log.h"

#include "utils.h"
#include "action.h"
#include "utilActions/actionExtract.h"
#include "utilActions/actionDyna.h"

#include "lib/timer/timer.h"

static json_el * _action_json = NULL;
static uint32_t _action_jsonLength = 0;

static json_el * _action_var = NULL;
static uint32_t _action_varLength = 0;

typedef struct
{
	uint32_t stepId;
	uint32_t * actionsId;
	uint32_t * timeout;
	time_t * start;
	bool * blocking;
	json_el ** params;
	uint32_t length;
}
currentWork;

static currentWork * _action_current = NULL;
static uint32_t _action_currentLength = 0;

static uint32_t _action_currentIndex = 0;

static const char * _action_name[] = {
	[aT(none)] = "none",
	[aT(servo)] = "setServo",
	[aT(pause)] = "Pause",
	[aT(get_var)] = "getVar",
	[aT(set_var)] = "setVar",
	[aT(set_dyna)] = "setDyna",
	[aT(get_dyna)] = "getDyna",
	[aT(timeout)] = "PermAction",
	[aT(last)] = NULL
};

int _action_mcpFd = -1;
int _action_pcaFd = -1;
int _action_dynaFd = -1;

////////////////////////////////////////////////////////////////////////////////
/// internals functions
static int newCurrent ( uint32_t step, uint32_t action )
{
	uint32_t i;
	void *tmp = NULL;

	for ( i = 0; i < _action_currentLength; i++ )
	{
		if ( _action_current[ i ].stepId == step )
		{
			break;
		}
	}

	// if stepId not found create new entry
	if ( i == _action_currentLength )
	{
		tmp = realloc ( _action_current, sizeof ( currentWork ) * ( _action_currentLength + 1 ) );
		
		if ( !tmp )
		{
			logDebug ( "\n" );
			return ( -__LINE__ );
		}

		_action_current = tmp;
		_action_current[ i ].length = 0;
		_action_current[ i ].stepId = step;
		_action_current[ i ].actionsId = NULL;
		_action_current[ i ].params = NULL;
		_action_current[ i ].timeout = NULL;
		_action_current[ i ].start = NULL;
		_action_current[ i ].blocking = NULL;
		_action_currentLength += 1;
	}

	// current actions ID
	tmp = realloc ( _action_current[ i ].actionsId, sizeof ( uint32_t ) * ( _action_current[ i ].length + 1 ) );
	if ( !tmp )
	{
		logDebug ( "\n" );
		return ( -__LINE__ );
	}
	else
	{
		_action_current[ i ].actionsId = tmp;
		_action_current[ i ].actionsId[ _action_current[ i ].length ] = action;
	}

	// current parameters
	tmp = realloc ( _action_current[ i ].params, sizeof ( json_el* ) * ( _action_current[ i ].length + 1 ) );
	if ( !tmp )
	{
		logDebug ( "\n" );
		return ( -__LINE__ );
	}
	else
	{ // add params to the current array
		_action_current[ i ].params = tmp;
		tmp = NULL;
		uint32_t l = 0;
		getActionParams ( _action_json, action, (json_el**)&tmp, &l );
		_action_current[ i ].params[ _action_current[ i ].length ] = tmp;
	}

	// current actions timeout
	tmp = realloc ( _action_current[ i ].timeout, sizeof ( uint32_t ) * ( _action_current[ i ].length + 1 ) );
	if ( !tmp )
	{
		logDebug ( "\n" );
		return ( -__LINE__ );
	}
	else
	{
		void * t = NULL;
		JSON_TYPE type = jT( undefined );

		if ( !jsonGet ( _action_current[ i ].params[ _action_current[ i ].length ], 0, "Timeout", (void**)&t, &type ) ||
			type != jT ( str ) ||
			t == NULL )
		{ // no timeout
			((uint32_t*)tmp)[ _action_current[ i ].length ] = 0;
		}
		else
		{
			((uint32_t*)tmp)[ _action_current[ i ].length ] = atol ( (char*)t );
		}
		_action_current[ i ].timeout = tmp;
	}

	// current actions blocking
	tmp = realloc ( _action_current[ i ].blocking, sizeof ( bool ) * ( _action_current[ i ].length + 1 ) );
	if ( !tmp )
	{
		logDebug ( "\n" );
		return ( -__LINE__ );
	}
	else
	{
		_action_current[ i ].blocking = tmp;

		void * b = NULL;
		JSON_TYPE type = jT( undefined );

		if ( !jsonGet ( _action_json, action, "blocante", (void**)&b, &type ) ||
			type != jT ( bool ) ||
			b == NULL )
		{ // no timeout
			_action_current[ i ].blocking[ _action_current[ i ].length ] = false;
		}
		else
		{
			_action_current[ i ].blocking[ _action_current[ i ].length ] = *(bool*)b;
		}
	}

	// current actions start time
	tmp = realloc ( _action_current[ i ].start, sizeof ( time_t ) * ( _action_current[ i ].length + 1 ) );
	if ( !tmp )
	{
		logDebug ( "\n" );
		return ( -__LINE__ );
	}
	else
	{
		_action_current[ i ].start = tmp;
		_action_current[ i ].start[ _action_current[ i ].length ] = getDateMs ( );
	}

	_action_current[ i ].length += 1;

	return ( 0 );
}

static void delCurrent ( uint32_t step, uint32_t action )
{
	uint8_t stepExchange = 0;
	for ( uint32_t i = 0; i < ( _action_currentLength - stepExchange ); i++ )
	{
		if ( _action_current[i].stepId != step )
		{
			continue;
		}

		uint8_t exchange = 0;
		for ( uint32_t j = 0; j < ( _action_current[ i ].length - exchange ); j++ )
		{
			if ( _action_current[ i ].actionsId[ j ] == action )
			{
				exchange = 1;
			}

			if ( _action_current[ i ].params[ j ] == action )
			{
				jsonFree ( &_action_current[ i ].params[ j ], 1 );
				_action_current[ i ].params[ j ] = NULL;
			}

			if ( !exchange ||
				j >= ( _action_current[ i ].length - exchange ) )
			{
				continue;
			}
			
			_action_current[ i ].actionsId[ j ] = _action_current[ i ].actionsId[ j + 1 ];
			_action_current[ i ].actionsId[ j + 1 ] = 0;

			_action_current[ i ].timeout[ j ] = _action_current[ i ].timeout[ j + 1 ];
			_action_current[ i ].timeout[ j + 1 ] = 0;

			_action_current[ i ].start[ j ] = _action_current[ i ].start[ j + 1 ];
			_action_current[ i ].start[ j + 1 ] = 0;

			_action_current[ i ].blocking[ j ] = _action_current[ i ].blocking[ j + 1 ];
			_action_current[ i ].blocking[ j + 1 ] = 0;

			_action_current[ i ].params[ j ] = _action_current[ i ].params[ j + 1 ];
			_action_current[ i ].params[ j + 1 ] = NULL;
		}

		_action_current[ i ].length -= exchange;

		if ( _action_current[ i ].length == 0 )
		{
			free ( _action_current[ i ].actionsId );
			free ( _action_current[ i ].blocking );
			free ( _action_current[ i ].timeout );
			free ( _action_current[ i ].params );
			free ( _action_current[ i ].start );

			_action_current[ i ].actionsId = NULL;
			_action_current[ i ].blocking = NULL;
			_action_current[ i ].timeout = NULL;
			_action_current[ i ].params = NULL;
			_action_current[ i ].start = NULL;

			stepExchange = 1;
		}

		if ( stepExchange &&
			i < _action_currentLength - stepExchange )
		{
			_action_current[ i ] = _action_current[ i + 1 ];
		}
	}
	_action_currentLength -= stepExchange;

	if ( _action_currentLength == 0 )
	{
		free ( _action_current );
		_action_current = NULL;
	}
}

// return the index of key in the _action_name array
// if -1 key is not in array
static int actionNameToId ( const char * __restrict__ const  key )
{
	uint32_t i = 0;
	while ( _action_name[i] )
	{
		if ( !strcmp ( _action_name[i], key ) )
		{
			return ( i );
		}
		i++;
	}
	return ( -1 );
}

static void cleanCurrent ( void )
{
	for ( uint32_t i = 0; i < _action_currentLength; i++ )
	{
		for ( uint32_t j = 0; j < _action_current[ i ].length; j++ )
		{
			jsonFree ( &_action_current[ i ].params[ j ], 1 );
		}
		free ( _action_current[ i ].actionsId );
		free ( _action_current[ i ].blocking );
		free ( _action_current[ i ].timeout );
		free ( _action_current[ i ].params );
		free ( _action_current[ i ].start );

		_action_current[ i ].actionsId = NULL;
		_action_current[ i ].blocking = NULL;
		_action_current[ i ].timeout = NULL;
		_action_current[ i ].params = NULL;
		_action_current[ i ].start = NULL;
	}

	free ( _action_current );
	_action_current = NULL;
	_action_currentLength = 0;
}

// get char* from _action_current array
// return 0 if OK else error
// if error set status to "done"
static inline int getCharFromParams ( const uint32_t step, const uint32_t action, const char * __restrict__ const str, void ** const out )
{
	JSON_TYPE type;
	if ( !jsonGet ( _action_current[ step ].params[ action ], 0, str, out, &type ) ||
		type != jT( str ) )
	{
		logDebug ( "ERROR param \"%s\" not found %p %d\n", str, *out, type );
		logDebug ( "      %p\n", _action_current[ step ].params[ action ] );
		jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );
		return ( __LINE__ );
	}
	return ( 0 );
}

// get char* from _action_json array
// return 0 if OK else error
// if error set status to "done"
static inline int getCharFromMain ( const uint32_t step, const uint32_t action, const char * __restrict__ const str, void ** const out )
{
	JSON_TYPE type;
	if ( !jsonGet ( _action_json,  _action_current[ step ].actionsId[ action ], str, out, &type ) ||
		type != jT( str ) )
	{
		logDebug ( "ERROR param \"%s\" not found %p %d\n", str, *out, type );
		jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );
		return ( __LINE__ );
	}
	return ( 0 );
}

typedef struct {
	uint32_t stepId;
	uint32_t length;
	uint32_t * nexts;
}actionCleanAndSet_t;

static void * actionCleanAndSet ( void * arg )
{
	printf ( "action timeout\n" );
	actionCleanAndSet_t * a = arg;

	cleanCurrent ( );

	for ( uint32_t k = 0; k < a->length; k++ )
	{
		getActionId ( _action_json, a->stepId, &(a->nexts[ k ]) );
		newCurrent ( a->stepId, a->nexts[ k ] );
		_action_currentIndex++;
	}

	free ( a->nexts );
	a->nexts = NULL;
	free ( a );
	a = NULL;
	return ( NULL );
}

// make one action select by step index and action index
// return 0 Ok else error
static int execOne ( const uint32_t step, const uint32_t action )
{
	JSON_TYPE type;
	char * actionName = NULL;

	if ( getCharFromMain ( step, action, "nomAction", (void**)&actionName ) )
	{ // si une action n'a pas de nom alors on la finie quoi qu'il arrive, ça evitera des bloquages plus tard
		logDebug ( "no name\n" );
		return ( 0 );
	}

	logDebug ( "make %s\n", actionName );

	switch ( actionNameToId ( actionName ) )
	{
		// case aT(servo):
			// { // done
			// 	//setPCA9685PWM ( atoi ( listAction[ indiceAction ].params[ 0 ] ), 0, 210 + atoi ( listAction[ indiceAction ].params[ 1 ] ) % 360, *_management_pca9685 );
			// 	listAction[indiceAction].isDone = 1;
			// 	break;
			// }
		case aT(set_dyna):
		{
			if ( _action_dynaFd <= 0 )
			{ // arm disabled
				return ( 0 );
			}

			uint32_t mID, mVitesse, mValue;

			void * tmp = NULL;
			if ( getCharFromParams ( step, action, "id", &tmp ) )
			{
				return ( __LINE__ );
			}
			mID = atoi ( (char*)tmp );

			if ( getCharFromParams ( step, action, "Vitesse", &tmp ) )
			{
				return ( __LINE__ );
			}
			mVitesse = atoi ( (char*)tmp );

			if ( getCharFromParams ( step, action, "Value", &tmp ) )
			{
				return ( __LINE__ );
			}
			mValue = atoi ( (char*)tmp );

			if ( setVitesseDyna ( mID, ( int )( 10.23*mVitesse ) ) )
			{
				logVerbose ( "Erreur set vitesse dyna \n" );
			}
			if ( setPositionDyna ( mID, mValue ) )
			{
				logVerbose ( "Erreur set angle dyna \n" );
			}
			break;
		}
		case aT(get_dyna):
		{
			if ( _action_dynaFd <= 0 )
			{ // arm disabled
				jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );
				return ( 0 );
			}

			uint32_t mID, mTolerance, mValue;

			void * tmp = NULL;
			if ( getCharFromParams ( step, action, "id", (void**)&actionName ) )
			{
				return ( __LINE__ );
			}
			mID = atoi ( (char*)tmp );

			if ( getCharFromParams ( step, action, "Tolerance", (void**)&actionName ) )
			{
				return ( __LINE__ );
			}
			mTolerance = atoi ( (char*)tmp );

			if ( getCharFromParams ( step, action, "Value", (void**)&actionName ) )
			{
				return ( __LINE__ );
			}
			mValue = atoi ( (char*)tmp );

			if ( (uint32_t)abs ( getPositionDyna ( mID ) - mValue ) < mTolerance )
			{
				jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );
			}
			break;
		}
		// case TYPE_CAPTEUR:
			// {
			// 	break;
			// }
			// case TYPE_MOTEUR:
			// { // done 
			// 	if ( _management_flagAction->noDrive )
			// 	{
			// 		if ( _management_flagAction->driveScan )
			// 		{
			// 			while ( _kbhit ( ) )
			// 			{
			// 				listAction[indiceAction].isDone = 1;
			// 			}
			// 		}
			// 		else if ( _management_flagAction->driveWait )
			// 		{
			// 		}
			// 		else
			// 		{ // drive done
			// 			listAction[indiceAction].isDone = 1;
			// 		}
			// 	}
			// 	else
			// 	{

			// 	}
			// 	break;
			// }
			// case TYPE_AUTRE:
			// {
			// 	break;
			// }
			// case TYPE_POSITION:
			// { // done
			// 	if ( _management_newDeplacement == 1 )
			// 	{
			// 		_management_newDeplacement = 0;
			// 		robot->vitesseGaucheDefault = 0.;
			// 		robot->vitesseDroiteDefault = 0.;
			// 		resetBlocage();

			// 		robot->cible.xCible = atoi ( listAction[ indiceAction ].params[ 0 ] );
			// 		robot->cible.yCible = atoi ( listAction[ indiceAction ].params[ 1 ] );
			// 		robot->cible.vitesseMax = atoi ( listAction[ indiceAction ].params[ 2 ] );
			// 		robot->cible.acc = atoi ( listAction[ indiceAction ].params[ 3 ] );
			// 		robot->cible.dec = atoi ( listAction[ indiceAction ].params[ 4 ] );
			// 		robot->cible.sens = atoi ( listAction[ indiceAction ].params[ 5 ] );
			// 		robot->cible.precision = atoi ( listAction[ indiceAction ].params[ 6 ] );
			// 		robot->cible.distanceFreinage = atoi ( listAction[ indiceAction ].params[ 7 ] );
			// 		robot->setDetection = atoi ( listAction[ indiceAction ].params[ 8 ] );
			// 		premierAppel ( robot );
			// 	}
			// 	else if ( calculDeplacement ( robot )==1 )
			// 	{
			// 		_management_newDeplacement = 1;
			// 		listAction[indiceAction].isDone = 1;
			// 		robot->vitesseGaucheDefault = 0.;
			// 		robot->vitesseDroiteDefault = 0.;
			// 	}

			// 	break;
			// }
			// case TYPE_ORIENTATION:
			// { // done
			// 	if ( _management_newDeplacement == 1 )
			// 	{
			// 		_management_newDeplacement = 0;
			// 		robot->vitesseGaucheDefault = 0.;
			// 		robot->vitesseDroiteDefault = 0.;
			// 		resetBlocage();
			// 		robot->orientationVisee = atoi ( listAction[ indiceAction ].params[ 0 ] );
			// 		robot->cible.vitesseMax = atoi ( listAction[ indiceAction ].params[ 1 ] );
			// 		robot->cible.precision = atoi ( listAction[ indiceAction ].params[ 2 ] );
			// 		premierAppelTenirAngle ( robot );
			// 	}
			// 	else
			// 	{
			// 		if ( tenirAngle ( robot )==1 )
			// 		{
			// 			_management_newDeplacement = 1;
			// 			listAction[indiceAction].isDone = 1;
			// 			robot->vitesseGaucheDefault = 0.;
			// 			robot->vitesseDroiteDefault = 0.;
			// 		}
			// 	}
			// 	break;
			// }
			// case TYPE_SEQUENCE:
			// {
			// 	break;
			// }
			// case TYPE_ENTREE:
			// { // done
			// 	listAction[indiceAction].isDone = 1;
			// 	break;
			// }
			// case TYPE_ATTENTE_SERVO:
			// {
			// 	break;
			// }
		case aT(pause):
		{ // done
			void * tmp = NULL;

			if ( getCharFromParams ( step, action, "Temps", &tmp ) )
			{
				return ( __LINE__ );
			}

			uint32_t temps = atoi ( (char*)tmp );
			if ( getDateMs ( ) - _action_current[ step ].start[ action ] > temps )
			{ // le temps est passé
				jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );
			}
			break;
		}
		// case TYPE_RETOUR_DEPLACEMENT:
			// {

			// 	break;
			// }
			// case TYPE_RETOUR_ORIENTATION:
			// {
			// 	break;
			// }
			// case TYPE_RETOUR_POSITION:
			// {
			// 	break;
			// }
			// case TYPE_GPIO:
			// {
			// 	//printf("GPIO : %s %s %d\n",listAction[ indiceAction ].params[ 0 ],listAction[ indiceAction ].params[ 1 ],*(_management_mcp23017));
			// 	gpioSet ( _management_mcp23017, 'A', atoi ( listAction[ indiceAction ].params[ 0 ] ), atoi ( listAction[ indiceAction ].params[ 1 ] ) != 1 );

			// 	listAction[indiceAction].isDone = 1;
			// 	break;
			// }
			// case TYPE_RETOUR_GPIO:
			// {
			// 	if(GPIORead(atoi ( listAction[ indiceAction ].params[ 0 ] )) == atoi ( listAction[ indiceAction ].params[ 1 ] ))
			// 	{

			// 		listAction[indiceAction].isDone = 1;
			// 	}
					
			// 	break;
			// }
			// case TYPE_AND:
			// {
			// 	break;
			// }
			// case TYPE_SET_VALEUR: //fonction
			// { // done
			// 	switch ( atoi ( listAction[ indiceAction ].params[ 0 ] ) )
			// 	{
			// 		case 0:
			// 		{
			// 			//xRobot
			// 			robot->xRobot = atoi ( listAction[ indiceAction ].params[ 1 ] );
			// 			listAction[indiceAction].isDone = 1;
			// 			break;
			// 		}
			// 		case 1:
			// 		{
			// 			//yRobot
			// 			robot->yRobot = atoi ( listAction[ indiceAction ].params[ 1 ] );
			// 			listAction[indiceAction].isDone = 1;
			// 			break;
			// 		}
			// 		case 2:
			// 		{
			// 			//Orientation Robot
			// 			robot->orientationRobot = atoi ( listAction[ indiceAction ].params[ 1 ] );
			// 			robot->orientationVisee = atoi ( listAction[ indiceAction ].params[ 1 ] );

			// 			listAction[indiceAction].isDone = 1;
			// 			break;
			// 		}
			// 		case 3:
			// 		{
			// 			//Vitesse Linéaire
			// 			exit(0);
			// 			robot->vitesseGaucheDefault = atoi ( listAction[ indiceAction ].params[ 1 ] );
			// 			robot->vitesseDroiteDefault = atoi ( listAction[ indiceAction ].params[ 1 ] );
			// 			listAction[indiceAction].isDone = 1;
			// 			break;
			// 		}
			// 		case 4:
			// 		{
			// 			//Vitesse Angulaire
			// 			robot->vitesseGaucheDefault = -1.* atoi ( listAction[ indiceAction ].params[ 1 ] );
			// 			robot->vitesseDroiteDefault = atoi ( listAction[ indiceAction ].params[ 1 ] );
			// 			listAction[indiceAction].isDone = 1;
			// 			break;
			// 		}
			// 	}
			// 	break;
			// }
			// case TYPE_COURBE:
			// {
			// 	break;
			// }
			// case TYPE_ATTENTE_BLOCAGE:
			// {
			// 	break;
			// }
			// case TYPE_DEPLACEMENT:
			// {
			// 	break;
			// }
			// case TYPE_FIN:
			// {
			// 	listAction[indiceAction].isDone = 1;
			// 	break;
			// }
			// case TYPE_SET_VARIABLE:
			// {
			// 	jsonSet ( _management_json, 0, listAction[ indiceAction ].params[ 0 ], listAction[ indiceAction ].params[ 1 ], jT ( str ) );
			// 	listAction[indiceAction].isDone = 1;
			// 	break;
			// }
			// case TYPE_GET_VARIABLE:
			// {
			// 	jsonGet ( _management_json, 0, listAction[ indiceAction ].params[ 0 ], (void **)&listAction[ indiceAction ].params[ 2 ], NULL );
			// 	if ( !strcmp ( listAction[ indiceAction ].params[ 1 ], listAction[ indiceAction ].params[ 2 ] ) )
			// 	{
			// 		listAction[indiceAction].isDone = 1;
			// 	}
			// 	break;
			// }
		case aT(get_var):
		{
			char * name = NULL;
			type = jT(undefined);

			if ( getCharFromParams ( step, action, "key", (void**)&name ) )
			{ // si une action n'a pas de nom alors on la finie quoi qu'il arrive, ça evitera des bloquages plus tard
				return ( __LINE__ );
			}

			// if ( !jsonGet ( _action_current[ step ].params[ action ], 0, "key", (void**)&name, &type ) )
			// { // no key for variable in params
			// 	logDebug ( "\n" );
			// 	return ( __LINE__ );
			// }

			// if ( type != jT(str) )
			// { // the key is no a string
			// 	logDebug ( "\n" );
			// 	return ( __LINE__ );
			// }

			void * value = NULL;

			jsonGet ( _action_var, 0, name, &value, &type );
			break;
		}
		case aT(set_var):
		{
			char * name = NULL;
			type = jT(undefined);

			if ( getCharFromParams ( step, action, "id", (void**)&name ) )
			{ // no key for variable in params
				return ( __LINE__ );
			}
		
			// on recupère la cible
			double * target = NULL;
			double tmp = 0.0;

			if ( !jsonGet ( _action_var, 0, name, (void**)&target, &type ) )
			{ // the var $name doesn't existe
				target = &tmp;
			}
			else if ( type != jT(double) )
			{ // the var is not a number... not normal
				logDebug ( "\n" );
				return ( __LINE__ );
			}

			// le type d'action à faire
			char * op = NULL;
			if ( getCharFromParams ( step, action, "condition", (void**)&op ) )
			{
				return ( __LINE__ );
			}

			// l'operande
			char * t = NULL;
			if ( getCharFromParams ( step, action, "value", (void**)&t ) )
			{
				return ( __LINE__ );
			}
			double value = atof ( t );
			
			// et puis on fini par faire le calcul
			if ( !strcmp( op, "+" ) )
			{
				value += (*target);
			}
			else if ( !strcmp( op, "*" ) )
			{
				value *= (*target);
			}
			else if ( !strcmp( op, "/" ) &&
				( (*target) != 0 ) )
			{
				value = (*target) / value;
			}
			else if ( !strcmp( op, "-" ) )
			{
				value = (*target) - value;
			}

			jsonSet ( _action_var, 0, name, (void*)&value, jT ( double ) );

			break;
		}
		case aT(timeout):
		{
			jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );

			uint32_t delay = _action_current[ step ].timeout[ action ];
			delay -= ( getDateMs ( ) - _action_current[ step ].start[ action ] );
			delay *= 1000;

			actionCleanAndSet_t *arg = malloc ( sizeof(*arg) );
			if ( !arg )
			{
				return ( __LINE__ );
			}

			arg->stepId = _action_current[ step ].stepId;
			
			switch( getNextActions ( _action_json, _action_current[ step ].actionsId[ action ], &(arg->nexts), &(arg->length), true ) )
			{
				case -1:
				{ // nothing more to be done
					free ( arg );
					return ( 0 );
				}
				case 0:
				{ // next step
					if ( !startTimer( delay, actionCleanAndSet, (void *)arg) )
					{
						free ( arg );
						logDebug ( "externel timeout start failed\n" );
						return ( __LINE__ );
					}
					break;
				}
				default:
				{ // error case
					free ( arg );
					return ( __LINE__ );
				}
			}
			break;
		}
		case aT(none):
		case aT(last):
		default:
		{
			logDebug ( "\e[33m unknow action %s\e[0m\n", actionName );
			break;
		}
	}

	return ( 0 );
}

////////////////////////////////////////////////////////////////////////////////
/// init part
int actionManagerInit ( const char * __restrict__ const file )
{
	actionManagerDeInit ( );

	if ( jsonParseFile ( file, &_action_json, &_action_jsonLength ) )
	{
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	if ( jsonParseString ( "{}", &_action_var, &_action_varLength ) )
	{
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	return ( 0 );
}

int actionManagerDeInit ( void )
{
	if ( _action_json )
	{
		jsonFree ( &_action_json, _action_jsonLength );
		_action_json = NULL;
		_action_jsonLength = 0;
	}

	if ( _action_var )
	{
		jsonFree ( &_action_var, _action_varLength );
		_action_var = NULL;
		_action_jsonLength = 0;
	}

	cleanCurrent ( );
	
	return ( 0 );
}

////////////////////////////////////////////////////////////////////////////////
/// function for the daily work
int actionStartStep ( void )
{
	static uint32_t stepIndex = 0xffffffff;
	stepIndex++;

	uint32_t stepId = stepIndex;

	if ( getStepId ( _action_json, &stepId ) )
	{
		logDebug ( "\n" );
		return ( -__LINE__ );
	}

	uint32_t actionIndex = 0;
	do
	{
		uint32_t actionId = actionIndex;

		if ( getActionId ( _action_json, stepId, &actionId ) )
		{
			logDebug ( "%d\n", actionId );
			break;
		}

		char * name = NULL;
		jsonGet ( _action_json, actionId, "nomAction", (void**)&name, NULL );

		if ( !strcmp ( "Départ", name ) )
		{
			newCurrent ( stepId, actionId );

			return ( stepId );
		}

		actionIndex++;
	}
	while ( true );
	
	// pas d'etatpe nomé Début trouve dans la sequence
	logDebug ( "\n" );
	return ( -__LINE__ );
}

int actionManagerUpdate ( void )
{
	// pour toutes les etapes courrantes (normalement on ne devrait en avoir qu'une mais ça permet déviter un oubli)
	for ( uint32_t i = 0; i < _action_currentLength; i++ )
	{
		#ifdef ONE_LEVEL_BY_LOOP
		// on sauvegarde le nombre d'action en cours car s'il y en à de nouvelle opn les rajoutera à la suite donc il faut
		/// eviter de rajouter une action et la retraiter aussitot ce qui nous empecherait d'executer l'action par la suite
		uint32_t length = _action_current[ i ].length;
		#endif
		
		// pour toutes les actions enregistrées dans le tableau
		#ifdef ONE_LEVEL_BY_LOOP
		for ( uint32_t j = 0; j < length; j++ )
		#else
		for ( uint32_t j = 0; _action_current && j < _action_current[ i ].length; j++ )
		#endif
		{
			bool timeout = false;

			// TODO
			// si l'action n'a pas de params ce n'est pas normal donc on la traite pas (pour le moment)
			if ( !_action_current[ i ].params )
			{ // no parameters
				continue;
			}


			uint32_t now = getDateMs ( );

			if ( ( 0 != _action_current[ i ].timeout[ j ] ) &&
				( ( now - _action_current[ i ].start[ j ] ) > _action_current[ i ].timeout[ j ] ) )
			{ // si il y à un timeout et qu'il est passé 
				timeout = true;
			}
			else if ( _action_current[ i ].blocking[ j ] )
			{ // s'il n'est pas passé,  on verifie son etat d'avancement
				timeout = false;
				char * status = NULL;
				JSON_TYPE type = jT( undefined );

				if ( !jsonGet ( _action_current[ i ].params[ j ], 0, "status", (void**)&status, &type ) )
				{ // on a rien trouvé
					continue;
				}
				else if ( type != jT ( str ) )
				{ // la valeur retourné n'est pas un string
					logDebug( "\n" );
					continue;
				}
				else if ( status == NULL )
				{ // pointeur null donc pas de string à tester
					logDebug( "\n" );
					continue;
				}
				else if ( strcmp ( status, "done" ) )
				{ // action pas finie
					logDebug( "\n" );
					continue;
				}
			}
			else
			{ // si l'action n'est pas bloquante ...
				#ifndef ONE_LEVEL_BY_LOOP
				// sin on peut faire plusieurs niveau de l'arbre généalogique en une boucle
				// alors on doit traiter les action à ce niveau
				execOne ( i, j );
				#endif
			}
			
			uint32_t * next = NULL;
			uint32_t l = 0;

			// on recupère la liste des actions filles / timeout
			switch( getNextActions ( _action_json, _action_current[ i ].actionsId[ j ], &next, &l, timeout ) )
			{
				case -1:
				{ // nothing more to be done
					break;
				}
				case 0:
				{ // next step
					for ( uint32_t k = 0; k < l; k++ )
					{
						getActionId ( _action_json, _action_current[ i ].stepId, &next[ k ] );
						newCurrent ( _action_current[ i ].stepId, next[ k ] );
					}
					break;
				}
				default:
				{ // error case
					logDebug( "ERROR\n" );
					break;
				}
			}

			free ( next );
			
			// on supprime l'action courrante
			delCurrent ( _action_current[ i ].stepId, _action_current[ i ].actionsId[ j ] );
			_action_currentIndex++;

			j--; // remove one element so need to take care about
			#ifdef ONE_LEVEL_BY_LOOP
			length--;
			#endif
		}
	}
	return ( 0 );
}

uint32_t actionManagerCurrentIndex ( void )
{
	return ( _action_currentIndex );
}

int actionManagerCurrentNumber ( const uint32_t step )
{
	for ( uint32_t i = 0; i < _action_currentLength; i++ )
	{
		if ( _action_current[ i ].stepId == step )
		{
			return (  _action_current[ i ].length );
		}
	}
	return ( 0 );
}

void actionManagerSetFd ( const int mcpFd, const int pcaFd, const int dynaFd )
{
	_action_mcpFd = mcpFd;
	_action_pcaFd = pcaFd;
	_action_dynaFd = dynaFd;
	setPortNum(_action_dynaFd);
}

int actionManagerExec ( void )
{
	for ( uint32_t i = 0; i < _action_currentLength; i++ )
	{
		for ( uint32_t j = 0; j < _action_current[ i ].length; j++ )
		{
			int err = execOne ( i, j );
			if ( err )
			{
				return ( err );
			}
		}
	}
	return ( 0 );
}

////////////////////////////////////////////////////////////////////////////////
/// prints functions
void actionManagerPrint ( void )
{
	// id of the step in the sequence array
	uint32_t stepIndex = 0;
	do 
	{
		// id of the step in main _action_json memory array
		uint32_t stepId = stepIndex;

		if ( getStepId ( _action_json, &stepId ) )
		{
			logDebug ( "\n" );
			break;
		}
		
		printf ( "etape %d\n", stepId );

		uint32_t actionIndex = 0;
		do
		{
			uint32_t actionId = actionIndex;

			if ( getActionId ( _action_json, stepId, &actionId ) )
			{
				logDebug ( "\n" );
				break;
			}

			char * name = NULL;
			jsonGet ( _action_json, actionId, "nomAction", (void**)&name, NULL );
			printf ( "\taction %d (%d) : %s\n", actionIndex, actionId, name );

			uint32_t *tableNext = NULL;
			uint32_t length = 0;

			switch ( getNextActions ( _action_json, actionId, &tableNext, &length, true ) )
			{
				case -1:
				{
					logDebug ( "\n" );
					break;
				}
				case 0:
				{
					for ( uint32_t i = 0; i < length; i++ )
					{
						printf ( "\t\tnext timeout : %d\n", tableNext[ i ] );
					}
					free ( tableNext );
					tableNext = NULL;
					break;
				}
				default:
				{
					logDebug ( "\n" );
					return;
				}
			}

			switch ( getNextActions ( _action_json, actionId, &tableNext, &length, false ) )
			{
				case -1:
				{
					logDebug ( "\n" );
					actionIndex = -1;
					break;
				}
				case 0:
				{
					for ( uint32_t i = 0; i < length; i++ )
					{
						printf ( "\t\tnext action : %d\n", tableNext[ i ] );
					}

					actionIndex = tableNext[ 0 ];

					free ( tableNext );
					tableNext = NULL;
					break;
				}
				default:
				{
					logDebug ( "\n" );
					return;
				}
			}
		}
		while ( getchar() );

		stepIndex++;
	}
	while ( true );


	return;
}

void actionManagerPrintCurrent ( void )
{
	printf ( "\033[4m%67s\e[0m\n", "" );
	printf ( "\033[4mtype   | %24s | %3s | %10s | %10s |\n\e[0m", "name", "id", "start", "timeout" );
	for ( uint32_t i = 0; i < _action_currentLength; i++ )
	{
		printf ( "step   | %24s | %3d | %10s | %10s |\n", "", _action_current[ i ].stepId, "", "" );

		for ( uint32_t j = 0; j < _action_current[ i ].length; j++ )
		{
			char * name;
			jsonGet ( _action_json, _action_current[ i ].actionsId[ j ], "nomAction", (void**)&name, NULL );
			printf ( "action | %24s | %3d | %10ld | %10d |\n", 
				name, 
				_action_current[ i ].actionsId[ j ], 
				_action_current[ i ].start[ j ], 
				(uint32_t)_action_current[ i ].timeout[ j ] );
		}
	}
}

void actionManagerPrintEnv ( void )
{
	jsonPrint ( _action_var, 0, 0 );
}
