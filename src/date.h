#ifndef __DATE_H__
#define __DATE_H__

#include <time.h>
#include <sys/time.h>
#include <stdint.h>

int startChrono ( void );
int getChronoValue ( struct timeval * const __restrict__ ret );
double getChronoMs ( void );

#endif