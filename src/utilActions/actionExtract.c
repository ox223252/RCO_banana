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

#include "actionExtract.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "actionExtractDefines.h"

#include "../lib/log/log.h"
#include "../lib/freeOnExit/freeOnExit.h"
#include "../actionGetEnv.h" // need exctra from env
#include "../date.h"

static uint32_t * stepDone = NULL; ///< used to store what steps was done

////////////////////////////////////////////////////////////////////////////////
/// \fn static double getTaux ( const json_el * const data, const uint32_t id );
/// \param [ in ] data :  pointer on json main database
/// \param [ in ] id : id of the step to evaluate
/// \return the rate of point available for a specifi step at this moment in the
///     game. 
/// \note its the AI part of our software
////////////////////////////////////////////////////////////////////////////////
static double getTaux ( const json_el * const data, const uint32_t id )
{
	// recuperation de nombre de verres
	uint16_t nbCups = getNbCups ( );

	// recuperation du temps
	struct timeval time;
	getChronoValue ( &time );

	logDebug ( "nb cup : %d at %d\n", nbCups, time.tv_sec );

	// recuperation du bon taux
	uint32_t *index;
	JSON_TYPE type;

	if ( !jsonGet ( data, id, RATE_ARRAY, (void**)&index, &type ) )
	{ // pas de TauxArray trouvé
		return ( -1.0 );
	}

	if ( type != jT(array) )
	{
		logDebug ( "type : %d %d\n", type, index );
		logDebug ( "     : %d\n", id );
		return ( -1.0 );
	}

	for ( uint32_t i = 0; i < data[ *index ].length; i++ )
	{
		double *cond = NULL;
		double *value = NULL;
		double *param = NULL;

		// on recupère l'id du l'objet 'i' dans le tableau data[*index]
		uint32_t nextObjId = *(uint32_t*)data[ *index ].value[ i ];

		if ( !jsonGet ( data, nextObjId, CONDITINON, (void**)&cond, NULL ) ||
			!jsonGet ( data, nextObjId, VALUE, (void**)&value, NULL ) ||
			!jsonGet ( data, nextObjId, PARAMETER, (void**)&param, NULL ) )
		{ // une des trois valeur precedente non trouvée
			logDebug ( "Taux array not correct\n" );
			continue;
		}

		double comparator;
		switch ( (param_t)((int)*param) )
		{
			case TIME:
			{
				comparator = getChronoMs ( );
				break;
			}
			case CUPS:
			{
				comparator = nbCups;
				break;
			}
			default:
			{
				break;
			}
		}

		double *taux;
		if ( !jsonGet ( data, nextObjId, RATE, (void**)&taux, NULL ) )
		{ // on à pas trouvé le taux... pas normal ça
			logDebug ( "Taux not found\n" );
			continue;
		}

		switch ( (condition_t)((int)*cond) )
		{
			case UT:
			{
				if ( comparator > *value )
				{
					return ( *taux );
				}
				break;
			}
			case EQ:
			{
				if ( comparator == *value )
				{
					return ( *taux );
				}
				break;
			}
			case LT:
			{
				if ( comparator < *value )
				{
					return ( *taux );
				}
				break;
			}
			default:
			{
				break;
			}
		}
	}
	return ( 0 );
}

////////////////////////////////////////////////////////////////////////////////
/// \param [ in ] data : pointeur on json main database
/// \param [ in ] a : index of first element in the database
/// \param [ in ] b : index of second element in the database
/// \return -1 on error, 0 if a beter than b or 1 if b better than a
////////////////////////////////////////////////////////////////////////////////
static inline int stepCmp (  const json_el * const data, const uint32_t a, const uint32_t b )
{
	double *a_points = NULL;
	double *b_points = NULL;


	jsonGet ( data, a, "nbPoints", (void**)&a_points, NULL );
	jsonGet ( data, b, "nbPoints", (void**)&b_points, NULL );

	double a_taux = getTaux ( data, a );
	double b_taux = getTaux ( data, b );

	#ifdef MODE_DEBUG
	{
		char *a_name;
		char *b_name;
		jsonGet ( data, a, "nomEtape", (void**)&a_name, NULL );
		jsonGet ( data, b, "nomEtape", (void**)&b_name, NULL );
		logDebug ( "%s %lf : %lf\n", a_name, *a_points, a_taux );
		logDebug ( "%s %lf : %lf\n", b_name, *b_points, b_taux );
	}
	#endif

	if ( *a_points * a_taux > *b_points * b_taux )
	{
		return ( 0 );
	}

	return ( 1 );
}

/// \retrun 0 : valid id found
///     -1 : no step remaining
///     X : line error
int getStepId ( const json_el * const data, uint32_t * const stepId )
{
	if ( !data ||
		!stepId )
	{
		errno = EINVAL;
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	uint32_t * strategieId = 0;
	JSON_TYPE type = jT( undefined );

	if ( !jsonGetRecursive ( data, 0, "Strategie", (void*)&strategieId, &type ) )
	{ // no object found
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	if ( type != jT( array ) )
	{ // strategie is not an array
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	if ( *stepId == 0 )
	{ // only for the first step or for a remake of the full game
		if ( stepDone )
		{
			unsetFreeOnExit ( stepDone );
			free ( stepDone );
		}

		stepDone = malloc ( sizeof ( *stepDone ) * data[ *strategieId ].length );
		if ( !stepDone )
		{
			return ( __LINE__ );
		}
		setFreeOnExit ( stepDone );

		memset ( stepDone, 0, sizeof ( *stepDone ) * data[ *strategieId ].length );
	}
		
	uint32_t best = 0; // a step ID can't be zero, because it's the stategy ID

	// search the new step to do
	for ( uint32_t i = 0; i < data[ *strategieId ].length; i++ )
	{
		if ( data[ *strategieId ].type[ i ] != jT( obj ) )
		{ // if it's not an object.. we have a problem
			continue;
		}

		// get id in the stategy array of the nex step
		uint32_t tmp = *(uint32_t*) data[ *strategieId ].value[ i ];

		// verify if we have already done this step [ tmp ]
		bool thisOneDone = false;
		for ( uint32_t j = 0; j < data[ *strategieId ].length; j++ )
		{
			if ( stepDone && 
				( tmp == stepDone[ j ] ) )
			{
				thisOneDone = true;
				break;
			}
			else if ( !tmp )
			{
				break;
			}
		}

		if ( thisOneDone )
		{ // if done we continue
			continue;
		}

		if ( !best )
		{ // if its the first step meet we saved it
			best = tmp;
		}
		else if ( stepCmp ( data, best, tmp ) )
		{ // else we compare the value of the two step (AI part here)
			best = tmp;
		}
	}

	if ( best )
	{ // if we found a valid step to do
		int i = 0;
		while ( stepDone[ i ] )
		{
			printf ( "%d\n", stepDone[ i ] );
			i++;
		}
		stepDone[ i ] = best;
		*stepId = best;
		
		return ( 0 );
	}

	return ( -1 );
}

int getActionId ( const json_el * const data, const uint32_t stepId, uint32_t * const actionId )
{
	if ( !data ||
		!actionId )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	JSON_TYPE type = jT( undefined );
	uint32_t * sequenceId = NULL;

	if ( ( sequenceId == NULL ) &&
		!jsonGetRecursive ( data, stepId, "arraySequence", (void*)&sequenceId, &type ) )
	{ // no object found
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	if ( type != jT( array ) )
	{ // strategie is not an array
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	// verify if index is valid and if action associated is an object
	if ( *actionId < data[ *sequenceId ].length )
	{
		if ( data[ *sequenceId ].type[ *actionId ] == jT( obj ) )
		{
			*actionId = *(uint32_t*)data[ *sequenceId ].value[ *actionId ];
			return ( 0 );
		}
	}

	return ( -1 );
}

int getNextActions ( const json_el * const data, const uint32_t actionId,
	uint32_t ** const out, uint32_t * const length, bool timeout )
{
	if ( !data ||
		!out ||
		!length )
	{
		errno = EINVAL;
		logDebug ( "\n" ); // c'est pas un tableau
		return ( __LINE__ );
	}

	char search[16] = "arrayGirl";

	if ( timeout )
	{
		sprintf ( search, "arrayTimeout" ); 
	}

	JSON_TYPE type;
	uint32_t *fils = NULL;

	if ( !jsonGet ( data, actionId, search, (void**)&fils, &type ) )
	{ // pas d'element'
		return ( -1 );
	}

	if ( type != jT( array ) )
	{
		logDebug ( "%d\n", type ); // c'est pas un tableau
		return ( __LINE__ );
	}

	if ( data[ *fils ].length == 0 )
	{ // tableau vide
		return ( -1 );
	}

	void * tmp = malloc ( sizeof ( uint32_t ) * data[ *fils ].length );
	if ( !tmp )
	{
		logDebug ( "\n" ); // problème de malloc
		return ( __LINE__ );
	}

	*out = tmp;
	*length = data[ *fils ].length;

	if ( !timeout )
	{
		sprintf ( search, "indiceFille" ); 
	}
	else
	{
		sprintf ( search, "indiceTimeout" ); 
	}

	for ( uint32_t i = 0; i < data[ *fils ].length; i++ )
	{
		double *id = NULL;

		if( !jsonGet ( data, *(uint32_t*)data[ *fils ].value[ i ], search, (void**)&id, &type ) )
		{
			logDebug ( "\n" ); // error il manque l'action suivante
			return ( __LINE__ );
		}
		if ( type != jT( double ) )
		{
			logDebug ( "\n" ); // error la donnée n'est pas un nombre
			return ( __LINE__ );
		}
		(*out)[ i ] = (uint32_t)*id;
	}

	return ( 0 );
}

int getActionParams ( const json_el * const data, const uint32_t actionId, json_el ** out, uint32_t * outLength )
{

	if ( !data ||
		!outLength ||
		*outLength )
	{ // if data provided then clean all static vars
		errno = EINVAL;
		return ( __LINE__ );
	}

	uint32_t * paramArrayId = NULL;
	JSON_TYPE type = jT( undefined );

	if ( !jsonGetRecursive ( data, actionId, "arrayParam", (void*)&paramArrayId, &type ) )
	{ // no object found
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	if ( type != jT( array ) )
	{ // strategie is not an array
		logDebug ( "%d %d\n", *paramArrayId, type );
		return ( __LINE__ );
	}

	jsonParseString ( "{}", out, outLength );

	for ( uint32_t k = 0; k < data[ *paramArrayId ].length; k++ )
	{
		uint32_t paramId = *(uint32_t*)data[ *paramArrayId ].value[ k ];

		char * key = NULL;
		char * value = NULL;
		jsonGetRecursive ( data, paramId, "nomParam", (void*)&key, &type );
		if ( type != jT( str ) )
		{ // key invalid
			continue;
		}

		jsonGetRecursive ( data, paramId, "defaultValue", (void*)&value, &type );
		jsonSet ( *out, 0, key, value, type );
	}

	return ( 0 );
}

int __attribute__((weak)) main ( void )
{
	json_el *data = NULL;
	uint32_t dataLength = 0;

	if ( jsonParseFile ( "res/export.json", &data, &dataLength ) )
	{
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	// id of the step in the sequence array
	uint32_t stepIndex = 0;
	do 
	{
		// id of the step in main data memory array
		uint32_t stepId = stepIndex;

		if ( getStepId ( data, &stepId ) )
		{
			logDebug ( "\n" );
			break;
		}
		
		printf ( "etape %d\n", stepId );

		uint32_t actionIndex = 0;
		do
		{
			uint32_t actionId = actionIndex;

			if ( getActionId ( data, stepId, &actionId ) )
			{
				logDebug ( "\n" );
				break;
			}

			char * name = NULL;
			jsonGet ( data, actionId, "nomAction", (void**)&name, NULL );
			printf ( "    action %d : %s\n", actionId, name );

			json_el * params = NULL;
			uint32_t length = 0;
			if ( getActionParams ( data, actionId, &params, &length ) )
			{
				printf ( "\tno parameters found\n" );
			}
			else
			{
				printf ( "\tparameters :\n" );
				if ( params->length == 0 )
				{
					printf ( "\tarray empty\n" );
				}
				else
				{
					printf("\t{\n" );
					jsonPrint ( params, 0, 1 );
					printf("\t}\n" );
				}
				jsonFree ( &params, length );
			}
			actionIndex++;
		}
		while ( true );

		stepIndex++;
	}
	while ( true );

	jsonFree ( &data, dataLength );

	return ( 0 );
}