////////////////////////////////////////////////////////////////////////////////
/// \copiright RCO, 2019
///
/// This program is free software: you can redistribute it and/or modify it
///     under the terms of the GNU General Public License published by the Free
///     Software Foundation, either version 2 of the License, or (at your
///     option) any later version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
///     ANY WARRANTY; without even the implied of MERCHANTABILITY or FITNESS FOR
///     A PARTICULAR PURPOSE. See the GNU General Public License for more
///     details.
///
/// You should have received a copy of the GNU General Public License along with
///     this program. If not, see <http://www.gnu.org/licenses/>
////////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "lib/log/log.h"

#include "utils.h"
#include "action.h"
#include "utilActions/actionExtract.h"
#include "utilActions/actionDyna.h"
#include "lib/freeOnExit/freeOnExit.h"
#include "deplacement/gestionPosition.h"

#include "lib/timer/timer.h"
#include "lib/termRequest/request.h"
#include "lib/pca9685/pca9685.h"

#include "utilActions/actionVars.h"

///< mutex used to allow usage of thread in actions and their management
static pthread_mutex_t _action_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

static bool isPremierAppel = true;
static int32_t posGauche = 0;
static int32_t posDroite = 0;

/// \brief this struct store the current actions
typedef struct
{
	uint32_t stepId; ///< the step what this instance is related
	uint32_t * actionsId; ///< running action array 
	uint32_t * timeout; ///< running action's timeout array (ms)
	time_t * start; ///< running actions's start time array (ms)
	bool * blocking; ///< running action's blocking mode's status
	json_el ** params; ///< running action's parameters stored as json 
	uint32_t length; ///< parameters length
}
currentWork;

static currentWork * _action_current = NULL; ///< array of action running
static uint32_t _action_currentLength = 0; ///< length of running action's array

static uint32_t _action_currentIndex = 0; ///< count hwo many action was done 
	/// from the beginin used to know changed from two call of actionManagerUpdate
static Robot * mRobot = NULL; ///< struct used to manage robot

///< array that contain string reffderence to manage action
static const char * _action_name[] = {
	[aT(none)] = "none",
	[aT(servo)] = "setServo",
	[aT(pause)] = "Pause",
	[aT(get_var)] = "getVar",
	[aT(set_var)] = "setVar",
	[aT(set_dyna)] = "setDyna",
	[aT(get_dyna)] = "getDyna",
	[aT(timeout)] = "PermAction",
	[aT(start)] = "Départ",
	[aT(sequence)] = "Sequence",
	[aT(position)] = "Position",
	[aT(orientation)] = "tenirAngle",
	[aT(stopMove)] = "arretMoteur",
	[aT(blocked)] = "attenteBlocage",
	[aT(pick)] = "PriseVerreExterieur",
	[aT(place)] = "DeposeVerrePort",
	[aT(move)] = "Déplacement",
	[aT(getGpio)] = "getGpio",
	[aT(setGpio)] = "setGpio",
	[aT(setPasAPas)] = "setPasAPas",
	[aT(end)] = "Fin",
	[aT(last)] = NULL
};

static ActionFlag *_action_flags = NULL; ///< flag action noArm/noDrive

static int _action_mcpFd = -1; ///< file descriptor of mcp23017
static int _action_pcaFd = -1; ///< file descriptor of pca9685
static int _action_dynaFd = -1; ///< file descriptor of dynamixels

////////////////////////////////////////////////////////////////////////////////
/// internals functions
/// \param [ in ] step : step if of the current action
/// \param [ in ] action : action if of the new action
/// \brief create new element in the current array
/// \return 0 if OK else return num line of error
static int newCurrent ( const uint32_t step, const uint32_t action )
{
	uint32_t i;
	void *tmp = NULL;
	int rt = 0;

	pthread_mutex_lock ( &_action_mutex );
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
			rt = __LINE__;
			goto newCurrent_end;
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
		rt = __LINE__;
		goto newCurrent_end;
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
		rt = __LINE__;
		goto newCurrent_end;
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
		rt = __LINE__;
		goto newCurrent_end;
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
		rt = __LINE__;
		goto newCurrent_end;
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
		rt = __LINE__;
		goto newCurrent_end;
	}
	else
	{
		_action_current[ i ].start = tmp;
		_action_current[ i ].start[ _action_current[ i ].length ] = getDateMs ( );
	}

	_action_current[ i ].length += 1;

newCurrent_end:
	pthread_mutex_unlock ( &_action_mutex );
	return ( rt );
}

/// \param [ in ] step : step if of the current action
/// \param [ in ] action : action if of the selected action
/// \brief remove all actions (with couple ste/action) of the current actions array
static void delCurrent ( const uint32_t step, const uint32_t action )
{
	uint8_t stepExchange = 0;
	pthread_mutex_lock ( &_action_mutex );

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

				if ( _action_current[ i ].params[ j ] )
				{
					jsonFree ( &_action_current[ i ].params[ j ], 1 );
					_action_current[ i ].params[ j ] = NULL;
				}
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
	
	pthread_mutex_unlock ( &_action_mutex );
}

/// \param [ in ] step : step if of the current action
/// \param [ in ] action : action if of the selected action
/// \brief set string "status" in params at "done"
static inline void actionDone ( const uint32_t step, const uint32_t action )
{
	pthread_mutex_lock ( &_action_mutex );
	if ( _action_current &&
		( _action_currentLength > step ) &&
		( _action_current[ step ].length > action ) )
	{
		jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );
	}
	pthread_mutex_unlock ( &_action_mutex );
}

/// \param [ in ] step : step if of the current action
/// \param [ in ] action : action if of the selected action
/// \brief set string "status" in params at "done"
static inline void actionWIP ( const uint32_t step, const uint32_t action )
{
	pthread_mutex_lock ( &_action_mutex );
	if ( _action_current &&
		( _action_currentLength > step ) &&
		( _action_current[ step ].length > action ) )
	{
		jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"wip", jT ( str ) );
	}
	pthread_mutex_unlock ( &_action_mutex );
}

/// \param [ in ] step : step if of the current action
/// \param [ in ] action : action if of the selected action
/// \param [ in ] str : string searched
/// \param [ out ] out : value returned
/// \param [ out ] type : type of element returned
/// \brief search a key in the params jsaon  array of a selected action (with couple step/action).
/// \return 0 if value found else value not found
static inline int actionGetFromParams ( const uint32_t step, const uint32_t action, const char * str,  void ** const out, JSON_TYPE *type )
{

	pthread_mutex_lock ( &_action_mutex );
	if ( str &&
		out &&
		_action_current &&
		( _action_currentLength > step ) &&
		( _action_current[ step ].length > action ) )
	{
		jsonGet ( _action_current[ step ].params[ action ], 0, str, out, type );
	}
	pthread_mutex_unlock ( &_action_mutex );
	return ( *out == NULL );
}

/// \param [ in ] step : step if of the current action
/// \param [ in ] action : action if of the selected action
/// \param [ in ] str : string searched
/// \param [ out ] out : value returned
/// \param [ out ] type : type of element returned
/// \brief search a key in the main jaon array of a selected action (with couple step/action).
/// \return 0 if value found else value not found
static inline int actionGetFromMain ( const uint32_t step, const uint32_t action, const char * str,  void ** const out, JSON_TYPE *type )
{

	pthread_mutex_lock ( &_action_mutex );
	if ( str &&
		out &&
		_action_current &&
		( _action_currentLength > step ) &&
		( _action_current[ step ].length > action ) )
	{
		jsonGet ( _action_json, _action_current[ step ].actionsId[ action ], str, out, type );
	}
	pthread_mutex_unlock ( &_action_mutex );
	return ( *out == NULL );
}

/// \brief search the index of a string in the _action_name array
/// \return if >= 0 valid index else key is not in array
static int actionNameToId ( const char * __restrict__ const  key )
{
	uint32_t i = 0;
	pthread_mutex_lock ( &_action_mutex );
	while ( _action_name[i] )
	{
		if ( !strcmp ( _action_name[i], key ) )
		{
			pthread_mutex_unlock ( &_action_mutex );
			return ( i );
		}
		i++;
	}
	
	pthread_mutex_unlock ( &_action_mutex );
	return ( -1 );
}

/// \brief clean all current actions
static void cleanCurrent ( void )
{
	pthread_mutex_lock ( &_action_mutex );
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
	
	pthread_mutex_unlock ( &_action_mutex );
}

// get char* from _action_current array
// return 0 if OK else error
// if error set status to "done"
static inline int getCharFromParams ( const uint32_t step, const uint32_t action, const char * __restrict__ const str, void ** const out )
{

	JSON_TYPE type = jT ( undefined );

	if ( actionGetFromParams ( step, action, str, out, &type ) ||
		type != jT( str ) )
	{
		logDebug ( "ERROR : from param \"%s\" not found %p %d\n", str, *out, type );
		actionDone ( step, action );
		return ( __LINE__ );
	}
	return ( 0 );
}

// get char* from _action_json array
// return 0 if OK else error
// if error set status to "done"
static inline int getCharFromMain ( const uint32_t step, const uint32_t action, const char * __restrict__ const str, void ** const out )
{
	JSON_TYPE type = jT ( undefined );

	if ( actionGetFromMain ( step, action, str, out, &type ) ||
		type != jT( str ) )
	{
		logDebug ( "ERROR : from main \"%s\" not found %p %d\n", str, *out, type );
		actionDone ( step, action );
		return ( __LINE__ );
	}
	return ( 0 );
}

// manage case when action flag for arms are set
// retrun 0 if no flag set else no action need to be done
static inline int testFlagsArms ( const uint32_t step, const uint32_t action, const bool bloquante )
{
	if ( _action_flags &&
		_action_flags->noArm )
	{
		if ( bloquante )
		{
			if ( _action_flags->armWait )
			{ // wait imeout (default)
			}
			else if ( _action_flags->armScan &&
				_kbhit ( ) &&
				_getch ( ) )
			{ // if wait key hit
				actionDone ( step, action );
			}
			else if ( _action_flags->armDone &&
				bloquante )
			{
				actionDone ( step, action );
			}
		}
		return ( __LINE__ );
	}
	return ( 0 );
}

// manage case when action flag for drivers are set
// retrun 0 if no flag set else no action need to be done
static inline int testFlagsDrivers ( const uint32_t step, const uint32_t action, const bool bloquante )
{
	if ( _action_flags &&
		_action_flags->noDrive )
	{
		if ( bloquante )
		{
			if ( _action_flags->driveWait )
			{ // wait imeout (default)
			}
			else if ( _action_flags->driveScan &&
				_kbhit ( ) &&
				_getch ( ) )
			{ // if wait key hit
				actionDone ( step, action );
			}
			else if ( _action_flags->driveDone &&
				bloquante )
			{
				actionDone ( step, action );
			}
		}
		return ( __LINE__ );
	}
	return ( 0 );
}

typedef struct {
	uint32_t stepId;
	uint32_t length;
	uint32_t * nexts;
}actionCleanAndSet_t;

// step used by external timeout to stop everything and set new actions
static void actionCleanAndSet ( void * arg )
{
	printf ( "action timeout\n" );
	actionCleanAndSet_t * a = arg;

	cleanCurrent ( );

	for ( uint32_t k = 0; k < a->length; k++ )
	{
		pthread_mutex_lock ( &_action_mutex );
		getActionId ( _action_json, a->stepId, &(a->nexts[ k ]) );
		pthread_mutex_unlock ( &_action_mutex );
		newCurrent ( a->stepId, a->nexts[ k ] );
		_action_currentIndex++;
	}

	unsetFreeOnExit ( a->nexts );
	free ( a->nexts );
	a->nexts = NULL;

	unsetFreeOnExit ( a );
	free ( a );
	a = NULL;
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
		case aT(set_dyna):
		{ // done
			if ( testFlagsArms ( step, action, false  ) ||
				( _action_dynaFd <= 0 ) )
			{ // fd unvalid of arm disabled
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
		{ // done
			if ( testFlagsArms ( step, action, true  ) ||
				( _action_dynaFd <= 0 ) )
			{ // arm disabled
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
				actionDone ( step, action );
			}
			break;
		}
		case aT(pause):
		{ // done
			void * tmp = NULL;

			if ( getCharFromParams ( step, action, "Temps", &tmp ) )
			{
				return ( __LINE__ );
			}

			pthread_mutex_lock ( &_action_mutex );
			uint32_t temps = atoi ( (char*)tmp );
			if ( getDateMs ( ) - _action_current[ step ].start[ action ] > temps )
			{ // le temps est passé
				jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );
			}
			pthread_mutex_unlock ( &_action_mutex );
			break;
		}
		case aT(set_var):
		{ // done
			char * name = NULL;
			type = jT(undefined);

			if ( getCharFromParams ( step, action, "id", (void**)&name ) )
			{ // no key for variable in params
				return ( __LINE__ );
			}

			// on recupère la cible
			double * target = NULL;
			double tmp = 0.0;
			if ( !actionGetFromParams ( step, action, name, (void**)&target, &type ) ||
				( type != jT(double) ) )
			{ // the var $name doesn't existe
				// the var is not a number... not normal
				target = &tmp;
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
			else
			{
				(*target) = value;
			}
			pthread_mutex_lock ( &_action_mutex );
			jsonSet ( _action_var, 0, name, (void*)&value, jT ( double ) );
			pthread_mutex_unlock ( &_action_mutex );
			break;
		}
		case aT(timeout):
		{ // done
			pthread_mutex_lock ( &_action_mutex );
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
			setFreeOnExit ( arg );

			switch( getNextActions ( _action_json, _action_current[ step ].actionsId[ action ], &(arg->nexts), &(arg->length), true ) )
			{
				case -1:
				{ // nothing more to be done
					free ( arg );
					return ( 0 );
				}
				case 0:
				{ // next step
					setFreeOnExit ( arg->nexts );
					if ( !startTimer( delay, actionCleanAndSet, (void *)arg) )
					{
						free ( arg );
						logDebug ( "externel timeout start failed\n" );
						pthread_mutex_unlock ( &_action_mutex );
						return ( __LINE__ );
					}
					break;
				}
				default:
				{ // error case
					free ( arg );
					pthread_mutex_unlock ( &_action_mutex );
					return ( __LINE__ );
				}
			}
			pthread_mutex_unlock ( &_action_mutex );
			break;
		}
		case aT(servo):
		{ // to be tested
			if ( testFlagsArms ( step, action, false  ) ||
				( _action_pcaFd <= 0 ) )
			{ // fd unvalid of arm disabled
				return ( 0 );
			}
			uint32_t id, angle;

			void * tmp = NULL;
			if ( getCharFromParams ( step, action, "id", &tmp ) )
			{
				return ( __LINE__ );
			}
			id = atoi ( (char*)tmp );

			if ( getCharFromParams ( step, action, "Value", &tmp ) )
			{
				return ( __LINE__ );
			}
			angle = atoi ( (char*)tmp );

			setPCA9685PWM ( id, 0, 210 + angle % 360, _action_pcaFd );
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
		case aT(get_var):
		{
			char * name = NULL;
			type = jT(undefined);

			if ( getCharFromParams ( step, action, "id", (void**)&name ) )
			{
				return ( __LINE__ );
			}

			void * tmp = NULL;
			if ( getCharFromParams ( step, action, "value", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			double value = atof ( ( char*)tmp );

			// le type d'action à faire
			char * op = NULL;
			if ( getCharFromParams ( step, action, "condition", (void**)&op ) )
			{
				return ( __LINE__ );
			}

			// on recupère la cible
			double * target = NULL;
			if ( !actionGetFromParams ( step, action, name, (void**)&target, &type ) )
			{ // the var doesn't existe
				return ( 0 );
			}
			else if ( type != jT(double) )
			{ // the var is not a number... not normal
				logDebug ( "\n" );
				return ( __LINE__ );
			}
			
			// et puis on fini par faire le calcul
			if ( ( !strcmp( op, "!=" ) && ( *target != value ) ) ||
				( !strcmp( op, "==" ) && ( *target == value ) ) ||
				( !strcmp( op, "<" ) && ( value < *target ) ) ||
				( !strcmp( op, ">" ) && ( value > *target  ) ) )
			{
				actionDone ( step, action );
			}
			break;
		}
		case aT(move):
		{
			void * tmp = NULL;
			if ( getCharFromParams ( step, action, "Type", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t _type = atoi ( ( char * )tmp );
			if ( getCharFromParams ( step, action, "Value", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t value = atoi ( ( char * )tmp );
			if ( getCharFromParams ( step, action, "Vitesse", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t vitesse = atoi ( ( char * )tmp );
			if ( getCharFromParams ( step, action, "Accel", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t accel = atoi ( ( char * )tmp );
			if ( getCharFromParams ( step, action, "Decel", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t decel = atoi ( ( char * )tmp );
			if ( getCharFromParams ( step, action, "Tolerance", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t tolerance = atoi ( ( char * )tmp );

			if(isPremierAppel)
			{
				isPremierAppel = false;
				premierAppelMouvement(mRobot, _type, value, vitesse, accel, decel, &posGauche, &posDroite);
			}

			if(setMouvement(mRobot, _type, posGauche, posDroite, tolerance))
			{
				isPremierAppel = true;
				jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );
			}
			break;
		}
		case aT(position):
		{	
			void * tmp = NULL;
			if ( getCharFromParams ( step, action, "Sens", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t sens = atoi ( ( char * )tmp );
			mRobot->cible.sens = sens;
			if ( getCharFromParams ( step, action, "Tolérance", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t tolerance = atoi ( ( char * )tmp );
			mRobot->cible.precision = tolerance;
			if ( getCharFromParams ( step, action, "Accélération", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t acceleration = atoi ( ( char * )tmp );
			mRobot->cible.acc = acceleration;
			if ( getCharFromParams ( step, action, "Décélération", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t deceleration = atoi ( ( char * )tmp );
			mRobot->cible.dec = deceleration;
			if ( getCharFromParams ( step, action, "Vitesse", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t vitesse = atoi ( ( char * )tmp );
			mRobot->cible.vitesseMax = vitesse;
			if ( getCharFromParams ( step, action, "x", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t x = atoi ( ( char * )tmp );
			mRobot->cible.xCible = x;
			if ( getCharFromParams ( step, action, "y", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t y = atoi ( ( char * )tmp );
			mRobot->cible.yCible = y;


			if(calculDeplacement(mRobot))
			{
				jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );
			}
			break;
		}
		case aT(orientation):
		{
			void * tmp = NULL;
			if ( getCharFromParams ( step, action, "Orientation", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t orientation = atoi ( ( char * )tmp );		
			if ( getCharFromParams ( step, action, "Vitesse", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t vitesse = atoi ( ( char * )tmp );
			if ( getCharFromParams ( step, action, "Acc", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t accel = atoi ( ( char * )tmp );
			if ( getCharFromParams ( step, action, "Dec", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t decel = atoi ( ( char * )tmp );
			if ( getCharFromParams ( step, action, "Tolerance", (void**)&tmp ) )
			{
				return ( __LINE__ );
			}
			uint32_t tolerance = atoi ( ( char * )tmp );

			if(isPremierAppel)
			{
				isPremierAppel = false;
				premierAppelTenirAngle(mRobot, orientation, vitesse, accel, decel, &posGauche, &posDroite);
			}

			if(tenirAngle(mRobot, posGauche, posDroite, tolerance))
			{
				isPremierAppel = true;
				jsonSet ( _action_current[ step ].params[ action ], 0, "status", &"done", jT ( str ) );
			}

			pthread_mutex_lock ( &_action_mutex );
			// x, y, vitesse, acceleration, tolerance, sens
			// TODO
			pthread_mutex_unlock ( &_action_mutex );
			break;
		}
		case aT(stopMove):
		{
			break;
		}
		case aT(pick):
		{
			void * tmp = NULL;
			if ( getCharFromParams ( step, action, "id", &tmp ) )
			{
				return ( __LINE__ );
			}
			break;
		}
		case aT(blocked):
		case aT(place):	
		case aT(getGpio):
		case aT(setGpio):
		case aT(setPasAPas):
		case aT(start):
		case aT(sequence):
		case aT(end):
		case aT(none):
		{
			break;
		}
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
int actionManagerInit ( const char * __restrict__ const file , Robot* robot)
{
	actionManagerDeInit ( );
	pthread_mutex_lock ( &_action_mutex );

	mRobot = robot;

	if ( jsonParseFile ( file, &_action_json, &_action_jsonLength ) )
	{
		pthread_mutex_unlock ( &_action_mutex );
		return ( __LINE__ );
	}

	if ( jsonParseString ( "{}", &_action_var, &_action_varLength ) )
	{
		logDebug ( "\n" );
		pthread_mutex_unlock ( &_action_mutex );
		return ( __LINE__ );
	}

	pthread_mutex_unlock ( &_action_mutex );
	return ( 0 );
}

int actionManagerDeInit ( void )
{
	pthread_mutex_lock ( &_action_mutex );
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
	pthread_mutex_unlock ( &_action_mutex );

	mRobot = NULL;

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
		pthread_mutex_lock ( &_action_mutex );
		jsonGet ( _action_json, actionId, "nomAction", (void**)&name, NULL );
		pthread_mutex_unlock ( &_action_mutex );

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
		// pour toutes les actions enregistrées dans le tableau
		for ( uint32_t j = 0; _action_current && j < _action_current[ i ].length; j++ )
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

				pthread_mutex_lock ( &_action_mutex );
				void * tmp = jsonGet ( _action_current[ i ].params[ j ], 0, "status", (void**)&status, &type );
				pthread_mutex_unlock ( &_action_mutex );
				if ( !tmp )
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
				// si on peut faire plusieurs niveau de l'arbre généalogique en une boucle
				// alors on doit traiter les action à ce niveau
				execOne ( i, j );
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

void actionManagerSetFlags ( ActionFlag * __restrict__ const f )
{
	_action_flags = f;
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
