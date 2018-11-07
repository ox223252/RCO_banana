#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../log/log.h"
#include "loadXML.h"
#include "../mxml/mxml.h"

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
				}else
				{
					tabActionTotal[indiceActionEnCours].timeout = 0;
				}

				const char* type = mxmlElementGetAttr ( node, "type" );
				nodeBis = mxmlFindElement ( node, node,"fils",NULL, NULL,MXML_DESCEND );
				if ( nodeBis!=NULL )
				{
					tabActionTotal[ indiceActionEnCours ].listFils = (char*)malloc(sizeof(char)*strlen(mxmlElementGetAttr ( nodeBis, "liste" )));
					strcpy(tabActionTotal[ indiceActionEnCours ].listFils, ( char* ) mxmlElementGetAttr ( nodeBis, "liste" ));
				}else
				{
					tabActionTotal[ indiceActionEnCours ].listFils = (char*)malloc(sizeof(char)*1);
					strcpy(tabActionTotal[ indiceActionEnCours ].listFils, "-1");
				}
				//tabActionTotal[ indiceActionEnCours ].listFils = ( char* ) mxmlElementGetAttr ( nodeBis, "liste" );
				nodeBis = mxmlFindElement ( node, node,"pere",NULL, NULL,MXML_DESCEND );
				if ( nodeBis!=NULL )
				{
					tabActionTotal[ indiceActionEnCours ].listPere = (char*)malloc(sizeof(char)*strlen(mxmlElementGetAttr ( nodeBis, "liste" )));
					strcpy(tabActionTotal[ indiceActionEnCours ].listPere, ( char* ) mxmlElementGetAttr ( nodeBis, "liste" ));
				}else
				{
					tabActionTotal[ indiceActionEnCours ].listPere = (char*)malloc(sizeof(char)*1);
					strcpy(tabActionTotal[ indiceActionEnCours ].listPere, "-1");
				}
				//	tabActionTotal[ indiceActionEnCours ].listPere = ( char* ) mxmlElementGetAttr ( nodeBis, "liste" );
				nodeBis = mxmlFindElement ( node, node,"timeout",NULL, NULL,MXML_DESCEND );
				if ( nodeBis!=NULL )
				{
					tabActionTotal[ indiceActionEnCours ].listTimeOut = ( char* ) mxmlElementGetAttr ( nodeBis, "liste" );
				}
				nodeBis = mxmlFindElement ( node, node,"parametres",NULL, NULL,MXML_DESCEND );
				if(tabActionTotal[ indiceActionEnCours ].listFils != NULL && tabActionTotal[ indiceActionEnCours ].listPere != NULL)
				{
					logDebug ( "Type : %s numero %d fils : %s pere %s\n",type,tabActionTotal[ indiceActionEnCours ].numero,tabActionTotal[ indiceActionEnCours ].listFils,tabActionTotal[ indiceActionEnCours ].listPere );
				}

				if ( strcmp ( type,"actionServo" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_SERVO;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 2*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "id" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "angle" );
					logDebug ( "Servo %s %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				}
				else if ( strcmp ( type,"actionDyna" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_DYNA;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 3*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "id" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "angle" );
					tabActionTotal[ indiceActionEnCours ].params[ 2 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "vitesse" );
					logDebug ( "Dyna %s %s %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ],tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
				}
				else if ( strcmp ( type,"actionRetourDyna" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_ATTENTE_DYNA;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 2*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "id" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "angle" );
					logDebug ( "Retour Dyna %s %s \n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				}
				else if ( strcmp ( type,"actionPosition" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_POSITION;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 7*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "x" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "y" );
					tabActionTotal[ indiceActionEnCours ].params[ 2 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "vitesse" );
					tabActionTotal[ indiceActionEnCours ].params[ 3 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "acc" );
					tabActionTotal[ indiceActionEnCours ].params[ 4 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "dec" );
					tabActionTotal[ indiceActionEnCours ].params[ 5 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "sens" );
					tabActionTotal[ indiceActionEnCours ].params[ 6 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "precision" );
					logDebug ( "Position x: %s y: %s vitesse: %s acc: %s dec: %s sens: %s preci: %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ],tabActionTotal[ indiceActionEnCours ].params[ 2 ],tabActionTotal[ indiceActionEnCours ].params[ 3 ],tabActionTotal[ indiceActionEnCours ].params[ 4 ],tabActionTotal[ indiceActionEnCours ].params[ 5 ],tabActionTotal[ indiceActionEnCours ].params[ 6 ] );
				}
				else if ( strcmp ( type,"actionRotation" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_ORIENTATION;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 3*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "angle" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "vitesse" );
					tabActionTotal[ indiceActionEnCours ].params[ 2 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "precision" );
					logDebug ( "Orientation angle : %s, vitesse %s, preci %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ],tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
				}
				else if ( strcmp ( type,"actionDeplacement" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_DEPLACEMENT;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 3*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "id" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "value" );
					tabActionTotal[ indiceActionEnCours ].params[ 2 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "vitesse" );
					logDebug ( "Deplacement id : %s, value %s, vitesse %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ],tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
				}
				else if ( strcmp ( type,"actionValeur" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_SET_VALEUR;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 2*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "id" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "value" );
					logDebug ( "Valeur id : %s, value %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				}
				else if ( strcmp ( type,"actionRetourBlocage" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_ATTENTE_BLOAGE;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 1*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "sensibilite" );
					logDebug ( "Valeur sensibilite : %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				}
				else if ( strcmp ( type,"actionPause" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_ATTENTE_TEMPS;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 1*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "temps" );
					logDebug ( "Pause temps : %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ] );
				}
				else if ( strcmp ( type,"actionVar" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_SET_VARIABLE;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 3*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "numero" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "cible" );
					tabActionTotal[ indiceActionEnCours ].params[ 2 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "commande" );
					logDebug ( "Var numero : %s cible : %s commande %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ],tabActionTotal[ indiceActionEnCours ].params[ 2 ] );
				}
				else if ( strcmp ( type,"actionRetourVar" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_GET_VARIABLE;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 4*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "numero" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "cible" );
					tabActionTotal[ indiceActionEnCours ].params[ 2 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "condition" );
					tabActionTotal[ indiceActionEnCours ].params[ 3 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "value" );
					logDebug ( "Retour var numero : %s cible : %s condition %s value %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ],tabActionTotal[ indiceActionEnCours ].params[ 2 ],tabActionTotal[ indiceActionEnCours ].params[ 3 ] );
				}
				else if ( strcmp ( type,"actionGPIO" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_GPIO;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 2*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "pin" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "value" );
					logDebug ( "Valeur pin : %s, value %s\n",tabActionTotal[ indiceActionEnCours ].params[ 0 ],tabActionTotal[ indiceActionEnCours ].params[ 1 ] );
				}
				else if ( strcmp ( type,"actionRetourGpio" ) == 0 )
				{
					tabActionTotal[ indiceActionEnCours ].type = TYPE_RETOUR_GPIO;
					tabActionTotal[ indiceActionEnCours ].params = ( char** ) malloc ( 2*sizeof ( char* ) );
					tabActionTotal[ indiceActionEnCours ].params[ 0 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "pin" );
					tabActionTotal[ indiceActionEnCours ].params[ 1 ] = ( char* ) mxmlElementGetAttr ( nodeBis, "value" );
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

		logDebug ( "\e[1;33mFin parsage \e[0m%d\n",tabActionTotal[ 28 ].type );
		mxmlDelete ( tree );

		return ( tabActionTotal );
	}
