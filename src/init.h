#ifndef __INIT_H__
#define __INIT_H__

#include <stdint.h>

int initMCP23017 ( const char * const i2c, const int addr, int * const fd );
int initPAC9685 ( const char * const i2c, const int addr, int * const fd );
int initDyna ( const char * const addr, long int * const fd, const uint32_t speed );

#endif