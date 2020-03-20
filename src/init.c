#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "lib/mcp23017/mcp23017.h"
#include "lib/pca9685/pca9685.h"
#include "lib/log/log.h"
#include "lib/freeOnExit/freeOnExit.h"
#include "lib/dynamixel_sdk/dynamixel_sdk.h"

#include "init.h"


static void dynamixelClose ( void * arg )
{
	closePort ( ( long ) arg );
}

int initMCP23017 ( const char * const i2c, const int addr, int * const fd )
{
	// init gpio expander
	if ( addr )
	{
		int err = 0;
		logVerbose ( "   - mcp23017 : %d\n", addr );
		if ( err = openMCP23017 ( i2c, addr, fd ), err )
		{
			logVerbose ( "      - error : %d\n", err );
			logVerbose ( "         %s\n", strerror ( errno ) );
			return ( __LINE__ );
		}

		if ( err = setCloseOnExit ( *fd ), err )
		{
			logVerbose ( "       - error : %d\n", err );
			logVerbose ( "         %s\n", strerror ( errno ) );
			close ( *fd );
			return ( __LINE__ );
		}

		gpioSetDir ( *fd, 'A', 0, mcp23017_OUTPUT );
		gpioSetDir ( *fd, 'A', 1, mcp23017_OUTPUT );
		gpioSet ( *fd, 'A', 0, 1 );
		gpioSet ( *fd, 'A', 1, 1 );
	}
	return ( 0 );
}

int initPAC9685 ( const char * const i2c, const int addr, int * const fd )
{
	// init pwm expander
	if ( addr )
	{
		int err = 0;
		logVerbose ( "   - pca9685 : %d\n", addr );
		if ( err = openPCA9685 ( i2c, addr, fd ), err )
		{
			logVerbose ( "   - error : %d\n", err );
			logVerbose ( "     %s\n", strerror ( errno ) );
			return ( __LINE__ );
		}

		if ( err = setCloseOnExit ( *fd ), err )
		{
			logVerbose ( "   - error : %d\n", err );
			logVerbose ( "     %s\n", strerror ( errno ) );
			close ( *fd );
			return ( __LINE__ );
		}
	}
	return ( 0 );
}

int initDyna ( const char * const addr, long int * const fd, const uint32_t speed )
{
	// init dynamixel
	*fd = portHandler ( addr );
	packetHandler ( );
	if ( !openPort ( *fd ) )
	{ // can't open port
		logVerbose ( " - dyna : \e[31m%s\e[0m (open failure)\n", addr );
		return ( __LINE__ );
	}
	else
	{
		logVerbose ( " - dyna : %s\n", addr );
		logVerbose ( "   - Device Name : %s\n", addr );

		logVerbose ( "   - Baudrate : %d \n", speed );
		if ( !setBaudRate ( *fd, speed ) )
		{
			logVerbose ( "   - Baudratesetting failed, stay with last value\n" );
		}
		else
		{
			logVerbose ( "   - Baudrate	: %d\n", getBaudRate ( *fd ) );
		}

		setExecAfterAllOnExit ( dynamixelClose, ( void * )*fd );
	}
	return ( 0 );
}