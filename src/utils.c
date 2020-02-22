#include <stdio.h>
#include <sys/time.h>

#include "utils.h"

uint32_t getDateMs ( void )
{
	struct timeval now;
	gettimeofday ( &now, NULL );
	return ( now.tv_usec / 1000 + now.tv_sec * 1000 );
}