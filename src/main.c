
#include <stdio.h>
#include <stdlib.h>

#include "lib/config/config_arg.h"
#include "lib/freeOnExit/freeOnExit.h"
#include "lib/log/log.h"
#include "lib/signalHandler/signalHandler.h"
#include "lib/termRequest/request.h"

enum
{
	MENU_red,
	MENU_green,
	MENU_exit
};

void proccessNormalEnd ( void * arg )
{
	if ( arg )
	{
		printf ( "\e[2K\r\e[1;33m%s\e[0m\n", ( char * )arg );
	}
	exit ( 0 );
}

int main ( int argc, char * argv[] )
{
	int i = 0;
	char *menuItems[] = {
		"run \e[1;31mRED\e[0m",
		"run \e[1;32mGREEN\e[0m",
		"continue",
		"exit",
		NULL
	};

	struct
	{
		uint8_t un0:1,  // 0x01
			strat:1,    // 0x02
			un2:1,      // 0x04
			help:1,     // 0x08
			quiet:1,    // 0x10
			debug:1,    // 0x20
			color:1;    // 0x40
	}
	flag = { 0 };

	param_el paramList[] = 
	{
		{ "--help", "-h",  0x08, cT ( bool ), &flag, "this window" },
		{ "--red", "-r",   0x02, cT ( bool ), &flag, "launch the red prog" },
		{ "--green", "-g", 0x00, cT ( bool ), &flag, "launch the green prog" },
		{ "--q", "-q",     0x10, cT ( bool ), &flag, "hide all trace point" },
		{ "--debug", "-d", 0x20, cT ( bool ), &flag, "display many trace point" },
		{ "--color", "-c", 0x40, cT ( bool ), &flag, "add color to debug traces" },
		{ NULL, NULL, 0, 0, NULL, NULL }
	};

	if ( argc < 2 )
	{ // if no cmd line parameter
		switch ( menu ( 0, menuItems, NULL ) )
		{
			case MENU_red:
			{
				flag.strat = 1;
				break;
			}
			case MENU_green:
			{
				flag.strat = 0;
				break;
			}
			case MENU_exit :
			default:
			{
				return ( __LINE__ );
			}
		}
	}
	else if ( readParamArgs ( argc, argv, paramList ) )
	{
		return ( __LINE__ );
	}

	if ( flag.help )
	{
		printf ( "build date: %s\n\n", DATE_BUILD );
		helpParamArgs ( paramList );
		return ( 0 );
	}

	// manage ending signals
	signalHandling signal = {
		.flag = {
			.Int = 1, //ctrl+C
			.Term = 1, // kill
		},
		.Int = {
			.func = proccessNormalEnd,
			.arg = "Ctrl+C requested"
		},
		.Term = {
			.func = proccessNormalEnd,
			.arg = "Kill requested"
		}
	};

	signalHandlerInit ( &signal );

	logSetQuiet ( flag.quiet );
	logSetColor ( flag.color );
	logSetDebug ( flag.debug );

	// init memory free on exit
	if ( initFreeOnExit ( ) )
	{
		logVerbose ( "init free_on_exit's subroutine failed\n" );
		return ( __LINE__ );
	}

	printf ( "run %s\n", ( flag.strat )? "red" : "green" );
	logVerbose ( "bon bah c'est fini quoi\n" );

	i = 90;
	printf ( "\e[2K\r%6d\n\e[A", i );
	while ( 1 )
	{
		sleep ( 1 );
		printf ( "\e[2K\r%6d\n\e[A", --i );
	}

	return ( 0 );
}