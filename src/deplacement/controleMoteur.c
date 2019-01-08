#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "controleMoteur.h"
#include "../lib/freeOnExit/freeOnExit.h"

#define MAX_SPEED_VALUE ( ( float )32767.0 )

static uint32_t _controlMoteur_delay = 5000000;
static uint32_t _controlMoteur_boostdelay = 0;
static uint8_t _controlMoteur_boostRequested = false;
static struct roboclaw *_controlMoteur_motorBoard = NULL;
static struct timeval _controlMoteur_lastDate;
static struct
{
	float min;
	float current;
	float max;
	float boost;
}
_controlMoteur_voltage = { 0.0, 0.0, 0.0, 0.0 };

static void roboClawClose ( void * arg )
{
	roboclaw_duty_m1m2 ( ( struct roboclaw * ) arg, 0x80, 0, 0 );
	roboclaw_close ( ( struct roboclaw * )arg );
}

void initBoost ( const float voltage, const uint32_t delay )
{
	_controlMoteur_voltage.boost = voltage;
	_controlMoteur_boostdelay = delay;
}

int requestBoost ( bool flag )
{
	static bool last = false;
	_controlMoteur_boostRequested = flag & ( flag ^ last );
	last = flag;
	return ( getBattery ( ) < 0 );
}

float getBattery ( void )
{
	int16_t voltage = 0;


	if ( roboclaw_main_battery_voltage ( _controlMoteur_motorBoard, 0x80, &voltage ) == ROBOCLAW_OK )
	{
		return ( -1.0 );
	}

	gettimeofday ( &_controlMoteur_lastDate, NULL );

	_controlMoteur_voltage.current = ( float )voltage / 10.0f;
	return ( _controlMoteur_voltage.current );
}

int initEngine ( const char * restrict const path,
	const uint32_t speed,
	const float maxVoltage,
	const float minVoltage,
	const uint32_t delay,
	struct roboclaw ** const ptr )
{
	int16_t voltage = 0;

	if ( !path ||
		( maxVoltage < 0 ) ||
		( minVoltage < 0 ) )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	_controlMoteur_voltage.min = minVoltage;
	_controlMoteur_voltage.max = maxVoltage;
	_controlMoteur_delay = delay;

	_controlMoteur_motorBoard = roboclaw_init ( path, speed );
	if ( !_controlMoteur_motorBoard )
	{
		return ( __LINE__ );
	}

	setExecBeforeAllOnExit ( roboClawClose, ( void * )_controlMoteur_motorBoard );

	if ( roboclaw_main_battery_voltage ( _controlMoteur_motorBoard, 0x80, &voltage ) != ROBOCLAW_OK)
	{
		return ( __LINE__ );
	}

	if ( ptr )
	{
		*ptr = _controlMoteur_motorBoard;
	}
	getBattery();

	return ( 0 );
}

int envoiOrdreMoteur ( int16_t left, int16_t right, int16_t limitSpeed )
{
	float coefVoltage = 1.0;

	struct timeval now;
	uint32_t time = 0;
	int16_t voltage = 0;

	// init speed left and right from limits
	if ( abs ( left ) > limitSpeed )
	{
		if ( left > 0 )
		{
			left = limitSpeed;
		}
		else
		{
			left = -limitSpeed;
		}
	}
	if ( abs ( right ) > limitSpeed )
	{
		if ( right > 0 )
		{
			right = limitSpeed;
		}
		else
		{
			right = -limitSpeed;
		}
	}

	// if battery power verification requested
	if ( _controlMoteur_delay > 0 )
	{
		gettimeofday ( &now, NULL );

		time = ( now.tv_sec - _controlMoteur_lastDate.tv_sec ) * 1000000 + ( now.tv_usec - _controlMoteur_lastDate.tv_usec );

		// get battery voltage if delay is past
		if ( time > _controlMoteur_delay )
		{
			_controlMoteur_lastDate = now;
			if ( roboclaw_main_battery_voltage ( _controlMoteur_motorBoard, 0x80, &voltage ) != ROBOCLAW_OK )
			{
				return ( __LINE__ );
			}

			_controlMoteur_voltage.current = ( float )voltage / 10.0f;
			_controlMoteur_boostRequested = false;
		}

		// if battery too low stop motor

		if ( _controlMoteur_voltage.current < _controlMoteur_voltage.min )
		{
			printf ( "Battery too low stop motor\n" );
			roboclaw_duty_m1m2 ( _controlMoteur_motorBoard, 0x80, 0, 0 );
			errno = 1;
			return ( __LINE__ );
		}

		if ( _controlMoteur_boostRequested &&
			( _controlMoteur_voltage.boost > _controlMoteur_voltage.max ) &&
			( time < _controlMoteur_boostdelay ) )
		{
			// if power change coef will did it too to correct this change
		//	coefVoltage = _controlMoteur_voltage.boost / _controlMoteur_voltage.current;
		}
		else
		{
			// if power change coef will did it too to correct this change
		//	coefVoltage = _controlMoteur_voltage.max / _controlMoteur_voltage.current;
			_controlMoteur_boostRequested = false;
		}
	}

	left = ( MAX_SPEED_VALUE * coefVoltage ) * left / 800;
	right = ( MAX_SPEED_VALUE * coefVoltage ) * right / 800;

	if ( roboclaw_duty_m1m2 ( _controlMoteur_motorBoard, 0x80, -1*left, -1*right ) != ROBOCLAW_OK )
	{
		return ( __LINE__ );
	}

	return(  0 );
}
