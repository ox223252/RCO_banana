
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "lib/config/config_arg.h"
#include "lib/config/config_file.h"
#include "lib/freeOnExit/freeOnExit.h"
#include "lib/log/log.h"
#include "lib/signalHandler/signalHandler.h"
#include "lib/termRequest/request.h"
#include "lib/timer/timer.h"
#include "lib/roboclaw/roboclaw.h"

#include "lib/dynamixel_sdk/dynamixel_sdk.h"

#include "lib/parserXML/loadXML.h"
// #include "deplacement/odometrie.h"
#include "struct/structRobot.h"
#include "struct/structAction.h"


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

const uint8_t speedStep = 10;

void dynamixelClose ( void * arg )
{
	closePort ( ( long ) arg );
}

void roboClawClose ( void * arg )
{
	roboclaw_close ( ( struct roboclaw * )arg );
}

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
	uint16_t i = 0; // loop counter / loop flag / or temp var
	void * tmp = NULL;

	char dynamixelsPath[ 128 ] = { 0 }; // dynamixel acces point /dev/dyna
	uint32_t dynamixelUartSpeed = 115200; // uart speed
	long int dynaPortNum = 0;
	char motorBoadPath[ 128 ] = { 0 }; // roboclaw access point /dev/roboclaw
	
	uint32_t motorBoardUartSpeed = 115200; // uart speed
	struct roboclaw *motorBoard = NULL;
	uint8_t address = 0x80;
	int16_t maxSpeed = 32767; // motor max speed, ti neved should cross this limit
	uint32_t globalTime = 0; // global game time
	char xmlInitPath[ 128 ] = { 0 };
	char xmlActionPath[ 128 ] = { 0 };

	uint8_t pca9685 = 0; // servo driver handler (i2c)

	Action* tabActionTotal = NULL;
	int nbAction = 0;

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
		uint8_t green:1,    // 0x01
			red:1,          // 0x02
			un2:1,          // 0x04
			help:1,         // 0x08
			quiet:1,        // 0x10
			debug:1,        // 0x20
			color:1,        // 0x40
			noDrive:1;      // 0x80
		uint8_t noArm:1;	// 0x01
	}
	flag = { 0 };

	param_el paramList[] = 
	{
		{ "--help", "-h",     0x08, cT ( bool ), ((uint8_t * )&flag), "this window" },
		{ "--green", "-g",    0x01, cT ( bool ), ((uint8_t * )&flag), "launch the green prog" },
		{ "--red", "-r",      0x02, cT ( bool ), ((uint8_t * )&flag), "launch the red prog" },
		{ "--q", "-q",        0x10, cT ( bool ), ((uint8_t * )&flag), "hide all trace point" },
		{ "--debug", "-d",    0x20, cT ( bool ), ((uint8_t * )&flag), "display many trace point" },
		{ "--color", "-c",    0x40, cT ( bool ), ((uint8_t * )&flag), "add color to debug traces" },
		{ "--noDrive", "-nD", 0x80, cT ( bool ), ((uint8_t * )&flag), "use it to disable drive power" },
		{ "--noArm", "-nA",   0x01, cT ( bool ), ((uint8_t * )&flag) + 1, "use it to disable servo motor" },
		{ "--MaxSpeed", "-Ms", 1, cT ( int16_t ), &maxSpeed, "set max speed [ 0 ; 32767 ]" },
		{ "--ini", "-i", 1, cT ( str ), xmlInitPath, "xml initialisation file path" },
		{ "--xml", "-x", 1, cT ( str ), xmlActionPath, "xml action file path" },
		{ "--time", "-t", 1, cT ( uint32_t ), &globalTime, "game duration in seconds" },
		{ NULL, NULL, 0, 0, NULL, NULL }
	};


	config_el configList[] =
	{
		{ "PATH_DYNA", cT ( str ), dynamixelsPath, "PATH to access to dynamixels"},
		{ "PATH_MOTOR_BOARD", cT ( str ), motorBoadPath, "PATH to access to dynamixels"},
		{ "PATH_MOTOR_BOARD_UART_SPEED", cT ( uint32_t ), &motorBoardUartSpeed, "UART speed for robocloaw board" },
		{ "PCA9695_ADDR", cT ( uint8_t ), &pca9685, "pca9685 board i2c addr"},
		{ "XML_ACTION_PATH", cT ( str ), xmlInitPath, "xml initialisation file path"},
		{ "XML_INIT_PATH", cT ( str ), xmlActionPath, "xml action file path"},
		{ "GLOBAL_TIME", cT ( uint32_t ), &globalTime, "game duration in seconds"},
		{ NULL, 0, NULL, NULL }
	};

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

	// init memory free on exit
	if ( initFreeOnExit ( ) )
	{
		logVerbose ( "init free_on_exit's subroutine failed\n" );
		return ( __LINE__ );
	}

	tmp = NULL;
	if ( getTermSatatus ( &tmp ) )
	{
		printf ( "terminal init failed\n" );
		return ( __LINE__ );
	}
	else if ( setExecAfterAllOnExit ( (void(*)(void*))setTermSatatus, tmp ) )
	{
		printf ( "terminal init failed\n" );
		return ( __LINE__ );
	}


	if ( readParamArgs ( argc, argv, paramList ) ||
		readConfigFile ( "res/config.rco", configList ) )
	{
		return ( __LINE__ );
	}

	if ( flag.help )
	{
		printf ( "build date: %s\n\n", DATE_BUILD );
		helpParamArgs ( paramList );
		return ( 0 );
	}

	logSetQuiet ( flag.quiet );
	logSetColor ( flag.color );
	logSetDebug ( flag.debug );


	if ( !flag.noDrive )
	{ // if engine wasn't disabled
		// init motor
		motorBoard = roboclaw_init ( motorBoadPath, motorBoardUartSpeed);
		if ( !motorBoard )
		{
			logVerbose ( "can't open robo claw bus at %s\n", motorBoadPath );
			logVerbose ( "%s\n", strerror ( errno ) );
			return ( __LINE__ );
		}
		setExecAfterAllOnExit ( roboClawClose, ( void * )motorBoard );

		if ( !roboclaw_main_battery_voltage ( motorBoard, address, ( int16_t * )&i ) )
		{
			logVerbose ( "error on reading battery voltage\n" );
			return ( __LINE__ );
		}
		else
		{
			printf("battery voltage is : %f V\n", (float)i/10.0f);
		}
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
			{ // manual drive of robot zqsd / wasd / UP/LEFT/DOWN/RIGHT
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
							if ( motorBoard )
							{
								roboclaw_duty_m1m2 ( motorBoard, address, 
									moteur.left, 
									moteur.right );
							}

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
										moteur.left += speedStep;
									}
									if ( moteur.right < maxSpeed )
									{
										moteur.right += speedStep;
									}
									break;
								}
								case KEYCODE_LEFT:
								{
									if ( moteur.left > -maxSpeed )
									{
										moteur.left -= speedStep;
									}
									if ( moteur.right < maxSpeed )
									{
										moteur.right += speedStep;
									}
									break;
								}
								case KEYCODE_DOWN:
								{
									if ( moteur.left > -maxSpeed )
									{
										moteur.left -= speedStep;
									}
									if ( moteur.right > -maxSpeed )
									{
										moteur.right -= speedStep;
									}
									break;
								}
								case KEYCODE_RIGHT:
								{
									if ( moteur.left < maxSpeed )
									{
										moteur.left += speedStep;
									}
									if ( moteur.right > -maxSpeed )
									{
										moteur.right -= speedStep;
									}
									break;
								}
								case KEYCODE_SPACE:
								{
									moteur.left = 0;
									moteur.right = 0;
									break;
								}
								default:
								{
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

	printf ( "run %s\n", ( flag.red )? "\e[1;31mred\e[0m" : "\e[1;32mgreen\e[0m" );

	if ( !flag.noArm )
	{ // arm enabled
		
		dynaPortNum = portHandler ( dynamixelsPath );
		if ( !openPort ( dynaPortNum ) )
		{ // can't open port
			flag.noArm = 0;
			logVerbose ( " - dyna : \e[31m%s\e[0m (open failure)\n", dynamixelsPath );
		}
		else
		{
			logVerbose ( " - dyna : %s\n", dynamixelsPath );
			logVerbose ( "   - Device Name : %s\n", dynamixelsPath );

			packetHandler ( );

			if ( setBaudRate ( dynaPortNum, dynamixelUartSpeed ) )
			{
				logVerbose ( "   - Baudratesetting failed, stay with last value\n" );
			}
			logVerbose ( "   - Baudrate    : %d\n", getBaudRate ( dynaPortNum ) );

			setExecAfterAllOnExit ( dynamixelClose, ( void * )dynaPortNum );
		}

		logVerbose ( " - pca9685 : %d\n", pca9685 );
	}
	else
	{ // arm disabled
		logVerbose ( " - dyna : \e[31m%s\e[0m\n", dynamixelsPath );
		logVerbose ( " - pca9685 : \e[31m%d\e[0m\n", pca9685 );
	}

	// only for display
	if ( !flag.noDrive )
	{ // engine enabled
		logVerbose ( " - robotclaw : %s\n", motorBoadPath );
	}
	else
	{ // engne disabled
		logVerbose ( " - robotclaw : \e[31m%s\e[0m\n", motorBoadPath );
	}

	tabActionTotal = ouvrirXML ( &nbAction, xmlInitPath );
	if ( !tabActionTotal )
	{
		logVerbose ( "xml loading failed: -%s-\n %s\n", xmlInitPath, strerror ( errno ) );
		return ( __LINE__ );
	}
	setFreeOnExit ( tabActionTotal );


	while ( 1 )
	{ // initialisation

		if ( !flag.noArm )
		{ // dynamixel
			// write1ByteTxRx(port_num, PROTOCOL_VERSION2, DXL2_ID, ADDR_PRO_TORQUE_ENABLE, TORQUE_ENABLE);
			// if ((dxl_comm_result = getLastTxRxResult(port_num, PROTOCOL_VERSION2)) != COMM_SUCCESS)
			// {
			// 	printf("%s\n", getTxRxResult(PROTOCOL_VERSION2, dxl_comm_result));
			// }
			// else if ((dxl_error = getLastRxPacketError(port_num, PROTOCOL_VERSION2)) != 0)
			// {
			// 	printf("%s\n", getRxPacketError(PROTOCOL_VERSION2, dxl_error));
			// }
			// else
			// {
			// 	printf("Dynamixel#%d has been successfully connected \n", DXL2_ID);
			// }
		}

		if ( !flag.noDrive )
		{ // engine roboclaw

		}
	}
	free ( tabActionTotal );
	unsetFreeOnExit ( tabActionTotal );
	tabActionTotal = NULL;


	tabActionTotal = ouvrirXML ( &nbAction, xmlInitPath );
	if ( !tabActionTotal )
	{
		logVerbose ( "xml loading failed: -%s-\n %s\n", xmlInitPath, strerror ( errno ) );
		return ( __LINE__ );
	}
	setFreeOnExit ( tabActionTotal );


	timer ( globalTime * 1000000, proccessNormalEnd, "stop request by timer", true );

	i = 0;
	printf ( "\e[2K\r%6d\n", i );

	while ( 1 )
	{
		sleep ( 1 );
		printf ( "\e[A\e[2K\r%6d\n", ++i );
	}

	return ( 0 );
}