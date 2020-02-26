#include "actionExtract.h"

#include <errno.h>
#include <stdio.h>
#include "../lib/log/log.h"

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

	// verify if index is valid and if step associated is an object
	if ( *stepId < data[ *strategieId ].length )
	{
		if ( data[ *strategieId ].type[ *stepId ] == jT( obj ) )
		{
			*stepId = *(uint32_t*)data[ *strategieId ].value[ *stepId ];
			return ( 0 );
		}
	}

	return ( -1 );
}

// TODO : fonction qui calcule l'ordre des etapes
int getNextStepId ( const json_el * const data, uint32_t * stepId )
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

	// verify if index is valid and if step associated is an object
	if ( *stepId < data[ *strategieId ].length )
	{
		if ( data[ *strategieId ].type[ *stepId ] == jT( obj ) )
		{
			*stepId = *(uint32_t*)data[ *strategieId ].value[ *stepId ];
			return ( 0 );
		}
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