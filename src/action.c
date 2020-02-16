#include <errno.h>
#include <stdio.h>
#include "lib/log/log.h"

#include "lib/jsonParser/jsonParser.h"

/// \retrun 0 : valid id found
///     -1 : no step remaining
///     X : line error
int getNextStep ( const json_el * const data, uint32_t * const out )
{
	static json_el * lastJson = NULL;
	static uint32_t * strategieId = NULL;
	static JSON_TYPE type = jT( undefined );
	static uint32_t index = 0;

	if ( data )
	{ // if data provided then clean all static vars
		lastJson = data;
		strategieId = NULL;
		index = 0;
		type = jT( undefined );
	}
	else if ( !lastJson )
	{
		errno = EINVAL;
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	if ( ( strategieId == NULL ) &&
		!jsonGetRecursive ( lastJson, 0, "Strategie", (void*)&strategieId, &type ) )
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
	while ( index < lastJson[ *strategieId ].length )
	{
		*out = *(uint32_t*)lastJson[ *strategieId ].value[ index ];

		if ( lastJson[ *strategieId ].type[ index++ ] == jT( obj ) )
		{
			return ( 0 );
		}
	}

	return ( -1 );

		// for ( int i = 0; i < data[ *strategieId ].length; i++ )
		// {
		// 	uint32_t etapeId = *(uint32_t*)data[ *strategieId ].value[ i ];

		// 	char * nomEtape = NULL;
		// 	uint32_t * sequenceId = NULL;

		// 	jsonGetRecursive ( data, etapeId, "nomEtape", &nomEtape, NULL );
		// 	jsonGetRecursive ( data, etapeId, "arraySequence", &sequenceId, NULL );

		// 	printf ( "%d : %s\n", i, nomEtape );

		// 	for ( int j = 0; j < data[ *sequenceId ].length; j++ )
		// 	{
		// 		uint32_t actionId = *(uint32_t*)data[ *sequenceId ].value[ j ];
		// 		char * nomAction = NULL;
		// 		jsonGetRecursive ( data, actionId, "nomAction", &nomAction, NULL );

		// 		printf ( "         %s\n", nomAction );

		// 		uint32_t * paramId = NULL;
		// 		jsonGetRecursive ( data, actionId, "arrayParam", &paramId, &type );

		// 		if ( !paramId )
		// 		{
		// 			printf ( "no param\n" );
		// 		}
		// 		else if ( *paramId && type == jT(array) )
		// 		{
		// 			json_el * json = NULL;
		// 			uint32_t l = 0;
		// 			jsonParseString ( "{}", &json, &l );
		// 			for ( int k = 0; k < data[ *paramId ].length; k++ )
		// 			{
		// 				uint32_t paramIdInArray = *(uint32_t*)data[ *paramId ].value[ k ];
			
		// 				char * key = NULL;
		// 				char * value = NULL;
		// 				jsonGetRecursive ( data, paramIdInArray, "nomParam", &key, NULL );
		// 				jsonGetRecursive ( data, paramIdInArray, "defaultValue", &value, &type );

		// 				// printf ( "                %s %s %d\n", key, value );

		// 				jsonSet ( json, 0, key, value, jT(str) );
		// 			}

		// 			jsonPrint ( json, 0, 0 );
		// 			jsonFree ( &json, l );
		// 		}
		// 	}
		// }
		// return ( 0 );
}

int getNextAction ( const json_el * const data, const uint32_t step, uint32_t * const out )
{
	static json_el * lastJson = NULL;
	static JSON_TYPE type = jT( undefined );
	static uint32_t * sequenceId = NULL;
	static uint32_t stepId = 0;
	static uint32_t index = 0;

	if ( data )
	{ // if data provided then clean all static vars
		lastJson = data;
		sequenceId = NULL;
		index = 0;
		stepId = step;
		type = jT( undefined );
	}
	else if ( !lastJson )
	{
		errno = EINVAL;
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	if ( ( sequenceId == NULL ) &&
		!jsonGetRecursive ( lastJson, stepId, "arraySequence", (void*)&sequenceId, &type ) )
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
	while ( index < lastJson[ *sequenceId ].length )
	{
		*out = *(uint32_t*)lastJson[ *sequenceId ].value[ index ];

		if ( lastJson[ *sequenceId ].type[ index++ ] == jT( obj ) )
		{
			return ( 0 );
		}
	}
	return ( -1 );
}

int getParams ( const json_el * const data, const uint32_t action, json_el ** out, uint32_t * outLength )
{

	if ( !data )
	{ // if data provided then clean all static vars
		errno = EINVAL;
		return ( __LINE__ );
	}

	uint32_t * paramArrayId = NULL;
	JSON_TYPE type = jT( undefined );

	if ( !jsonGetRecursive ( data, action, "arrayParam", (void*)&paramArrayId, &type ) )
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

	for ( int k = 0; k < data[ *paramArrayId ].length; k++ )
	{
		uint32_t paramId = *(uint32_t*)data[ *paramArrayId ].value[ k ];

		char * key = NULL;
		char * value = NULL;
		jsonGetRecursive ( data, paramId, "nomParam", &key, NULL );
		jsonGetRecursive ( data, paramId, "defaultValue", &value, &type );
		jsonSet ( *out, 0, key, value, jT(str) );
	}

	return ( 0 );
}

int initActionNew ( )
{
	int err = 0;
	json_el *data = NULL;
	uint32_t dataLength = 0;

	if ( jsonParseFile ( "res/export.json", &data, &dataLength ) )
	{
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	int stop = 10;

	uint32_t id = 0;
	void * ptr = data;
	do 
	{
		if ( getNextStep ( ptr, &id ) )
		{
			logDebug ( "\n" );
			break;
		}
		ptr = NULL;
		
		printf ( "etape %d\n", id );

		uint32_t actionId = 0;
		ptr = data;
		do
		{
			if ( getNextAction ( ptr, id, &actionId ) )
			{
				logDebug ( "\n" );
				break;
			}
			ptr = NULL;

			char * name = NULL;
			jsonGet ( data, actionId, "nomAction", &name, NULL );
			printf ( "    action %d : %s\n", actionId, name );

			json_el * params = NULL;
			uint32_t length = 0;
			if ( getParams ( data, actionId, &params, &length ) )
			{
				printf ( "    no parameters found\n" );
			}
			else
			{
				printf ( "    parameters :\n" );
				if ( params->length == 0 )
				{
					printf ( "\tarray empty\n" );
				}
				else
				{
					jsonPrint ( params, 0, 1 );
				}
				jsonFree ( &params, length );
			}
		}
		while ( true );
	}
	while ( true );

	jsonFree ( &data, dataLength );

	return ( 0 );
}

