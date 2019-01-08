#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <stdint.h>

#include "mqtt.h"
#include "../freeOnExit/freeOnExit.h"
#include "../log/log.h"

static struct
{
	char ** list;
	uint16_t length;
}
_mqtt_topics = { NULL, 0 };

static struct mosquitto * _mqtt_mosq = NULL;
static struct
{
	uint8_t connected:1,
		ackDone:1;
}
_mqtt_flags = { 0 };

#pragma GCC diagnostic ignored "-Wunused-parameter"
static void MQTTAfterExit ( void * arg )
{
	mosquitto_destroy ( _mqtt_mosq );
	mosquitto_lib_cleanup ( );
}

static void MQTTBeforeExit ( void * arg )
{
	if ( _mqtt_flags.connected )
	{ // to stop correctly infinite loop
		mosquitto_disconnect ( _mqtt_mosq );
		_mqtt_flags.connected = false;
	}

	logVerbose ( "\n\
		mqtt disconnection required,\n\
		wait for normal temrination,\n\
		or press ctrl + C.\n" );
}

static void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	logVerbose ( "Topic : %s payload : %s\n", message->topic, message->payload );
}

static void my_connect_callback ( struct mosquitto *mosq, void *obj, int result )
{
	if ( !result )
	{
		_mqtt_flags.ackDone = true;

		for ( int i = 0; i < _mqtt_topics.length; i++ )
		{
			mqtt_subscribe ( NULL, _mqtt_topics.list[ i ], 1 );
		}
	}
	else if ( result )
	{
		fprintf ( stderr, "%s\n", mosquitto_connack_string ( result ) );
	}
}
#pragma GCC diagnostic pop

static void *infiniteLoop ( void *arg )
{
	mosquitto_loop_forever ( ( struct mosquitto * )arg, 0, 1 );
	pthread_exit ( NULL );
}

int mqtt_init ( const char * const restrict name, const char* ipAddress, int port )
{
	pthread_t threadID;
	int rc;

	if ( _mqtt_mosq )
	{ // multiple init call
		errno = EINVAL;
		return ( __LINE__ );
	}
	
	_mqtt_flags.ackDone = false;
	_mqtt_flags.connected = false;

	if ( mosquitto_lib_init ( ) )
	{
		return ( __LINE__ );
	}

	_mqtt_mosq = mosquitto_new ( name, true, NULL );

	if ( !_mqtt_mosq )
	{
		switch ( errno )
		{
			case ENOMEM:
			{
				fprintf ( stderr, "Error: Out of memory.\n" );
				break;
			}
			case EINVAL:
			{
				fprintf ( stderr, "Error: Invalid id.\n" );
				break;
			}
		}

		mosquitto_lib_cleanup ( );

		return ( __LINE__ );
	}

	setExecAfterAllOnExit ( MQTTAfterExit, NULL );
	setExecBeforeAllOnExit ( MQTTBeforeExit, NULL );

	mosquitto_message_callback_set ( _mqtt_mosq, my_message_callback );
	mosquitto_connect_callback_set ( _mqtt_mosq, my_connect_callback );

	if ( mosquitto_max_inflight_messages_set ( _mqtt_mosq, 20 ) )
	{
		printf ( "%d : %s\n", __LINE__, mosquitto_strerror ( rc ) );
		return ( __LINE__ );
	}

	rc = mosquitto_connect ( _mqtt_mosq, ipAddress, port, 1 );
	if ( rc )
	{
		fprintf ( stderr, "%s\n", mosquitto_strerror ( rc ) );
		return ( __LINE__ );
	}

	_mqtt_flags.connected = true;

	while ( _mqtt_flags.ackDone == false )
	{
		rc = mosquitto_loop ( _mqtt_mosq, -1, 1 );
		if ( rc )
		{
			printf ( "%d : %s\n", __LINE__, mosquitto_strerror ( rc ) );
			return ( 1 );
		}
		else
		{
			usleep ( 50000 );
		}
	}

	logDebug ( "server connected\n" );

	mosquitto_threaded_set ( _mqtt_mosq, true );

	if ( pthread_create ( &threadID, NULL, infiniteLoop, ( void * )_mqtt_mosq ) == -1 )
	{
		perror ( "pthread_create" );
		return ( __LINE__ );
	}
	setThreadJoinOnExit ( threadID );

	return ( 0 );
}

int erase_sub_topic ( const char * const restrict topic )
{
	uint16_t i = 0;

	// shear the topic need to be delet
	while ( ( i < _mqtt_topics.length ) &&
		strcmp ( topic, _mqtt_topics.list[ i ] ) )
	{
		i++;
	}

	// if not present return
	if ( i >= _mqtt_topics.length )
	{
		return ( __LINE__ );
	}

	// free the selected topic
	free ( _mqtt_topics.list[ i ] );

	// move all folloers to one place up
	while ( i < ( _mqtt_topics.length - 1 ) )
	{
		_mqtt_topics.list[ i ] = _mqtt_topics.list[ i + 1 ];
		i++;
	}

	// set last one is NULL
	_mqtt_topics.list[ i ] = NULL;

	return ( 0 );
}

int add_sub_topic ( const char * const restrict topic )
{
	char **ptr_realloc = realloc ( _mqtt_topics.list, sizeof ( char * ) * ( _mqtt_topics.length + 1 ) );

	if ( ptr_realloc != NULL )
	{
		_mqtt_topics.list = ptr_realloc;
		_mqtt_topics.list[ _mqtt_topics.length ] = malloc ( strlen( topic ) );

		if ( !_mqtt_topics.list[ _mqtt_topics.length ] )
		{
			return ( __LINE__ );
		}

		strcpy ( _mqtt_topics.list[ _mqtt_topics.length ], topic );
		mqtt_subscribe ( NULL, _mqtt_topics.list[ _mqtt_topics.length ], 1 );
		_mqtt_topics.length++;
		return ( 0 );
	}
	/* Même si ptr_realloc est nul, on ne vide pas la mémoire. On laisse l'initiative au programmeur. */
	return ( __LINE__ );
}

int mqtt_subscribe ( int * mid, const char * const restrict topic, int qos )
{
	return mosquitto_subscribe ( _mqtt_mosq, mid, topic, qos );
}

int mqtt_publish ( int * mid, const char * const restrict topic, int payloadlen, const void * payload, int qos, bool	retain )
{
	return mosquitto_publish ( _mqtt_mosq, mid, topic, payloadlen, payload, qos, retain );
}
