#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "lib/log/log.h"

#include "action.h"
#include "actionExtract.h"

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
	json_el ** params;
	uint32_t length;
}
currentWork;

static currentWork * _action_current = NULL;
static uint32_t _action_currentLength = 0;

static const char * _action_name[] = {
	[aT(none)] = "none",
	[aT(servo)] = "setServo",
	[aT(dyna)] = "setDyna",
	[aT(get_var)] = "getVar",
	[aT(set_var)] = "setVar",
	[aT(last)] = NULL
};

static int newCurrent ( uint32_t step, uint32_t action )
{
	uint32_t i;
	void *tmp = NULL;

	for ( i = 0; i < _action_currentLength; i++ )
	{
		if ( _action_current[ i ].stepId == i )
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
		_action_currentLength += 1;
	}

	// current actions ID
	tmp = realloc ( _action_current[ i ].actionsId, sizeof ( uint32_t ) * _action_current[ i ].length + 1 );
	if ( !tmp )
	{
		logDebug ( "\n" );
		return ( -__LINE__ );
	}
	else
	{
		((uint32_t*)tmp)[ _action_current[ i ].length ] = action;
		_action_current[ i ].actionsId = tmp;
	}

	// current parameters
	tmp = realloc ( _action_current[ i ].params, sizeof ( json_el* ) * _action_current[ i ].length + 1 );
	if ( !tmp )
	{
		logDebug ( "\n" );
		return ( -__LINE__ );
	}
	else
	{ // add params to the current array
		getActionParams ( _action_json, action, &(((json_el**)tmp)[ _action_current[ i ].length ]), NULL );
		_action_current[ i ].params = tmp;
	}

	// current actions timeout
	tmp = realloc ( _action_current[ i ].timeout, sizeof ( uint32_t ) * _action_current[ i ].length + 1 );
	if ( !tmp )
	{
		logDebug ( "\n" );
		return ( -__LINE__ );
	}
	else
	{
		void * t = NULL;
		JSON_TYPE type;

		if ( !jsonGet ( _action_json, action, "Timeout", &t, &type ) &&
			type == jT ( str ) ||
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

	// current actions start time
	tmp = realloc ( _action_current[ i ].timeout, sizeof ( time_t ) * _action_current[ i ].length + 1 );
	if ( !tmp )
	{
		logDebug ( "\n" );
		return ( -__LINE__ );
	}
	else
	{
		((time_t*)tmp)[i] = time ( NULL );
		_action_current[ i ].start = tmp;
	}

	_action_current[ i ].length += 1;

	return ( 0 );
}

static void delCurrent ( uint32_t step, uint32_t action )
{
	for ( uint32_t i = 0; i < _action_currentLength; i++ )
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

			if ( _action_current[ i ].params[ j ] )
			{
				jsonFree ( &_action_current[ i ].params[ j ], 1 );
			}

			_action_current[ i ].params[ j ] = _action_current[ i ].params[ j + 1 ];
			_action_current[ i ].params[ j + 1 ] = NULL;
		}

		_action_current[ i ].length -= exchange;
	}
}

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

int actionManagerInit ( const char * const file )
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

	for ( uint32_t i = 0; i < _action_currentLength; i++ )
	{
		for ( uint32_t j = 0; j < _action_current[ i ].length; j++ )
		{
			jsonFree ( &_action_current[ i ].params[j], 1 );
		}
		free ( _action_current[ i ].actionsId );
		free ( _action_current[ i ].timeout );
		free ( _action_current[ i ].params );
		free ( _action_current[ i ].start );

		_action_current[ i ].actionsId = NULL;
		_action_current[ i ].timeout = NULL;
		_action_current[ i ].params = NULL;
		_action_current[ i ].start = NULL;
	}

	free ( _action_current );
	_action_current = NULL;
	_action_currentLength = 0;
	
	return ( 0 );
}

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
	for ( uint32_t i = 0; i < _action_currentLength; i++ )
	{
		for ( uint32_t j = 0; j < _action_current[ i ].length; j++ )
		{
			if ( !_action_current[ i ].params )
			{ // no parameters
				continue;
			}

			time_t now = time ( NULL );
			// if ( _action_current[ i ]. )
			
			// uint32_t timeout = 

			if ( now - _action_current[ i ].start[ j ] > _action_current[ i ].timeout[ j ] )
			{

			}
		}
	}
	return ( 0 );
}

int actionManagerMain ( const uint32_t actionId, json_el * __restrict__ const params )
{
	char * actionName = NULL;
	if ( jsonGet ( _action_json, actionId, "nomAction", (void**)&actionName, NULL ) )
	{
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	switch ( actionNameToId ( actionName ) )
	{
		// case aT(servo):
		// { // done
		// 	//setPCA9685PWM ( atoi ( listAction[ indiceAction ].params[ 0 ] ), 0, 210 + atoi ( listAction[ indiceAction ].params[ 1 ] ) % 360, *_management_pca9685 );
		// 	listAction[indiceAction].isDone = 1;
		// 	break;
		// }
		// case aT(dyna):
		// { // done
		// 	if ( _management_flagAction->noArm )
		// 	{
		// 		if ( _management_flagAction->armScan )
		// 		{
		// 			while ( _kbhit ( ) )
		// 			{
		// 				listAction[indiceAction].isDone = 1;
		// 			}
		// 		}
		// 		else if ( _management_flagAction->armWait )
		// 		{ 
		// 		}
		// 		else
		// 		{ // arm done
		// 			listAction[indiceAction].isDone = 1;
		// 		}
		// 	}
		// 	else
		// 	{
		// 		if ( setVitesseDyna ( atoi ( listAction[ indiceAction ].params[ 0 ]), ( int )( 10.23*atoi ( listAction[ indiceAction ].params[ 2 ] ) ) ) )
		// 		{
		// 			logVerbose ( "Erreur set vitesse dyna \n" );
		// 		}
		// 		if ( setPositionDyna ( atoi ( listAction[ indiceAction ].params[ 0 ]), ( int )( atoi ( listAction[ indiceAction ].params[ 1 ] ) ) ) )
		// 		{
		// 			logVerbose ( "Erreur set angle dyna \n" );
		// 		}
		// 		logDebug ("Dyna : id %s, angle %s, vitesse %s\n",listAction[ indiceAction ].params[ 0 ],listAction[ indiceAction ].params[ 1 ],listAction[ indiceAction ].params[ 2 ]);
		// 		listAction[indiceAction].isDone = 1;
		// 	}
		// 	break;
		// }
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
		// case TYPE_ATTENTE_DYNA:
		// { // done
		// 	//id:param0 value:param1
		// 	if ( abs ( getPositionDyna ( atoi ( listAction[ indiceAction ].params[ 0 ] ) ) - atoi ( listAction[ indiceAction ].params[ 1 ] ) ) < 5 )
		// 	{
		// 		listAction[indiceAction].isDone = 1;
		// 	}
		// 	break;
		// }
		// case TYPE_ATTENTE_TEMPS:
		// { // done
		// 	gettimeofday ( &now, NULL );
		// 	logDebug ( "attente %d type : %d\n", ( now.tv_sec * 1000000 + now.tv_usec - listAction[ indiceAction ].heureCreation ),1000* atoi ( listAction[indiceAction].params[ 0 ]) );
		// 	if ( ( int )( now.tv_sec * 1000000 + now.tv_usec - listAction[ indiceAction ].heureCreation ) >= ( 1000 * atoi ( listAction[indiceAction].params[ 0 ] ) ) )
		// 	{
		// 		listAction[indiceAction].isDone = 1;
		// 	}
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
			JSON_TYPE type = jT(undefined);

			if ( !jsonGet ( params, 0, "key", (void**)&name, &type ) )
			{ // no key for variable in params
				logDebug ( "\n" );
				return ( __LINE__ );
			}

			if ( type != jT(str) )
			{ // the key is no a string
				logDebug ( "\n" );
				return ( __LINE__ );
			}

			void * value = NULL;

			jsonGet ( _action_var, 0, name, &value, &type );
			break;
		}
		case aT(set_var):
		{
			char * name = NULL;
			JSON_TYPE type = jT(undefined);

			if ( !jsonGet ( params, 0, "name", (void**)&name, &type ) )
			{ // no key for variable in params
				logDebug ( "\n" );
				return ( __LINE__ );
			}

			if ( type != jT(str) )
			{ // the key is no a string
				logDebug ( "\n" );
				return ( __LINE__ );
			}

			char * op = NULL;
			if ( !jsonGet ( params, 0, "op", (void**)&op, &type ) )
			{ // no key for variable in params
				logDebug ( "\n" );
				return ( __LINE__ );
			}

			if ( type != jT(str) )
			{ // the key is no a string
				logDebug ( "\n" );
				return ( __LINE__ );
			}

			double * value = NULL;
			if ( !jsonGet ( params, 0, "value", (void**)&value, &type ) )
			{ // no key for variable in params
				logDebug ( "\n" );
				return ( __LINE__ );
			}

			if ( type != jT(double) )
			{ // the key is no a string
				logDebug ( "\n" );
				return ( __LINE__ );
			}

			double * target = NULL;
			double tmp = 0.0;

			if ( !jsonGet ( params, 0, name, (void**)&target, &type ) )
			{ // the var $name doesn't existe
				target = &tmp;
			}
			
			if ( type != jT(double) )
			{ // the var is not a number... not normal
				logDebug ( "\n" );
				return ( __LINE__ );
			}
			else
			{
				if ( !strcmp( op, "+" ) )
				{
					(*value) += (*target);
				}
				else if ( !strcmp( op, "*" ) )
				{
					(*value) *= (*target);
				}
				else if ( !strcmp( op, "/" ) )
				{
					(*value) = (*target) / (*value);
				}
				else if ( !strcmp( op, "-" ) )
				{
					(*value) = (*target) - (*value);
				}

				jsonGet ( _action_var, 0, name, (void**)&value, &type );
			}
			break;
		}
		case aT(none):
		case aT(last):
		default:
		{
			break;
		}
	}
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

			switch ( getNextActions ( _action_json, stepId, actionId, &tableNext, &length, true ) )
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

			switch ( getNextActions ( _action_json, stepId, actionId, &tableNext, &length, false ) )
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
	for ( uint32_t i = 0; i < _action_currentLength; i++ )
	{
		printf ( "step : %d\n", _action_current[ i ].stepId );

		for ( uint32_t j = 0; j < _action_current[ i ].length; j++ )
		{
			char * name;
			jsonGet ( _action_json, _action_current[ i ].actionsId[ j ], "nomAction", (void**)&name, NULL );
			printf ( "\taction : %d = %s\n", _action_current[ i ].actionsId[ j ], name );
		}
	}
}
