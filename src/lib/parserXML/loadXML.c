#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../log/log.h"
#include "loadXML.h"
#include "../mxml/mxml.h"
#include "../freeOnExit/freeOnExit.h"

Action* ouvrirXML ( int * nbAction, const char * restrict file )
{
	FILE *fp = NULL;
	mxml_node_t *tree = NULL;
	mxml_node_t *node = NULL;
	mxml_node_t *nodeBis = NULL;
	int indiceActionEnCours = 0;
	Action* tabActionTotal = NULL;

	if ( !file ||
		!nbAction )
	{
		errno = EINVAL;
		return ( NULL );
	}

	fp = fopen ( file, "r" );

	if ( !fp )
	{
		return ( NULL );
	}
	tree = mxmlLoadFile ( NULL, fp, MXML_OPAQUE_CALLBACK );
	fclose ( fp );
	fp = NULL;

	//On doit pouvoir optimiser et enlever cette boucle... Mais pour l'instant, elle permet de compter le nombre d'action et de malloc comme il faut.
	for ( node = mxmlFindElement ( tree, tree, "Sequence", NULL, NULL, MXML_DESCEND );
	node != NULL;
	node = mxmlFindElement ( node, tree, "Action", NULL, NULL, MXML_DESCEND ) )
	{
		const char *name = mxmlGetElement ( node );
		if ( strcmp ( name, "Action" ) == 0 )
		{
			( * ( nbAction ) )++;
		}
	}

	tabActionTotal = ( Action* ) malloc ( ( *nbAction ) * ( sizeof ( Action ) ) );
	if ( !tabActionTotal )
	{
		return ( NULL );
	}

	for ( node = mxmlFindElement ( tree, tree, "Sequence", NULL, NULL, MXML_DESCEND );
		node != NULL;
		node = mxmlFindElement ( node, tree, "Action", NULL, NULL, MXML_DESCEND ) )
	{
		const char *name = mxmlGetElement ( node );
		if ( strcmp ( name, "Action" ) == 0 )
		{
			tabActionTotal[ indiceActionEnCours ].numero = atoi ( mxmlElementGetAttr ( node, "numero" ) );

			if ( mxmlElementGetAttr ( node, "timeout" ) != NULL )
			{
				tabActionTotal[ indiceActionEnCours ].timeout = atoi ( mxmlElementGetAttr ( node, "timeout" ) );
				
				nodeBis = mxmlFindElement ( node, node, "timeout", NULL, NULL, MXML_DESCEND );
				if ( nodeBis != NULL )
				{
					tabActionTotal[ indiceActionEnCours ].listTimeOut = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "liste" ) ) + 1 );
					setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].listTimeOut );
					strcpy ( tabActionTotal[ indiceActionEnCours ].listTimeOut, mxmlElementGetAttr ( nodeBis, "liste" ) );
					logDebug ( "%d %s\n", indiceActionEnCours, tabActionTotal[ indiceActionEnCours ].listTimeOut );
				}
				else
				{
					tabActionTotal[ indiceActionEnCours ].listFils = NULL;
				}
			}
			else
			{
				tabActionTotal[indiceActionEnCours].timeout = 0;
				tabActionTotal[ indiceActionEnCours ].listFils = NULL;
			}

			const char* type = mxmlElementGetAttr ( node, "type" );
			nodeBis = mxmlFindElement ( node, node,"fils",NULL, NULL,MXML_DESCEND );
			if ( nodeBis!=NULL )
			{
				tabActionTotal[ indiceActionEnCours ].listFils = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "liste" ) ) + 1 );
				strcpy ( tabActionTotal[ indiceActionEnCours ].listFils, mxmlElementGetAttr ( nodeBis, "liste" ) );
			}
			else
			{
				tabActionTotal[ indiceActionEnCours ].listFils = malloc ( 3 );
				strcpy ( tabActionTotal[ indiceActionEnCours ].listFils, "-1" );
			}
			setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].listFils );

			nodeBis = mxmlFindElement ( node, node,"pere",NULL, NULL,MXML_DESCEND );
			if ( nodeBis!=NULL )
			{
				tabActionTotal[ indiceActionEnCours ].listPere = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "liste" ) ) + 1 );
				strcpy(tabActionTotal[ indiceActionEnCours ].listPere, ( char* ) mxmlElementGetAttr ( nodeBis, "liste" ) );
			}
			else
			{
				tabActionTotal[ indiceActionEnCours ].listPere = malloc ( 3 );
				strcpy ( tabActionTotal[ indiceActionEnCours ].listPere, "-1" );
			}
			setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].listPere );

			nodeBis = mxmlFindElement ( node, node, "parametres", NULL, NULL, MXML_DESCEND );
			if ( ( tabActionTotal[ indiceActionEnCours ].listFils != NULL ) &&
				( tabActionTotal[ indiceActionEnCours ].listPere != NULL ) )
			{
				logDebug ( "Type : %s numero %d fils : %s pere %s\n",type,tabActionTotal[ indiceActionEnCours ].numero,tabActionTotal[ indiceActionEnCours ].listFils,tabActionTotal[ indiceActionEnCours ].listPere );
			}

			if ( strcmp ( type,"actionServo" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_SERVO;
				tabActionTotal[ indiceActionEnCours ].params = malloc ( 2 * sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "id" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "id" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "angle" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "angle" ) );


				logDebug ( "Servo %s %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
			}
			else if ( strcmp ( type,"actionDyna" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_DYNA;
				tabActionTotal[ indiceActionEnCours ].params = malloc ( 3 * sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "id" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );

				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "id" ) );
				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "angle" ) ) + 1 );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "angle" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 2 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "vitesse" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 2 ], ( char* ) mxmlElementGetAttr ( nodeBis, "vitesse" ) );

				logDebug ( "Dyna %s %s %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ],tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
			}
			else if ( strcmp ( type,"actionRetourDyna" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_ATTENTE_DYNA;
				tabActionTotal[ indiceActionEnCours ].params = malloc ( 2 * sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "id" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "id" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "angle" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "angle" ) );


				logDebug ( "Retour Dyna %s %s \n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
			}
			else if ( strcmp ( type,"actionPosition" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_POSITION;
				tabActionTotal[ indiceActionEnCours ].params = malloc ( 8 * sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "x" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "x" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "y" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "y" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 2 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "vitesse" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 2 ], ( char* ) mxmlElementGetAttr ( nodeBis, "vitesse" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 3 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "acc" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 3 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 3 ], ( char* ) mxmlElementGetAttr ( nodeBis, "acc" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 4 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "dec" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 4 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 4 ], ( char* ) mxmlElementGetAttr ( nodeBis, "dec" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 5 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "sens" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 5 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 5 ], ( char* ) mxmlElementGetAttr ( nodeBis, "sens" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 6 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "precision" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 6 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 6 ], ( char* ) mxmlElementGetAttr ( nodeBis, "precision" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 7 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "distanceFreinage" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 7 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 7 ], ( char* ) mxmlElementGetAttr ( nodeBis, "distanceFreinage" ) );

				logDebug ( "Position x: %s y: %s vitesse: %s acc: %s dec: %s sens: %s preci: %s distanceFreinage %s\n",
					tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ],
					tabActionTotal[ indiceActionEnCours ].params[ 2 ],tabActionTotal[ indiceActionEnCours ].params[ 3 ],
					tabActionTotal[ indiceActionEnCours ].params[ 4 ],tabActionTotal[ indiceActionEnCours ].params[ 5 ],
					tabActionTotal[ indiceActionEnCours ].params[ 6 ],tabActionTotal[ indiceActionEnCours ].params[ 7 ] );
			}
			else if ( strcmp ( type,"actionRotation" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_ORIENTATION;
				tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 3*sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "angle" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "angle" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "vitesse" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "vitesse" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 2 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "precision" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 2 ], ( char* ) mxmlElementGetAttr ( nodeBis, "precision" ) );


				logDebug ( "Orientation angle : %s, vitesse %s, preci %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ],tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
			}
			else if ( strcmp ( type,"actionDeplacement" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_DEPLACEMENT;
				tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 3*sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "id" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "id" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "value" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "value" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 2 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "vitesse" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 2 ], ( char* ) mxmlElementGetAttr ( nodeBis, "vitesse" ) );

				logDebug ( "Deplacement id : %s, value %s, vitesse %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ],tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
			}
			else if ( strcmp ( type,"actionValeur" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_SET_VALEUR;
				tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 2*sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "id" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "id" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "value" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "value" ) );

				logDebug ( "Valeur id : %s, value %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
			}
			else if ( strcmp ( type,"actionRetourBlocage" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_ATTENTE_BLOCAGE;
				tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 1*sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "sensibilite" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "sensibilite" ) );

				logDebug ( "Valeur sensibilite : %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
			}
			else if ( strcmp ( type,"actionPause" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_ATTENTE_TEMPS;
				tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 1*sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "temps" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "temps" ) );

				logDebug ( "Pause temps : %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
			}
			else if ( strcmp ( type,"actionVar" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_SET_VARIABLE;
				tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 2*sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "nom" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "nom" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "valeur" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "valeur" ) );

				logDebug ( "Var nom : %s valeur : %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
			}
			else if ( strcmp ( type,"actionRetourVar" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_GET_VARIABLE;
				tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 3*sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "nom" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "nom" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "attendu" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "attendue" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 2 ] = NULL;
				// one the value will be read its will be stored in param[ 2 ], no need to malloc for it else you will provoc memory leak

				logDebug ( "Var nom : %s valeur attendue : %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
			}
			else if ( strcmp ( type,"actionGPIO" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_GPIO;
				tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 2*sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "pin" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "pin" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "value" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "value" ) );

				logDebug ( "Valeur pin : %s, value %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
			}
			else if ( strcmp ( type,"actionRetourGpio" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_RETOUR_GPIO;
				tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 2*sizeof ( char* ) );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params );

				tabActionTotal[ indiceActionEnCours ].params[ 0 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "pin" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 0 ], ( char* ) mxmlElementGetAttr ( nodeBis, "pin" ) );

				tabActionTotal[ indiceActionEnCours ].params[ 1 ] = malloc ( strlen ( mxmlElementGetAttr ( nodeBis, "value" ) ) + 1 );
				setFreeOnExit ( tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				strcpy(tabActionTotal[ indiceActionEnCours ].params[ 1 ], ( char* ) mxmlElementGetAttr ( nodeBis, "value" ) );

				logDebug ( "Valeur pin : %s, value %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
			}
			else if ( strcmp ( type,"actionDepart" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_ENTREE;
				logDebug ( "Départ \n" );
			}
			else if ( strcmp ( type,"actionFin" ) == 0 )
			{
				tabActionTotal[ indiceActionEnCours ].type = TYPE_FIN;
				logDebug ( "Fin\n" );
			}
			else
			{
				logDebug ( "Type unknown !\n" );
			}
			
			indiceActionEnCours++;
		}
	}

	mxmlDelete ( tree );

	return ( tabActionTotal );
}
