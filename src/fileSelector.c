#include <string.h>

#include "lib/termRequest/request.h"
#include "lib/termRequest/menu.h"
#include "lib/log/log.h"

#include "utils.h"

#include "fileSelector.h"

int fileSelect ( char path[128], const uint8_t nb, colorSelect_t * const color )
{
fileSelect_start:
	if ( *path  &&
		fileExist ( path ) )
	{ // si le nom saisi correspond à un fichier on verifie pas la couleur
		return ( 0 );
	}

	int nbExist = 0;
	char *menuEl[ nb + 2];
	for ( uint8_t i = 0; i < nb; i++ )
	{
		color[ i ].exist = fileExist ( "%s-%s.json", path, color[ i ].name );
		if ( color[ i ].exist )
		{
			if ( color[ i ].active )
			{
				snprintf ( path, 127, "%s-%s.json", path, color[i].name );
				return ( 0 );
			}
			menuEl[nbExist] = color[ i ].title;
			nbExist++;
		}
	}

	if ( nbExist == 0 )
	{
		printf ( "no color available for this file\n" );
		printf ( "set new fileName\n" );
		if ( !scanf ( "%s", path ) )
		{
			logDebug ( "\n" );
			return ( __LINE__ );
		}
		while ( getchar() != '\n' );
		goto fileSelect_start;
	}

	menuEl[ nbExist ] = "Cancel";
	menuEl[ nbExist + 1 ] = NULL;

	printf ( "slect one of this file or cancel to redo\n" );
	int ret = menu ( 0, menuEl, NULL );
	if ( ret == nbExist )
	{
		*path = 0;
		goto fileSelect_start;
	}
	if ( ret > nbExist )
	{
		logDebug ( "\n" );
		return ( __LINE__ );
	}

	nbExist = 0;
	for ( uint8_t i = 0; i < nb; i++ )
	{
		if ( color[ i ].exist && nbExist == ret )
		{
			color[ i ].active = true;
			char s[128];
			snprintf ( s, 127, "%s-%s.json", path, color[i].name );
			strcpy ( path, s );
			return ( 0 );
		}
		nbExist += color[ i ].exist;
	}

	logDebug ( "\n" );
	return ( __LINE__ );
}

