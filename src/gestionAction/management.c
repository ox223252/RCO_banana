#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <netdb.h> 
#include <sys/types.h> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include "management.h"
#include "../lib/log/log.h"
#include "../lib/freeOnExit/freeOnExit.h"
#include "../lib/termRequest/request.h"
#include "../lib/jsonParser/jsonParser.h"
#include "../lib/pca9685/pca9685.h"
#include "../lib/GPIO/gpio.h"
#include "../lib/mcp23017/mcp23017.h"
#define PORT 12345
static char* _management_listActionEnCours = NULL;
static ActionFlag *_management_flagAction = NULL;
static int8_t _management_newDeplacement = 1;
static int  _management_pca9685 = 0;
static int  _management_mcp23017 = 0;

static int getIndiceActionByIndice ( Action* listAction, int indiceAction, int nbAction );

static json_el *_management_json = NULL;
static uint32_t _management_jsonLength = 0;

#pragma GCC diagnostic ignored "-Wunused-parameter"
void managementAfterAll ( void * arg )
{
	jsonFree ( &_management_json, _management_jsonLength );
}
#pragma GCC diagnostic pop
int sockfd;
struct hostent *he;
struct sockaddr_in their_addr;

int initAction ( ActionFlag *flag )
{
	if ( !flag )
	{ // verify parameter
		errno = EINVAL;
		return ( __LINE__ );
	}

	if ( !_management_listActionEnCours )
	{ // if not set
		_management_flagAction = flag;
		_management_listActionEnCours = malloc ( 256 );
		if ( !_management_listActionEnCours )
		{
			return ( __LINE__ );
		}
		setFreeOnExit ( _management_listActionEnCours );

		if ( jsonParseString ( "{}", &_management_json, &_management_jsonLength ) )
		{
			return ( __LINE__ );
		}
		setExecAfterAllOnExit ( managementAfterAll, NULL );

		sprintf ( _management_listActionEnCours, "0;" );
		he=gethostbyname("192.168.43.47");
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		their_addr.sin_family = AF_INET;      /* host byte order */
        their_addr.sin_port = htons(PORT);    /* short, network byte order */
        their_addr.sin_addr = *((struct in_addr *)he->h_addr);
        bzero(&(their_addr.sin_zero), 8);
		return ( 0 );
	}
	else
	{ // if already set, free all and set again
		free ( _management_listActionEnCours );
		unsetFreeOnExit ( _management_listActionEnCours );
		unsetExecAfterAllOnExit ( managementAfterAll );
		_management_listActionEnCours = NULL;

		return ( initAction ( flag ) );
	}
}

void actionSetFd ( int pca9685 , int mcp23017)
{
	_management_pca9685 = pca9685;
	_management_mcp23017 = mcp23017;
}

void gestionAction ( Action* listAction, Robot* robot, int indiceAction )
{
	logDebug ( "GESTION ACTION %d type : %d\n", indiceAction,listAction[indiceAction].type );

	struct timeval now;

	switch ( listAction[indiceAction].type )
	{
		case TYPE_SERVO:
		{ // done
			//setPCA9685PWM ( atoi ( listAction[ indiceAction ].params[ 0 ] ), 0, 210 + atoi ( listAction[ indiceAction ].params[ 1 ] ) % 360, *_management_pca9685 );
			listAction[indiceAction].isDone = 1;
			break;
		}
		case TYPE_DYNA:
		{ // done
			if ( _management_flagAction->noArm )
			{
				if ( _management_flagAction->armScan )
				{
					while ( _kbhit ( ) )
					{
						listAction[indiceAction].isDone = 1;
					}
				}
				else if ( _management_flagAction->armWait )
				{ 
				}
				else
				{ // arm done
					listAction[indiceAction].isDone = 1;
				}
			}
			else
			{
				if ( setVitesseDyna ( atoi ( listAction[ indiceAction ].params[ 0 ]), ( int )( 10.23*atoi ( listAction[ indiceAction ].params[ 2 ] ) ) ) )
				{
					logVerbose ( "Erreur set vitesse dyna \n" );
				}
				if ( setPositionDyna ( atoi ( listAction[ indiceAction ].params[ 0 ]), ( int )( atoi ( listAction[ indiceAction ].params[ 1 ] ) ) ) )
				{
					logVerbose ( "Erreur set angle dyna \n" );
				}
				logDebug ("Dyna : id %s, angle %s, vitesse %s\n",listAction[ indiceAction ].params[ 0 ],listAction[ indiceAction ].params[ 1 ],listAction[ indiceAction ].params[ 2 ]);
				listAction[indiceAction].isDone = 1;
			}
			break;
		}
		case TYPE_CAPTEUR:
		{
			break;
		}
		case TYPE_MOTEUR:
		{ // done 
			if ( _management_flagAction->noDrive )
			{
				if ( _management_flagAction->driveScan )
				{
					while ( _kbhit ( ) )
					{
						listAction[indiceAction].isDone = 1;
					}
				}
				else if ( _management_flagAction->driveWait )
				{
				}
				else
				{ // drive done
					listAction[indiceAction].isDone = 1;
				}
			}
			else
			{

			}
			break;
		}
		case TYPE_AUTRE:
		{
			break;
		}
		case TYPE_POSITION:
		{ // done
			if ( _management_newDeplacement == 1 )
			{
				_management_newDeplacement = 0;
				robot->vitesseGaucheDefault = 0.;
				robot->vitesseDroiteDefault = 0.;
				resetBlocage();
				robot->cible.xCible = atoi ( listAction[ indiceAction ].params[ 0 ] );
				robot->cible.yCible = atoi ( listAction[ indiceAction ].params[ 1 ] );
				robot->cible.vitesseMax = atoi ( listAction[ indiceAction ].params[ 2 ] );
				robot->cible.acc = atoi ( listAction[ indiceAction ].params[ 3 ] );
				robot->cible.dec = atoi ( listAction[ indiceAction ].params[ 4 ] );
				robot->cible.sens = atoi ( listAction[ indiceAction ].params[ 5 ] );
				robot->cible.precision = atoi ( listAction[ indiceAction ].params[ 6 ] );
				robot->cible.distanceFreinage = atoi ( listAction[ indiceAction ].params[ 7 ] );
				robot->setDetection = atoi ( listAction[ indiceAction ].params[ 8 ] );

				premierAppel ( robot );
			}
			else if ( calculDeplacement ( robot )==1 )
			{
				_management_newDeplacement = 1;
				listAction[indiceAction].isDone = 1;
				robot->vitesseGaucheDefault = 0.;
				robot->vitesseDroiteDefault = 0.;
			}

			break;
		}
		case TYPE_ORIENTATION:
		{ // done
			if ( _management_newDeplacement == 1 )
			{
				_management_newDeplacement = 0;
				robot->vitesseGaucheDefault = 0.;
				robot->vitesseDroiteDefault = 0.;
				resetBlocage();
				robot->orientationVisee = atoi ( listAction[ indiceAction ].params[ 0 ] );
				robot->cible.vitesseMax = atoi ( listAction[ indiceAction ].params[ 1 ] );
				robot->cible.precision = atoi ( listAction[ indiceAction ].params[ 2 ] );
				premierAppelTenirAngle ( robot );
			}
			else
			{
				if ( tenirAngle ( robot )==1 )
				{
					_management_newDeplacement = 1;
					listAction[indiceAction].isDone = 1;
					robot->vitesseGaucheDefault = 0.;
					robot->vitesseDroiteDefault = 0.;
				}
			}
			break;
		}
		case TYPE_SEQUENCE:
		{
			break;
		}
		case TYPE_ENTREE:
		{ // done
			listAction[indiceAction].isDone = 1;
			break;
		}
		case TYPE_ATTENTE_SERVO:
		{
			break;
		}
		case TYPE_ATTENTE_DYNA:
		{ // done
			//id:param0 value:param1
			if ( abs ( getPositionDyna ( atoi ( listAction[ indiceAction ].params[ 0 ] ) ) - atoi ( listAction[ indiceAction ].params[ 1 ] ) ) < 5 )
			{
				listAction[indiceAction].isDone = 1;
			}
			break;
		}
		case TYPE_ATTENTE_TEMPS:
		{ // done
			gettimeofday ( &now, NULL );
			logDebug ( "attente %d type : %d\n", ( now.tv_sec * 1000000 + now.tv_usec - listAction[ indiceAction ].heureCreation ),1000* atoi ( listAction[indiceAction].params[ 0 ]) );
			if ( ( int )( now.tv_sec * 1000000 + now.tv_usec - listAction[ indiceAction ].heureCreation ) >= ( 1000 * atoi ( listAction[indiceAction].params[ 0 ] ) ) )
			{
				listAction[indiceAction].isDone = 1;
			}
			break;
		}
		case TYPE_RETOUR_DEPLACEMENT:
		{

			break;
		}
		case TYPE_RETOUR_ORIENTATION:
		{
			break;
		}
		case TYPE_RETOUR_POSITION:
		{
			break;
		}
		case TYPE_GPIO:
		{
			//printf("GPIO : %s %s %d\n",listAction[ indiceAction ].params[ 0 ],listAction[ indiceAction ].params[ 1 ],*(_management_mcp23017));
			gpioSet ( _management_mcp23017, 'A', atoi ( listAction[ indiceAction ].params[ 0 ] ), atoi ( listAction[ indiceAction ].params[ 1 ] ) != 1 );

			listAction[indiceAction].isDone = 1;
			break;
		}
		case TYPE_RETOUR_GPIO:
		{
			if(GPIORead(atoi ( listAction[ indiceAction ].params[ 0 ] )) == atoi ( listAction[ indiceAction ].params[ 1 ] ))
			{
				listAction[indiceAction].isDone = 1;
			}
				
			break;
		}
		case TYPE_AND:
		{
			break;
		}
		case TYPE_SET_VALEUR: //fonction
		{ // done
			switch ( atoi ( listAction[ indiceAction ].params[ 0 ] ) )
			{
				case 0:
				{
					//xRobot
					robot->xRobot = atoi ( listAction[ indiceAction ].params[ 1 ] );
					listAction[indiceAction].isDone = 1;
					break;
				}
				case 1:
				{
					//yRobot
					robot->yRobot = atoi ( listAction[ indiceAction ].params[ 1 ] );
					listAction[indiceAction].isDone = 1;
					break;
				}
				case 2:
				{
					//Orientation Robot
					robot->orientationRobot = atoi ( listAction[ indiceAction ].params[ 1 ] );
					robot->orientationVisee = atoi ( listAction[ indiceAction ].params[ 1 ] );

					listAction[indiceAction].isDone = 1;
					break;
				}
				case 3:
				{
					//Vitesse Linéaire
					robot->vitesseGaucheDefault = atoi ( listAction[ indiceAction ].params[ 1 ] );
					robot->vitesseDroiteDefault = atoi ( listAction[ indiceAction ].params[ 1 ] );
					listAction[indiceAction].isDone = 1;
					break;
				}
				case 4:
				{
					//Vitesse Angulaire
					robot->vitesseGaucheDefault = -1.* atoi ( listAction[ indiceAction ].params[ 1 ] );
					robot->vitesseDroiteDefault = atoi ( listAction[ indiceAction ].params[ 1 ] );
					listAction[indiceAction].isDone = 1;
					break;
				}
			}
			break;
		}
		case TYPE_COURBE:
		{
			break;
		}
		case TYPE_ATTENTE_BLOCAGE:
		{
			break;
		}
		case TYPE_DEPLACEMENT:
		{
			break;
		}
		case TYPE_FIN:
		{
			listAction[indiceAction].isDone = 1;
			break;
		}
		case TYPE_SET_VARIABLE:
		{
			jsonSet ( _management_json, 0, listAction[ indiceAction ].params[ 0 ], listAction[ indiceAction ].params[ 1 ], jT ( str ) );
			listAction[indiceAction].isDone = 1;
			break;
		}
		case TYPE_GET_VARIABLE:
		{
			jsonGet ( _management_json, 0, listAction[ indiceAction ].params[ 0 ], (void **)&listAction[ indiceAction ].params[ 2 ], NULL );
			if ( !strcmp ( listAction[ indiceAction ].params[ 1 ], listAction[ indiceAction ].params[ 2 ] ) )
			{
				listAction[indiceAction].isDone = 1;
			}
			break;
		}
		default:
		{
			break;
		}
	}
}

int updateActionEnCours ( Action* listAction, int nbAction, Robot* robot )
{
	// Returns first token
	char* listCOPY = NULL;
	char buffer[10] = { 0 };
	char newList[ 256 ] = { 0 };
	char *token = NULL;
	int j = 0;
	int numAction = 0;
	int newAction = 0;

	int actionRemaining = 0;

	struct timeval now;

	if ( !_management_listActionEnCours )
	{
		logDebug ( "\n" );
		return 0;
	}

	listCOPY = malloc ( strlen ( _management_listActionEnCours ) + 1 );
	strcpy ( listCOPY, _management_listActionEnCours );

	token = strtok ( listCOPY, ";" );

	// Keep printing tokens while one of the
	// delimiters present in str[].
	gettimeofday ( &now, NULL );
	do
	{
		numAction = atoi ( token );

		logDebug ( "en cours : %s\n", token );
		logDebug ( " - start time : %ld\n", listAction[ numAction ].heureCreation / 1000000 );
		logDebug ( " - fils       : %s\n", listAction[ numAction ].listFils );
		if ( listAction[ numAction ].timeout > 0 )
		{
			logDebug ( " - timeout %d\n", listAction[ numAction ].timeout );
			logDebug ( "    - fils %s\n", listAction[ numAction ].listTimeOut );
		}


		gestionAction ( listAction, robot, numAction );
		if ( isDone ( &( listAction[ numAction ] ) ) == 1 )
		{ // normal termination
			if ( !listAction[ numAction ].listFils )
			{ // no child list provided
			}
			else
			{
				j = 0;
				while ( sscanf ( listAction[ numAction ].listFils + j, "%[^;]", buffer ) )
				{ // get actions
					if ( listAction[ numAction ].listFils[ j ] == 0 )
					{
						break;
					}
					newAction = getIndiceActionByIndice ( listAction, atoi ( buffer ), nbAction );
					listAction[ newAction ].heureCreation = now.tv_sec * 1000000 + now.tv_usec;
					sprintf ( newList, "%s%d;", newList, newAction );

					actionRemaining++;

					j += ( int )( strlen ( buffer ) + 1 );
					logDebug ( " - new action : %s\n", buffer );
				}
			}
		}
		else if ( ( listAction[ numAction ].timeout > 0 ) &&
			( ( int )( now.tv_sec * 1000000 + now.tv_usec - listAction[ numAction ].heureCreation ) > ( listAction[ numAction ].timeout * 1000 ) ) )
		{ // end by timeout
			logDebug ("Done by timeout %d %d\n", listAction[ numAction ].timeout,  now.tv_sec * 1000000 + now.tv_usec - listAction[ numAction ].heureCreation );

			if ( !listAction[ numAction ].listTimeOut )
			{ // no timeout list provided
			}
			else
			{
				logDebug ( " - liste : %s\n", listAction[ numAction ].listTimeOut );
				j = 0;
				while ( sscanf ( listAction[ numAction ].listTimeOut + j, "%[^;]", buffer ) )
				{ // get actions
					if ( listAction[ numAction ].listTimeOut[ j ] == 0 )
					{
						break;
					}
					newAction = getIndiceActionByIndice ( listAction, atoi ( buffer ), nbAction );
					listAction[ newAction ].heureCreation = now.tv_sec * 1000000 + now.tv_usec;
					sprintf ( newList, "%s%d;", newList, newAction );

					actionRemaining++;

					j += ( int )( strlen ( buffer ) + 1 );
					logDebug ( " - new action : %s\n", buffer );
					break;
				}
			}
		}
		else
		{ // not done
			logDebug ("not done yet\n");
			sprintf ( newList, "%s%s;", newList, token );
			actionRemaining++;
		}

		token = strtok ( NULL, ";");
	}
	while ( token != NULL );

	sprintf ( _management_listActionEnCours, "%s", newList );
	logDebug ( " - new list  : %s\n", _management_listActionEnCours );

	free ( listCOPY );
	return ( actionRemaining );
}

static int getIndiceActionByIndice ( Action* listAction, int indiceAction, int nbAction )
{
	for ( int i = 0; i < nbAction ; i++ )
	{
		if ( listAction[ i ].numero == indiceAction )
		{
			return ( i );
		}
	}
	return ( -1 );
}
