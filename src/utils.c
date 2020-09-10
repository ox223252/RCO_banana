#include <stdio.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include "lib/log/log.h"

#include "utils.h"

uint32_t fileExist ( const char * const format, ... )
{
	va_list args;
	
	char str[64] = "";
	
	va_start ( args, format );
	vsnprintf ( str, 63, format, args );
	va_end ( args );
	
	logDebug ( "%s\n", str );

	int f = open ( str, O_RDONLY );
	if ( f > 0 )
	{
		close ( f );
		return ( 1 );
	}
	return ( 0 );
}