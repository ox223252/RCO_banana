#include "management.h"
#include "../lib/log/log.h"
#include "../lib/freeOnExit/freeOnExit.h"

static char* listActionEnCours = NULL;
int nbActionEnCours = 1;

void gestionAction(Action* listAction, Robot* robot, int indiceAction)
{
	logDebug ( "GESTION ACTION %d type : %d\n", indiceAction,listAction[indiceAction].type );

	struct timeval now;


	switch(listAction[indiceAction].type)
	{
		case TYPE_SERVO:
		break;

		case TYPE_DYNA:
		logDebug("Dyna : id %s, angle %s, vitesse %s\n",listAction[ indiceAction ].params[ 0 ],listAction[ indiceAction ].params[ 1 ],listAction[ indiceAction ].params[ 2 ]);
		listAction[indiceAction].isDone = 1;
		break;

		case TYPE_CAPTEUR:
		break;

		case TYPE_MOTEUR:
		break;

		case TYPE_AUTRE:
		break;

		case TYPE_POSITION:
		break;

		case TYPE_ORIENTATION:
		break;

		case TYPE_SEQUENCE:
		break;

		case TYPE_ENTREE:
		listAction[indiceAction].isDone = 1;
		break;

		case TYPE_ATTENTE_SERVO:
		break;

		case TYPE_ATTENTE_DYNA:
		break;

		case TYPE_ATTENTE_TEMPS:

		gettimeofday ( &now, NULL );
		logDebug ( "attente %d type : %d\n", ( now.tv_sec * 1000000 + now.tv_usec - listAction[ indiceAction ].heureCreation ),1000* atoi(listAction[indiceAction].params[ 0 ]) );
		if(( now.tv_sec * 1000000 + now.tv_usec - listAction[ indiceAction ].heureCreation ) >= 1000* atoi(listAction[indiceAction].params[ 0 ]))
		{
			listAction[indiceAction].isDone = 1;
		}
		break;

		case TYPE_RETOUR_DEPLACEMENT:
		break;

		case TYPE_RETOUR_ORIENTATION:
		break;

		case TYPE_RETOUR_POSITION:
		break;

		case TYPE_GPIO:
		break;

		case TYPE_RETOUR_GPIO:
		break;

		case TYPE_AND:
		break;

		case TYPE_SET_VALEUR:
		break;

		case TYPE_COURBE:
		break;

		case TYPE_ATTENTE_BLOAGE:
		break;

		case TYPE_DEPLACEMENT:
		break;

		case TYPE_FIN:
		break;

		case TYPE_SET_VARIABLE:
		break;

		case TYPE_GET_VARIABLE:
		break;

	}
}

int initAction ( void )
{
	if ( !listActionEnCours )
	{
		listActionEnCours = malloc ( 256 );
		if ( !listActionEnCours )
		{
			return ( __LINE__ );
		}

		setFreeOnExit ( listActionEnCours );

		sprintf ( listActionEnCours, "0;" );

		return ( 0 );
	}
	else
	{
		free ( listActionEnCours );
		unsetFreeOnExit ( listActionEnCours );
		listActionEnCours = NULL;

		return ( initAction ( ) );
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


	if ( !listActionEnCours )
	{
		return;
	}

	listCOPY = malloc ( strlen ( listActionEnCours ) + 1 );
	strcpy ( listCOPY, listActionEnCours );

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

	sprintf ( listActionEnCours, "%s", newList );
	logDebug ( " - new list  : %s\n", listActionEnCours );

	free ( listCOPY );
	return ( actionRemaining );
}

int getIndiceActionByIndice(Action* listAction, int indiceAction, int nbAction)
{
	for(int i=0;i<nbAction;i++)
	{
		if(listAction[i].numero == indiceAction)
		{
			return i;
		}
	}
	return -1;
}
