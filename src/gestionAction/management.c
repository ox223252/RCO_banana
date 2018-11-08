#include <errno.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "management.h"
#include "../lib/log/log.h"
#include "../lib/freeOnExit/freeOnExit.h"
#include "../lib/termRequest/request.h"

static char* _management_listActionEnCours = NULL;
static ActionFlag *_management_flagAction = NULL;
static int _management_nbActionEnCours = 1;

int initAction ( ActionFlag *flag )
{
	if ( !_management_flagAction )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	if ( !_management_listActionEnCours )
	{
		_management_flagAction = flag;

		_management_listActionEnCours = malloc ( 256 );
		if ( !_management_listActionEnCours )
		{
			return ( __LINE__ );
		}

		setFreeOnExit ( _management_listActionEnCours );

		sprintf ( _management_listActionEnCours, "0;" );

		return ( 0 );
	}
	else
	{
		free ( _management_listActionEnCours );
		unsetFreeOnExit ( _management_listActionEnCours );
		_management_listActionEnCours = NULL;

		return ( initAction ( flag ) );
	}
}

void gestionAction ( Action* listAction, Robot* robot, int indiceAction )
{
	logDebug ( "GESTION ACTION %d type : %d\n", indiceAction,listAction[indiceAction].type );

	void * mask = NULL;
	struct timeval now;
	struct timeval tv = { 0 };
	fd_set rfds;

	switch(listAction[indiceAction].type)
	{
		case TYPE_SERVO:
		{
			break;
		}
		case TYPE_DYNA:
		{

			if ( _management_flagAction->noArm )
			{
				if ( _management_flagAction->armWait )
				{ // a faire

				}
				else if ( _management_flagAction->armScan )
				{
					FD_ZERO ( &rfds );
					FD_SET ( 0, &rfds );

					setBlockMode ( &mask, true );
					while ( select ( 1, &rfds, NULL, NULL, &tv) > 0 )
					{
						printf ( "%d", getchar ( ) );
						listAction[indiceAction].isDone = 1;
					}
					resetBlockMode ( mask );
				}
				else
				{ // arm done
					listAction[indiceAction].isDone = 1;
				}
			}
			else
			{
				logDebug("Dyna : id %s, angle %s, vitesse %s\n",listAction[ indiceAction ].params[ 0 ],listAction[ indiceAction ].params[ 1 ],listAction[ indiceAction ].params[ 2 ]);
				listAction[indiceAction].isDone = 1;
			}
			break;
		}
		case TYPE_CAPTEUR:
		{
			break;
		}
		case TYPE_MOTEUR:
		{
			if ( _management_flagAction->noDrive )
			{
				if ( _management_flagAction->driveWait )
				{ // a faire

				}
				else if ( _management_flagAction->driveScan )
				{
					void * mask = NULL;
					struct timeval tv = { 0 };
					fd_set rfds;

					FD_ZERO ( &rfds );
					FD_SET ( 0, &rfds );

					setBlockMode ( &mask, true );
					while ( select ( 1, &rfds, NULL, NULL, &tv) > 0 )
					{
						printf ( "%d", getchar ( ) );
						listAction[indiceAction].isDone = 1;
					}
					resetBlockMode ( mask );
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
		{
			break;
		}
		case TYPE_ORIENTATION:
		{
			break;
		}
		case TYPE_SEQUENCE:
		{
			break;
		}
		case TYPE_ENTREE:
		{
			listAction[indiceAction].isDone = 1;
			break;
		}
		case TYPE_ATTENTE_SERVO:
		{
			break;
		}
		case TYPE_ATTENTE_DYNA:
		{
			break;
		}
		case TYPE_ATTENTE_TEMPS:
		{
			gettimeofday ( &now, NULL );
			logDebug ( "attente %d type : %d\n", ( now.tv_sec * 1000000 + now.tv_usec - listAction[ indiceAction ].heureCreation ),1000* atoi(listAction[indiceAction].params[ 0 ]) );
			if(( now.tv_sec * 1000000 + now.tv_usec - listAction[ indiceAction ].heureCreation ) >= 1000* atoi(listAction[indiceAction].params[ 0 ]))
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
			break;
		}
		case TYPE_RETOUR_GPIO:
		{
			break;
		}
		case TYPE_AND:
		{
			break;
		}
		case TYPE_SET_VALEUR:
		{
			break;
		}
		case TYPE_COURBE:
		{
			break;
		}
		case TYPE_ATTENTE_BLOAGE:
		{
			break;
		}
		case TYPE_DEPLACEMENT:
		{
			break;
		}
		case TYPE_FIN:
		{
			break;
		}
		case TYPE_SET_VARIABLE:
		{
			break;
		}
		case TYPE_GET_VARIABLE:
		{
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
		return;
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


		gestionAction(listAction,robot, numAction);
		if ( isDone ( &(listAction[ numAction ] ) ) == 1 )
		{ // normal termination
			j = 0;
			while ( sscanf ( listAction[ numAction ].listFils + j, "%[^;]", buffer) )
			{ // get actions
				if ( listAction[ numAction ].listFils[ j ] == 0 )
				{
					break;
				}
				newAction = getIndiceActionByIndice ( listAction, atoi ( buffer ), nbAction );
				listAction[ newAction ].heureCreation = now.tv_sec * 1000000 + now.tv_usec;
				sprintf ( newList, "%s%d;", newList, newAction );

				actionRemaining++;

				j += (int)( strlen ( buffer ) + 1 );
				logDebug ( " - new action : %s\n", buffer );
			}
		}
		else if ( ( listAction[ numAction ].timeout > 0 ) &&
		( ( now.tv_sec * 1000000 + now.tv_usec - listAction[ numAction ].heureCreation ) < ( listAction[ numAction ].timeout * 1000 ) ) )
		{ // end by timeout

		}
		else
		{ // not done
			sprintf ( newList, "%s%s;", newList, token );
			actionRemaining++;
		}

		token = strtok(NULL, ";");
	}
	while ( token != NULL );

	sprintf ( _management_listActionEnCours, "%s", newList );
	logDebug ( " - new list  : %s\n", _management_listActionEnCours );

	free ( listCOPY );
	return ( actionRemaining );
}

int getIndiceActionByIndice ( Action* listAction, int indiceAction, int nbAction )
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
