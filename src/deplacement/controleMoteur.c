#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#include "controleMoteur.h"
#include "../lib/freeOnExit/freeOnExit.h"

#define MAX_SPEED_VALUE ( ( float )32767.0 )

static uint32_t _conrtolMoateur_delay = 5000000;
static uint32_t _conrtolMoateur_boostdelay = 0;
static uint8_t _conrtolMoateur_boostRequested = false;
static struct roboclaw *_conrtolMoateur_motorBoard = NULL;
static struct timeval _conrtolMoateur_lastDate;
static struct
{
	float min;
	float current;
	float max;
	float boost;
}
_conrtolMoateur_voltage = { 0.0, 0.0, 0.0, 0.0 };

static void roboClawClose ( void * arg )
{
	roboclaw_duty_m1m2 ( ( struct roboclaw * ) arg, 0x80, 0, 0 );
	roboclaw_close ( ( struct roboclaw * )arg );
}

void initBoost ( const float voltage, const uint32_t delay )
{
	_conrtolMoateur_voltage.boost = voltage;
	_conrtolMoateur_boostdelay = delay;
}

int requestBoost ( bool flag )
{
	static bool last = false;
	_conrtolMoateur_boostRequested = flag & ( flag ^ last );
	last = flag;
	return ( getBattery ( ) < 0 );
}

float getBattery ( void )
{
	int16_t volatge = 0;


	if ( roboclaw_main_battery_voltage ( _conrtolMoateur_motorBoard, 0x80, &volatge ) == ROBOCLAW_OK )
	{
		return ( -1.0 );
	}
	
	gettimeofday ( &_conrtolMoateur_lastDate, NULL );

	_conrtolMoateur_voltage.current = ( float )volatge / 10.0f;
	return ( _conrtolMoateur_voltage.current );
}

int initEngine ( const char * restrict const path,
	const uint32_t speed,
	const float maxVoltage,
	const float minVolatge,
	const uint32_t delay,
	struct roboclaw ** const ptr )
{
	int16_t volatge = 0;

	if ( !path ||
		( maxVoltage < 0 ) || 
		( minVolatge < 0 ) )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	_conrtolMoateur_voltage.min = minVolatge;
	_conrtolMoateur_voltage.max = maxVoltage;
	_conrtolMoateur_delay = delay;

	_conrtolMoateur_motorBoard = roboclaw_init ( path, speed );
	if ( !_conrtolMoateur_motorBoard )
	{
		return ( __LINE__ );
	}

	setExecAfterAllOnExit ( roboClawClose, ( void * )_conrtolMoateur_motorBoard );

	if ( roboclaw_main_battery_voltage ( _conrtolMoateur_motorBoard, 0x80, &volatge ) != ROBOCLAW_OK)
	{
		return ( __LINE__ );
	}

	_conrtolMoateur_voltage.current = ( float )volatge / 10.0f;

	if ( ptr )
	{
		*ptr = _conrtolMoateur_motorBoard;
	}

	gettimeofday ( &_conrtolMoateur_lastDate, NULL );

	return ( 0 );
}

int envoiOrdreMoteur ( int16_t left, int16_t right, int16_t limitSpeed )
{
	float coefVoltage = 1.0;

	struct timeval now;
	uint32_t time = 0;
	int16_t volatge = 0;

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
	if ( _conrtolMoateur_delay > 0 )
	{
		gettimeofday ( &now, NULL );

		time = ( now.tv_sec - _conrtolMoateur_lastDate.tv_sec ) * 1000000 + ( now.tv_usec - _conrtolMoateur_lastDate.tv_usec );

		// get battery volatge if delay is past
		if ( time > _conrtolMoateur_delay )
		{
			_conrtolMoateur_lastDate = now;
			if ( roboclaw_main_battery_voltage ( _conrtolMoateur_motorBoard, 0x80, &volatge ) != ROBOCLAW_OK )
			{
				return ( __LINE__ );
			}

			_conrtolMoateur_voltage.current = ( float )volatge / 10.0f;
			_conrtolMoateur_boostRequested = false;
		}

		// if battery too low stop motor
		if ( _conrtolMoateur_voltage.current < _conrtolMoateur_voltage.min )
		{
			printf ( "Battery too low stop motor\n" );
			roboclaw_duty_m1m2 ( _conrtolMoateur_motorBoard, 0x80, 0, 0 );
			errno = 1;
			return ( __LINE__ );
		}

		if ( _conrtolMoateur_boostRequested &&
			( _conrtolMoateur_voltage.boost > _conrtolMoateur_voltage.max ) &&
			( time < _conrtolMoateur_boostdelay ) )
		{
			// if power change coef will did it too to correct this change
			coefVoltage = _conrtolMoateur_voltage.boost / _conrtolMoateur_voltage.current;
		}
		else
		{
			// if power change coef will did it too to correct this change
			coefVoltage = _conrtolMoateur_voltage.max / _conrtolMoateur_voltage.current;
			_conrtolMoateur_boostRequested = false;
		}
	}

	left = ( MAX_SPEED_VALUE * coefVoltage ) * left / 1500;
	right = ( MAX_SPEED_VALUE * coefVoltage ) * right / 1500;

	if ( roboclaw_duty_m1m2 ( _conrtolMoateur_motorBoard, 0x80, left, right ) != ROBOCLAW_OK )
	{
		return ( __LINE__ );
	}

	return(  0 );
}
