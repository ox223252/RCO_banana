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
#include "lib/roboclaw/roboclaw.h"
#include "lib/dynamixel_sdk/dynamixel_sdk.h"
#include "lib/GPIO/gpio.h"
#include "lib/mcp23017/mcp23017.h"
#include "lib/sharedMem/sharedMem.h"
#include "lib/pca9685/pca9685.h"
#include "lib/termRequest/request.h"

#include "struct/structRobot.h"
#include "struct/structAction.h"
#include "struct/structDetection.h"

#include "deplacement/odometrie.h"
#include "deplacement/controleMoteur.h"
#include "deplacement/detectionBlocage.h"

#include "utils.h"
#include "action.h"
#include "fileSelector.h"

static Robot robot1 = { 0 };
const uint8_t speedStep = 10;

#pragma GCC diagnostic ignored "-Wunused-parameter"
void actionClose ( void * arg )
{
	logVerbose ( "clean\n" );
	actionManagerDeInit ( );
}
#pragma GCC diagnostic pop

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
	int err = 0;                       // var used to stor error of init function
	void * tmp = NULL;                 // tmp pointer used for initialisation only

	char dynamixelsPath[ 128 ] = { 0 }; // dynamixel acces point /dev/dyna
	uint32_t dynamixelUartSpeed = 1000000; // dynamixel uart speed
	long int dynaPortNum = 0;          // dynamixel file descriptor

	char motorBoadPath[ 128 ] = { 0 }; // roboclaw access point /dev/roboclaw
	struct roboclaw *motorBoard = NULL; // roboclaw file descriptor

	uint32_t motorBoardUartSpeed = 115200; // uart speed

	int16_t maxSpeed = 32767;          // motor max speed, ti neved should cross this limit
	char jsonActionPath[ 128 ] = { 0 }; // xmlAction base path concatened with color chossen

	robot1.memKey = 123456;            // memory key used to shared data with detection woftware

	// battery management
	uint32_t readDelay = 5000000;      // delay between two read of data from roboclaw to manage battery
	float Vmax = 12.0;                 // maximum voltage that should provide to engnie
	float Vmin = 10.0;                 // minimum voltage needded to start engine
	float Vboost = 14.0;               // voltage authorized in boost mode
	uint32_t tBoost = 1000000;         // duration of boost mode

	//Speed PID management
	float speedAsservPG = 1.;
	float speedAsservIG = 0.;
	float speedAsservDG = 0.;
	float speedAsservPD = 1.;
	float speedAsservID = 0.;
	float speedAsservDD = 0.;

	char i2cPortName[ 64 ] = "/dev/i2c-1";

	uint8_t pca9685Addr = 0;           // servo driver addr (i2c)
	int pca9685Fd = 0;                 // pca9685 file descriptor

	uint8_t mcp23017Addr = 0;          // gpio direr addr (i2c)
	int mcp23017Fd = 0;                // mcp23017 file descriptor

	char *menuItems[] = {              // menu items used to select strategy
		"run \e[1;32mGREEN\e[0m",
		"run \e[1;31mRED\e[0m",
		"manual mode",
		"exit",
		NULL
	};

	struct
	{
		uint8_t green:1,   // &flag + 0 : 0x01
			red:1,       //             0x02
			un2:1,          //             0x04
			help:1,         //             0x08
			quiet:1,        //             0x10
			debug:1,        //             0x20
			color:1,        //             0x40
			logTerm:1;      //             0x80
		uint8_t logFile:1,  // &flag + 1 : 0x01
			verbose:1,      //             0x02
			test:1;         //             0x04
	}
	flag = { 0 };                      // some flags used to set verbosity

	ActionFlag flagAction = { 0 };

	param_el paramList[] =
	{ // paramter list used in parsin of arg cmd line
		{ "--help", 	"-h",	0x08, 	cT ( bool ), ((uint8_t * )&flag), "this window" },
		{ "--green", 	"-G",	0x01, 	cT ( bool ), ((uint8_t * )&flag), "launch the \e[32mgreen\e[0m prog" },
		{ "--red", 		"-R",	0x02, 	cT ( bool ), ((uint8_t * )&flag), "launch the \e[31mred\e[0m prog" },
		{ "--quiet", 	"-q",	0x10, 	cT ( bool ), ((uint8_t * )&flag), "hide all trace point" },
		{ "--verbose", 	"-v",	0x02, 	cT ( bool ), ((uint8_t * )&flag + 1), "set verbose mode" },
		{ "--debug", 	"-d",	0x20, 	cT ( bool ), ((uint8_t * )&flag), "display many trace point" },
		{ "--color", 	"-c",	0x40, 	cT ( bool ), ((uint8_t * )&flag), "add color to debug traces" },
		{ "--term", 	"-lT",	0x80, 	cT ( bool ), ((uint8_t * )&flag), "add color to debug traces" },
		{ "--file", 	"-lF",	0x01, 	cT ( bool ), ((uint8_t * )&flag + 1), "add color to debug traces" },
		{ "--json", 	"-j",	1, 		cT ( str ), jsonActionPath, "json action file path" },
		{ "--test",		"-t", 	0x04, 	cT ( bool ), ((uint8_t * )&flag + 1), "testAction" },
		{ "--noArm", 	"-nA",	0x01, 	cT ( bool ), ((uint8_t * )&flagAction), "use it to disable servo motor" },
		{ "--armWait", 	NULL,	0x02, 	cT ( bool ), ((uint8_t * )&flagAction), "wait end of timeout before set action to done" },
		{ "--armScan", 	NULL,	0x04, 	cT ( bool ), ((uint8_t * )&flagAction), "wait a key pressed to action to done" },
		{ "--armDone", 	NULL,	0x08, 	cT ( bool ), ((uint8_t * )&flagAction), "automaticaly set action to done (default)" },
		{ "--noDrive", 	"-nD",	0x10, 	cT ( bool ), ((uint8_t * )&flagAction), "use it to disable drive power" },
		{ "--driveWait", NULL,	0x20, 	cT ( bool ), ((uint8_t * )&flagAction), "wait end of timeout before set action to done" },
		{ "--driveScan", NULL,	0x40, 	cT ( bool ), ((uint8_t * )&flagAction), "wait a key pressed to action to done" },
		{ "--driveDone", NULL,	0x80, 	cT ( bool ), ((uint8_t * )&flagAction), "automaticaly set action to done (default)" },
		{ "--MaxSpeed", "-Ms",	1, 		cT ( int16_t ), &maxSpeed, "set max speed [ 1 ; 32767 ]" },
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
	{ // config els list used in parsing of cmd line and config file
		{ "PATH_DYNA", cT ( str ), dynamixelsPath, "PATH to access to dynamixels"},
		{ "PATH_MOTOR_BOARD", cT ( str ), motorBoadPath, "PATH to access to dynamixels"},
		{ "PATH_MOTOR_BOARD_UART_SPEED", cT ( uint32_t ), &motorBoardUartSpeed, "UART speed for robocloaw board" },
		{ "JSON_ACTION_PATH", cT ( str ), jsonActionPath, "json action file path"},
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

	// set function to restore terminal before the end of software
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


	// get config file
	if ( readConfigFile ( "res/config.rco", configList ) ||
		readConfigArgs ( argc, argv, configList ) ||
		readParamArgs ( argc, argv, paramList ) )
	{
		printf ( "no config file\n" );
		return ( __LINE__ );
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
	

	// in case of help requested
	if ( flag.help )
	{
		printf ( "\n\e[4mparameter available for cmd line:\e[0m\n" );
		helpParamArgs ( paramList );
		printf ( "\n\e[4mparameter available in res/config.rco file:\e[0m\n" );
		helpConfigArgs ( configList );
		return ( 0 );
	}

	if ( flag.test )
	{
		logVerbose ( "### TEST ###\n" );
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
	}

	colorSelect_t c[2] = {
		{ .name="green", .title="\e[1;32mGREEN\e[0m", .active=flag.green },
		{ .name="red", .title="\e[1;31mRED\e[0m", .active=flag.red }
	};
	if ( fileSelect ( jsonActionPath, 2, c ) )
	{
		return ( 0 );
	}

	flag.green = c[0].active;
	flag.red = c[1].active;

	printf ( "   use %s\n", jsonActionPath );

	if ( !flagAction.noArm )
	{ // arm enabled
		// init dynamixel
		if ( initDyna ( dynamixelsPath, &dynamixelsPath, dynamixelUartSpeed ) )
		{
			return ( __LINE__ );
		}

		// init gpio expander
		if ( initMCP23017 ( i2cPortName, mcp23017Addr, &mcp23017Fd ) )
		{
			return ( __LINE__ );
		}

		// init pwm epander
		if ( initPAC9685 ( i2cPortName, pca9685Addr, &pca9685Fd ) )
		{
			return ( __LINE__ );
		}
	}
	else
	{ // arm disabled
		logVerbose ( " - dyna : \e[31m%s\e[0m\n", dynamixelsPath );
		logVerbose ( " - mcp23017 : \e[31m%d\e[0m (GPIO)\n", mcp23017Addr );
		logVerbose ( " - pca9685 : \e[31m%d\e[0m (PWM)\n", pca9685Addr );
	}

	// only for display
	logVerbose ( " - robotclaw : %s%s\e[0m\n", (flagAction.noDrive)?"\e[31m":"", motorBoadPath );

	// detection shared memory
	if ( err = getSharedMem ( ( void ** ) &(robot1.detection), sizeof ( *(robot1.detection) ), robot1.memKey ), err )
	{
		logVerbose ( " - shared mem error %d\n", err );
		logVerbose ( "   %s\n", strerror ( errno ) );
		return ( __LINE__ );
	}

	actionManagerSetFd ( mcp23017Fd, pca9685Fd, dynaPortNum );

	// printf ( "\e[3;33m--> Don't forget to start detection\e[0m\n" );
		// if ( !updateActionEnCours ( tabActionTotal, nbAction, &robot1 ) )
		// {
		// 	logVerbose ( "no more action remaining\n" );
		// 	break;
		// }

		// if ( !flagAction.noDrive &&
		// 	detectBlocage ( &robot1, 1000 ) )
		// { // if engines are set to value and braking detected
		// 	logVerbose ("BLOCAGE\n" );
		// 	arreteTesMoteursSimone ( );
		// }
		// else if ( !flagAction.noDrive &&
		// 	asservirVitesseGaucheDroite ( robot1.vitesseGaucheToSend, robot1.vitesseDroiteToSend, robot1.vitesseGauche, robot1.vitesseDroite ) )
		// { // error occured
		// 	logVerbose ( "%s\n ", strerror ( errno ) );
		// }
		// else if ( flagAction.driveScan &&
		// 	_kbhit ( ) &&
		// 	getchar ( ) )
		// { // if we select action manageùent mode were we can set next action by keyboard key bressed
		// 	robot1.xRobot = robot1.cible.xCible;
		// 	robot1.yRobot = robot1.cible.yCible;
		// 	robot1.orientationRobot = robot1.orientationVisee;
		// }
		// else if ( flagAction.driveWait )
		// {
		// 	// wait timeout
		// }
		// else
		// {
		// 	// nothing to be done yet
		// }

	if ( actionManagerInit ( jsonActionPath, &robot1) )
	{
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	setExecAfterAllOnExit ( actionClose, NULL );

	printf ( "\n" );

	uint32_t start = getDateMs ( );
	int step = actionStartStep ( );
	do 
	{
		logDebug ( "\n" ); 
		printf ( "\e[A\e[KLOOP : %6d\n", getDateMs ( ) - start );

		static uint32_t nb = 0;
		if ( nb != actionManagerCurrentIndex ( ) )
		{
			nb = actionManagerCurrentIndex ( );
			actionManagerPrintCurrent ( );
			actionManagerPrintEnv ( );
			printf("\n");
		}

		// if no action remainig for this step, search next available step
		if ( !actionManagerCurrentNumber ( step ) )
		{
			// TODO calcul de la prochaine etape dans la strategie
			step = actionStartStep ( );
			
			if ( step < 0 )
			{
				logDebug ( "\n" );
			}
		}

		if ( actionManagerUpdate ( ) )
		{

		}

		calculPosition( motorBoard, &robot1);
		/*logVerbose ( "\e[2K\rVGauche : %.3f VDroite : %.3f\n\e[A",
			robot1.vitesseGauche,
			robot1.vitesseDroite );*/
		
		// Mise à 0 des valeurs moteurs avant le parcours des actions, sans envoyer d'ordre.
		// Comme ça, si on a pas d'actions influant sur les moteurs, on arrête la bête.

		robot1.vitesseGaucheToSend = robot1.vitesseGaucheDefault;
		robot1.vitesseDroiteToSend = robot1.vitesseDroiteDefault;

		if ( actionManagerExec ( ) )
		{

		}

		usleep ( 1000*100 );
	}
	while ( true );

	logVerbose ( "normal end\n" );

	return ( 0 );
}
