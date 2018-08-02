
#include <stdio.h>
#include <stdlib.h>

#include "lib/config/config_arg.h"
#include "lib/config/config_file.h"
#include "lib/freeOnExit/freeOnExit.h"
#include "lib/log/log.h"
#include "lib/signalHandler/signalHandler.h"
#include "lib/termRequest/request.h"
#include "lib/timer/timer.h"

enum
{
	MENU_red,
	MENU_green,
	MENU_manual,
	MENU_exit
};

enum
{
	MENU_MANUAL_controller,
	MENU_MANUAL_keyboard,
	MENU_MANUAL_exit
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
	void * tmp = NULL;

	char dynamixelsPath[ 64 ] = { 0 };
	char motorBoadPath[ 64 ] = { 0 };
	uint8_t pca9685 = 0;
	int8_t maxSpeed = 127;

	struct
	{
		int8_t left;
		int8_t right;
	}
	moteur = { 0 };

	char *menuItems[] = {
		"run \e[1;31mRED\e[0m",
		"run \e[1;32mGREEN\e[0m",
		"manual mode",
		"exit",
		NULL
	};

	char *manualMenuItems[] = {
		"controller",
		"keyboard",
		"exit",
		NULL
	};

	struct
	{
		uint8_t green:1,// 0x01
			red:1,      // 0x02
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
		{ "--green", "-g", 0x01, cT ( bool ), &flag, "launch the green prog" },
		{ "--red", "-r",   0x02, cT ( bool ), &flag, "launch the red prog" },
		{ "--q", "-q",     0x10, cT ( bool ), &flag, "hide all trace point" },
		{ "--debug", "-d", 0x20, cT ( bool ), &flag, "display many trace point" },
		{ "--color", "-c", 0x40, cT ( bool ), &flag, "add color to debug traces" },
		{ "--MaxSpeed", "-Ms", 1, cT ( int8_t ), &maxSpeed, "set max speed [ 0 ; 127 ]" },
		{ NULL, NULL, 0, 0, NULL, NULL }
	};

	config_el configList[] =
	{
		{ "PATH_DYNA", cT ( str ), dynamixelsPath, "PATH to access to dynamixels"},
		{ "PATH_MOTOR_BOARD", cT ( str ), motorBoadPath, "PATH to access to dynamixels"},
		{ "PCA9695_ADDR", cT ( uint8_t ), &pca9685, "pca9685 board i2c addr"},
		{ NULL, 0, NULL, NULL }
	};

	if ( maxSpeed > 127 )
	{
		maxSpeed = 127;
	}
	else if ( maxSpeed < 0 )
	{
		maxSpeed = 127;
	}

	if ( readParamArgs ( argc, argv, paramList ) ||
		readConfigFile ( "res/config.rco", configList ) )
	{
		return ( __LINE__ );
	}
	
	while ( !( flag.red ^ flag.green ) )
	{ // if no color or both colors set
		switch ( menu ( 0, menuItems, NULL ) )
		{
			case MENU_red:
			{
				flag.red = 1;
				flag.green = 0;
				break;
			}
			case MENU_green:
			{
				flag.green = 1;
				flag.red = 0;
				break;
			}
			case MENU_manual:
			{
				switch ( menu ( 0, manualMenuItems, "  >", "   ", NULL ) )
				{
					case MENU_MANUAL_controller:
					{
						break;
					}
					case MENU_MANUAL_keyboard:
					{
						setBlockMode ( &tmp, true );

						i = 1;
						do
						{
							printf ( "%4d %4d\r", moteur.left, moteur.right );

							switch ( getMovePad ( false ) )
							{
								case 	KEYCODE_ESCAPE:
								{
									moteur.left = 0;
									moteur.right = 0;
									i = 0;
									break;
								}
								case KEYCODE_UP:
								{
									if ( moteur.left < maxSpeed )
									{
										moteur.left++;
									}
									if ( moteur.right < maxSpeed )
									{
										moteur.right++;
									}
									break;
								}
								case KEYCODE_LEFT:
								{
									if ( moteur.left > -maxSpeed )
									{
										moteur.left--;
									}
									if ( moteur.right < maxSpeed )
									{
										moteur.right++;
									}
									break;
								}
								case KEYCODE_DOWN:
								{
									if ( moteur.left > -maxSpeed )
									{
										moteur.left--;
									}
									if ( moteur.right > -maxSpeed )
									{
										moteur.right--;
									}
									break;
								}
								case KEYCODE_RIGHT:
								{
									if ( moteur.left < maxSpeed )
									{
										moteur.left++;
									}
									if ( moteur.right > -maxSpeed )
									{
										moteur.right--;
									}
									break;
								}
								case KEYCODE_SPACE:
								{
									moteur.left = 0;
									moteur.right = 0;
									break;
								}
							}

							// drive motor
						}
						while ( i );
						
						resetBlockMode ( tmp );
						
						break;
					}
					default:
					case MENU_MANUAL_exit:
					{
						break;
					}
				}
				break;
			}
			case MENU_exit:
			default:
			{
				return ( __LINE__ );
			}
		}
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

	printf ( "run %s\n", ( flag.red )? "\e[1;31mred\e[0m" : "\e[1;32mgreen\e[0m" );
	logVerbose ( " - dyna : %s\n", dynamixelsPath );
	logVerbose ( " - robot clow : %s\n", motorBoadPath );
	logVerbose ( " - pca9685 : %d\n", pca9685 );

	i = 0;
	printf ( "\e[2K\r%6d\n", i );

	timer ( 9000000, proccessNormalEnd, "stop request by timer", true );

	while ( 1 )
	{
		sleep ( 1 );
		printf ( "\e[A\e[2K\r%6d\n", ++i );
	}

	return ( 0 );
}