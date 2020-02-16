#include "lib/jsonParser/jsonParser.h"

json_el * getNextEtape ( json_el *data, uint32_t dataLength )
{
	uint32_t * strategieId = 0;
	JSON_TYPE type;

	jsonGetRecursive ( data, 0, "Strategie", &strategieId, &type );

	for ( int i = 0; i < data[ *strategieId ].length; i++ )
	{
		uint32_t etapeId = *(uint32_t*)data[ *strategieId ].value[ i ];

		char * nomEtape = NULL;
		uint32_t * sequenceId = NULL;

		jsonGetRecursive ( data, etapeId, "nomEtape", &nomEtape, NULL );
		jsonGetRecursive ( data, etapeId, "arraySequence", &sequenceId, NULL );

		printf ( "%d : %s\n", i, nomEtape );

		for ( int j = 0; j < data[ *sequenceId ].length; j++ )
		{
			uint32_t actionId = *(uint32_t*)data[ *sequenceId ].value[ j ];
			char * nomAction = NULL;
			jsonGetRecursive ( data, actionId, "nomAction", &nomAction, NULL );

			printf ( "         %s\n", nomAction );

			uint32_t * paramId = NULL;
			jsonGetRecursive ( data, actionId, "arrayParam", &paramId, &type );

			if ( !paramId )
			{
				printf ( "no param\n" );
			}
			else if ( *paramId && type == jT(array) )
			{
				json_el * json = NULL;
				uint32_t l = 0;
				jsonParseString ( "{}", &json, &l );
				for ( int k = 0; k < data[ *paramId ].length; k++ )
				{
					uint32_t paramIdInArray = *(uint32_t*)data[ *paramId ].value[ k ];
		
					char * key = NULL;
					char * value = NULL;
					jsonGetRecursive ( data, paramIdInArray, "nomParam", &key, NULL );
					jsonGetRecursive ( data, paramIdInArray, "defaultValue", &value, &type );

					// printf ( "                %s %s %d\n", key, value );

					jsonSet ( json, 0, key, value, jT(str) );
				}

				jsonPrint ( json, 0, 0 );
				jsonFree ( &json, l );
			}
		}
	}


	return ( 0 );
}

int initActionNew ( )
{
	json_el *data = NULL;
	uint32_t dataLength = 0;

	if ( jsonParseFile ( "export.json", &data, &dataLength ) )
	{
		return ( __LINE__ );
	}

	getNextEtape ( data, dataLength );

	jsonFree ( &data, dataLength );

	return ( 0 );
}

