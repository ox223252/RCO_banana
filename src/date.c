#include <time.h>
#include <sys/time.h>
#include <errno.h>

static struct timeval _date_timer;

int startChrono ( void )
{
	return ( gettimeofday( &_date_timer, NULL ) );
}

int getChronoValue ( struct timeval * const __restrict__ ret )
{
	if ( !ret )
	{
		errno = EINVAL;
		return ( __LINE__ );
	}

	struct timeval now;
	if ( gettimeofday ( &now, NULL ) )
	{
		return ( __LINE__ );
	}

	timersub ( &now, &_date_timer, ret );

	return ( 0 );
}