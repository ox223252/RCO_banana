#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "lib/config/config_arg.h"
#include "lib/config/config_file.h"
#include "lib/freeOnExit/freeOnExit.h"
#include "lib/log/log.h"
#include "lib/signalHandler/signalHandler.h"
#include "lib/termRequest/request.h"
#include "lib/timer/timer.h"
#include "lib/roboclaw/roboclaw.h"
#include "lib/parserXML/loadXML.h"
#include "lib/dynamixel_sdk/dynamixel_sdk.h"
#include "lib/Xbox360-wireless/cXbox360.h"
#include "lib/GPIO/gpio.h"
#include "lib/mcp23017/mcp23017.h"
#include "lib/sharedMem/sharedMem.h"
#include "lib/pca9685/pca9685.h"

#include "struct/structRobot.h"
#include "struct/structAction.h"
#include "struct/structDetection.h"

#include "gestionAction/management.h"
#include "gestionAction/action.h"
#include "deplacement/odometrie.h"
#include "deplacement/controleMoteur.h"
#include "deplacement/asservissementVitesse.h"
#include "deplacement/detectionBlocage.h"

static Robot robot1 = { 0 };

enum
{
	MENU_purple,
	MENU_yellow,
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
	void * tmp = NULL;

	struct timeval start;

	char dynamixelsPath[ 128 ] = { 0 }; // dynamixel acces point /dev/dyna
	uint32_t dynamixelUartSpeed = 1000000; // uart speed
	long int dynaPortNum = 0;
	char motorBoadPath[ 128 ] = { 0 }; // roboclaw access point /dev/roboclaw
	struct roboclaw *motorBoard = NULL;

	uint32_t motorBoardUartSpeed = 115200; // uart speed

	int joystick = 0;
	Xbox360Controller pad = { 0 };

	int16_t maxSpeed = 32767; // motor max speed, ti neved should cross this limit
	char xmlActionPath[ 128 ] = { 0 };

	robot1.memKey = 123456;

	// battery management
	uint32_t readDelay = 5000000;
	float Vmax = 12.0;
	float Vmin = 10.0;
	float Vboost = 14.0;
	uint32_t tBoost = 1000000;

	//Speed PID management
	float speedAsservPG = 1.;
	float speedAsservIG = 0.;
	float speedAsservDG = 0.;
	float speedAsservPD = 1.;
	float speedAsservID = 0.;
	float speedAsservDD = 0.;

	char i2cPortName[ 64 ] = "/dev/i2c-1";

	uint8_t pca9685Addr = 0; // servo driver addr (i2c)
	int pca9685Fd = 0;

	uint8_t mcp23017Addr = 0; // gpio direr addr
	int mcp23017Fd = 0;

	Action* tabActionTotal = NULL;
	int nbAction = 0;

	struct
	{
		int8_t left;
		int8_t right;
	}
	moteur = { 0 };

	char *menuItems[] = {
		"run \e[1;33mYELLOW\e[0m",
		"run \e[1;35mPURPLE\e[0m",
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
		uint8_t yellow:1,    // &flag + 0 : 0x01
			purple:1,          //             0x02
			un2:1,          //             0x04
			help:1,         //             0x08
			quiet:1,        //             0x10
			debug:1,        //             0x20
			color:1,        //             0x40
			logTerm:1;      //             0x80
		uint8_t logFile:1,  // &flag + 1 : 0x01
			verbose:1;      //             0x02
		}
		flag = { 0 };

		ActionFlag flagAction = { 0 };

		param_el paramList[] =
		{
			{ "--help", 	"-h",	0x08, 	cT ( bool ), ((uint8_t * )&flag), "this window" },
			{ "--yellow", 	"-Y",	0x01, 	cT ( bool ), ((uint8_t * )&flag), "launch the yellow prog" },
			{ "--purple", 	"-P",	0x02, 	cT ( bool ), ((uint8_t * )&flag), "launch the purple prog" },
			{ "--quiet", 	"-q",	0x10, 	cT ( bool ), ((uint8_t * )&flag), "hide all trace point" },
			{ "--verbose", 	"-v",	0x02, 	cT ( bool ), ((uint8_t * )&flag + 1), "set verbose mode" },
			{ "--debug", 	"-d",	0x20, 	cT ( bool ), ((uint8_t * )&flag), "display many trace point" },
			{ "--color", 	"-c",	0x40, 	cT ( bool ), ((uint8_t * )&flag), "add color to debug traces" },
			{ "--term", 	"-lT",	0x80, 	cT ( bool ), ((uint8_t * )&flag), "add color to debug traces" },
			{ "--file", 	"-lF",	0x01, 	cT ( bool ), ((uint8_t * )&flag + 1), "add color to debug traces" },
			{ "--noArm", 	"-nA",	0x01, 	cT ( bool ), ((uint8_t * )&flagAction), "use it to disable servo motor" },
			{ "--armWait", 	NULL,	0x02, 	cT ( bool ), ((uint8_t * )&flagAction), "wait end of timeout before set action to done" },
			{ "--armScan", 	NULL,	0x04, 	cT ( bool ), ((uint8_t * )&flagAction), "wait a key pressed to action to done" },
			{ "--armDone", 	NULL,	0x08, 	cT ( bool ), ((uint8_t * )&flagAction), "automaticaly set action to done (default)" },
			{ "--noDrive", 	"-nD",	0x10, 	cT ( bool ), ((uint8_t * )&flagAction), "use it to disable drive power" },
			{ "--driveWait", NULL,	0x20, 	cT ( bool ), ((uint8_t * )&flagAction), "wait end of timeout before set action to done" },
			{ "--driveScan", NULL,	0x40, 	cT ( bool ), ((uint8_t * )&flagAction), "wait a key pressed to action to done" },
			{ "--driveDone", NULL,	0x80, 	cT ( bool ), ((uint8_t * )&flagAction), "automaticaly set action to done (default)" },
			{ "--MaxSpeed", "-Ms",	1, 		cT ( int16_t ), &maxSpeed, "set max speed [ 1 ; 32767 ]" },
			{ "--xml", 		"-x",	1, 		cT ( str ), xmlActionPath, "xml action file path" },
			{ "--linear_left", "-ll", 1, 	cT ( float ), &robot1.coeffLongG, "linear coef for left wheel" },
			{ "--linear_right", "-lr", 1, 	cT ( float ), &robot1.coeffLongD, "linear coef for right wheel" },
			{ "--angle_left", "-al",1,  	cT ( float ), &robot1.coeffAngleG, "angular coef for right wheel" },
			{ "--angle_right", "-ar", 1, 	cT ( float ), &robot1.coeffAngleD, "angular coef for right wheel" },
			{ "--Vmax", 	"-vM",	1, 		cT ( float ), &Vmax, "maximum voltage that should provide systeme to engine" },
			{ "--Vmin", 	"-vm",	1, 		cT ( float ), &Vmin, "minimum voltage that should provide systeme to engine" },
			{ "--Vboost", 	"-vB",	1, 		cT ( float ), &Vboost, "maximum voltage that should provide systeme to engine during boost mode" },
			{ "--tBoost", 	"-tB",	1, 		cT ( uint32_t ), &tBoost, "maximum delay for boost mode" },
			{ "--i2cPortName", "-iN", 1, 	cT ( str ), i2cPortName, "i2c port name" },
			{ "--pcaAddr", 	"-p",	1, 		cT ( uint8_t ), &pca9685Addr, "pca9685 board i2c addr"},
			{ "--mcpAddr",	"-m",	1, 		cT ( uint8_t ), &mcp23017Addr, "mcp23017 board i2c addr"},
			{ "--memKey",	"-k", 	1, 		cT ( uint32_t ), &(robot1.memKey), "shared memory key" },
			{ NULL, NULL, 0, 0, NULL, NULL }
		};

		config_el configList[] =
		{
			{ "PATH_DYNA", cT ( str ), dynamixelsPath, "PATH to access to dynamixels"},
			{ "PATH_MOTOR_BOARD", cT ( str ), motorBoadPath, "PATH to access to dynamixels"},
			{ "PATH_MOTOR_BOARD_UART_SPEED", cT ( uint32_t ), &motorBoardUartSpeed, "UART speed for robocloaw board" },
			{ "XML_ACTION_PATH", cT ( str ), xmlActionPath, "xml action file path"},
			{ "COEF_LINEAR_LEFT", cT ( float ), &robot1.coeffLongG, "linear coef for left wheel" },
			{ "COEF_LINEAR_RIGHT", cT ( float ), &robot1.coeffLongD, "linear coef for right wheel" },
			{ "COEF_ANGLE_LEFT", cT ( float ), &robot1.coeffAngleG, "angular coef for right wheel" },
			{ "COEF_ANGLE_RIGHT", cT ( float ), &robot1.coeffAngleD, "angular coef for right wheel" },
			{ "BATTERY_DELAY", cT ( uint32_t ), &readDelay, "delay min between two read of battery delay during engin control" },
			{ "VOLATGE_MAX", cT ( float ), &Vmax, "maximum voltage that should provide systeme too engine" },
			{ "VOLATGE_MIN", cT ( float ), &Vmin, "minimum voltage that should provide systeme too engine" },
			{ "BOOST_VOLTAGE", cT ( float ), &Vboost, "maximum voltage that should provide systeme to engine during boost mode" },
			{ "BOOST_TIME", cT ( uint32_t ), &tBoost, "maximum delay for boost mode" },
			{ "COEFF_PG_VITESSE", cT ( float ), &speedAsservPG, "G proportionnel coefficient for speed asservissment" },
			{ "COEFF_IG_VITESSE", cT ( float ), &speedAsservIG, "G Integral coefficient for speed asservissment" },
			{ "COEFF_DG_VITESSE", cT ( float ), &speedAsservDG, "G derivative coefficient for speed asservissment" },
			{ "COEFF_PD_VITESSE", cT ( float ), &speedAsservPD, "D proportionnel coefficient for speed asservissment" },
			{ "COEFF_ID_VITESSE", cT ( float ), &speedAsservID, "D Integral coefficient for speed asservissment" },
			{ "COEFF_DD_VITESSE", cT ( float ), &speedAsservDD, "D derivative coefficient for speed asservissment" },
			{ "I2C_PORT_NAME", cT ( str ), i2cPortName, "i2c port name" },
			{ "PCA9685_ADDR", cT ( uint8_t ), &pca9685Addr, "pca9685 board i2c addr"},
			{ "MCP23017_ADDR", cT ( uint8_t ), &mcp23017Addr, "mcp23017 board i2c addr"},
			{ "SHARED_MEM_KEY",	cT ( uint32_t ), &(robot1.memKey), "shared memory key" },
			{ NULL, 0, NULL, NULL }
		};

	// manage ending signals
		signalHandling signal =
		{
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
	if ( getTermStatus ( &tmp ) )
	{
		printf ( "terminal init failed\n" );
		return ( __LINE__ );
	}
	else if ( setExecBeforeAllOnExit ( (void(*)(void*))setTermStatus, tmp ) )
	{
		setTermStatus ( tmp );
		printf ( "terminal init failed\n" );
		return ( __LINE__ );
	}
	else if ( setFreeOnExit ( tmp ) )
	{
		free ( tmp );
		printf ( "terminal init failed\n" );
		return ( __LINE__ );
	}

	if ( readConfigFile ( "res/config.rco", configList ) ||
		readConfigArgs ( argc, argv, configList ) ||
		readParamArgs ( argc, argv, paramList ) )
	{
		printf ( "no config file\n" );
		return ( __LINE__ );
	}
	else
	{
		if ( maxSpeed < 0 )
		{
			maxSpeed = -maxSpeed;
		}
		else if ( maxSpeed == 0 )
		{
			maxSpeed = 1;
		}

		logSetQuiet ( flag.quiet );
		logSetColor ( flag.color );
		logSetDebug ( flag.debug );
		logSetVerbose ( flag.verbose );

		logDebug ( "log File %s\n", flag.logFile ? "true" : "false" );
		if ( flag.logFile )
		{
			flag.logFile = ( logSetFileName ( "log.txt" ) == 0 );
		}

		logSetOutput ( ( !flag.logFile ) ? 1 : flag.logTerm, flag.logFile );
	}

	if ( flag.help )
	{
		printf ( "build date: %s\n", DATE_BUILD );
		printf ( "\n\e[4mparameter available for cmd line:\e[0m\n" );
		helpParamArgs ( paramList );
		printf ( "\n\e[4mparameter available in res/config.rco file:\e[0m\n" );
		helpConfigArgs ( configList );
		return ( 0 );
	}

	if ( !flagAction.noDrive )
	{ // if engine wasn't disabled
robot1.blocageVoulu = false;
initDetectionBlocage();
		// init motor
if ( initEngine ( motorBoadPath, motorBoardUartSpeed, Vmax, Vmin, readDelay, &motorBoard ) )
{
	logVerbose ( "can't open robo claw bus at %s\n", motorBoadPath );
	logVerbose ( "%s\n", strerror ( errno ) );
	return ( __LINE__ );
}

initBoost ( Vboost, tBoost );

initOdometrie ( motorBoard, &robot1 );

initAsservissementVitesse ( speedAsservPG, speedAsservIG, speedAsservDG, maxSpeed, speedAsservPD, speedAsservID, speedAsservDD );
}

while ( !( flag.yellow ^ flag.purple ) )
	{ // if no color or both colors set
		switch ( menu ( 0, menuItems, NULL ) )
		{
			case MENU_purple:
			{
				flag.purple = 1;
				flag.yellow = 0;
				break;
			}
			case MENU_yellow:
			{
				flag.yellow = 1;
				flag.purple = 0;
				break;
			}
			case MENU_manual:
			{ // manual drive of robot zqsd / wasd / UP/LEFT/DOWN/RIGHT
				switch ( menu ( 0, manualMenuItems, "  >", "   ", NULL ) )
				{
					case MENU_MANUAL_controller:
					{
						joystick = open ( "/dev/input/js0", O_RDONLY | O_NONBLOCK );

						if ( joystick < 0 )
						{
							logVerbose ( "%s\n", strerror ( errno ) );
							break;
						}

						getStatus360 ( joystick, &pad, true );
						do
						{
							if ( pad.back )
							{
								break;
							}

							envoiOrdreMoteur ( pad.Y1 >> 4, pad.Y2 >> 2, maxSpeed );
						}
						while ( getStatus360 ( joystick, &pad, false ) );

						close ( joystick );
						joystick = -1;
						break;
					}
					case MENU_MANUAL_keyboard:
					{
						uint8_t continusFlag = 1;
						do
						{
							printf ( "%4d %4d\r", moteur.left, moteur.right );
							if ( motorBoard )
							{
								envoiOrdreMoteur ( moteur.left * 30, moteur.right * 30, maxSpeed );
							}

							switch ( getMovePad ( true ) )
							{
								case 	KEYCODE_ESCAPE:
								{
									moteur.left = 0;
									moteur.right = 0;
									continusFlag = 0;
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
										moteur.left -= speedStep / 2;
									}
									if ( moteur.right < maxSpeed )
									{
										moteur.right += speedStep / 2;
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
										moteur.left += speedStep / 2;
									}
									if ( moteur.right > -maxSpeed )
									{
										moteur.right -= speedStep / 2;
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
						}
						while ( continusFlag );

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

	printf ( "run %s\n", ( flag.yellow )? "\e[1;33myellow\e[0m" : "\e[1;35mpurple\e[0m" );

	if ( !flagAction.noArm )
	{ // arm enabled
		dynaPortNum = portHandler ( dynamixelsPath );
		packetHandler ( );
		if ( !openPort ( dynaPortNum ) )
		{ // can't open port
	flagAction.noArm = 0;
	logVerbose ( " - dyna : \e[31m%s\e[0m (open failure)\n", dynamixelsPath );
}
else
{
	logVerbose ( " - dyna : %s\n", dynamixelsPath );
	logVerbose ( "   - Device Name : %s\n", dynamixelsPath );

	logVerbose ( "   - Baudrate : %d \n", dynamixelUartSpeed );
	setPortNum ( dynaPortNum );
	if ( !setBaudRate ( dynaPortNum, dynamixelUartSpeed ) )
	{
		logVerbose ( "   - Baudratesetting failed, stay with last value\n" );
	}
	else
	{
		logVerbose ( "   - Baudrate	: %d\n", getBaudRate ( dynaPortNum ) );
	}

	for ( uint8_t i = 40; i < 44; i++ )
	{
		write1ByteTxRx ( dynaPortNum, PROTOCOL_VERSION, i, ADDR_MX_TORQUE_ENABLE, 1 );
	}
	for ( uint8_t i = 50; i < 55; i++ )
	{
		write1ByteTxRx ( dynaPortNum, PROTOCOL_VERSION, i, ADDR_MX_TORQUE_ENABLE, 1 );
	}
	setExecAfterAllOnExit ( dynamixelClose, ( void * )dynaPortNum );
}

if ( mcp23017Addr )
{
	logVerbose ( "   - mcp23017 : %d\n", mcp23017Addr );
	int err = 0;
	if ( err = openMCP23017 ( i2cPortName, mcp23017Addr, &mcp23017Fd ), err )
	{
		logVerbose ( "   - error : %d\n", err );
		logVerbose ( "%s\n", strerror ( errno ) );
		return ( __LINE__ );
	}

	if ( setCloseOnExit ( mcp23017Fd ) )
	{
		logVerbose ( "%s\n", strerror ( errno ) );
		close ( mcp23017Fd );
		return ( __LINE__ );
	}

	gpioSetDir ( mcp23017Fd, 'A', 0, mcp23017_OUTPUT );
	gpioSetDir ( mcp23017Fd, 'A', 1, mcp23017_OUTPUT );
	gpioSet ( mcp23017Fd, 'A', 0, 1 );
	gpioSet ( mcp23017Fd, 'A', 1, 1 );
}

if ( pca9685Addr )
{
	logVerbose ( "   - pca9685 : %d\n", pca9685Addr );
	if ( openPCA9685 ( i2cPortName, pca9685Addr, &pca9685Fd ) )
	{
		return ( __LINE__ );
	}

	if ( setCloseOnExit ( pca9685Fd ) )
	{
		close ( pca9685Fd );
		return ( __LINE__ );
	}
}
}
else
	{ // arm disabled
		logVerbose ( " - dyna : \e[31m%s\e[0m\n", dynamixelsPath );
		logVerbose ( " - mcp23017 : \e[31m%d\e[0m (GPIO)\n", mcp23017Addr );
		logVerbose ( " - pca9685 : \e[31m%d\e[0m (PWM)\n", pca9685Addr );
		setArmDesabledState ( flagAction.noArm );
	}

	// only for display
	if ( !flagAction.noDrive )
	{ // engine enabled
		logVerbose ( " - robotclaw : %s\n", motorBoadPath );
	}
	else
	{ // engne disabled
		logVerbose ( " - robotclaw : \e[31m%s\e[0m\n", motorBoadPath );
	}

	// detection shared memory
	if ( getSharedMem ( ( void ** ) &(robot1.detection), sizeof ( *(robot1.detection) ), robot1.memKey ) )
	{
		return ( __LINE__ );
	}

	//
	// open actions xml
	//
	tabActionTotal = ouvrirXML ( &nbAction, xmlActionPath );
	if ( !tabActionTotal )
	{
		logVerbose ( "xml loading failed: -%s-\n %s\n", xmlActionPath, strerror ( errno ) );
		return ( __LINE__ );
	}
	setFreeOnExit ( tabActionTotal );
	initAction ( &flagAction );
	actionSetFd ( pca9685Fd , mcp23017Fd );

	gettimeofday ( &start, NULL );
	if ( nbAction > 0 )
	{
		tabActionTotal[0].heureCreation = start.tv_sec * 1000000 + start.tv_usec;
	}
	razAsserv();
	GPIO_init_gpio();
	while ( 1 )
	{
		calculPosition ( motorBoard, &robot1 );		

		logVerbose ( "\e[2K\rVGauche : %.3f VDroite : %.3f\n\e[A",
			robot1.vitesseGauche,
			robot1.vitesseDroite );

		// Mise à 0 des valeurs moteurs avant le parcours des actions, sans envoyer d'ordre.
		// Comme ça, si on a pas d'actions influant sur les moteurs, on arrête la bête.

		robot1.vitesseGaucheToSend = robot1.vitesseGaucheDefault;
		robot1.vitesseDroiteToSend = robot1.vitesseDroiteDefault;


		
		
		tenirAngle(&robot1);

		if ( !updateActionEnCours ( tabActionTotal, nbAction, &robot1 ) )
		{
			logVerbose ( "no more action remaining\n" );
			break;
		}

		
		/*printf("%f %f %f %f %f %f %f\n",robot1.xRobot,
			robot1.yRobot,robot1.cible.xCible,
			robot1.cible.yCible,robot1.orientationRobot,
			robot1.vitesseGaucheToSend, 
			robot1.vitesseDroiteToSend);*/

		if(detectBlocage ( &robot1, 1000 ) == 1)
		{
			arreteTesMoteursSimone();
		}else
		{
			if ( !flagAction.noDrive &&

				asservirVitesseGaucheDroite ( robot1.vitesseGaucheToSend, robot1.vitesseDroiteToSend, robot1.vitesseGauche, robot1.vitesseDroite ) )
			{ // error occured
				logVerbose ( "%s\n ", strerror ( errno ) );
			}
			else if ( flagAction.driveScan &&
				_kbhit ( ) &&
				getchar ( ) )
			{
				robot1.xRobot = robot1.cible.xCible;
				robot1.yRobot = robot1.cible.yCible;
				robot1.orientationRobot = robot1.orientationVisee;
			}
			else if ( flagAction.driveWait )
			{
				// wait timeout
			}
			else
			{
			
			}
		}



		usleep ( 1000*10 );
	}

return ( 0 );
}
