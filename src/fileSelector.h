
#ifndef __FILESELECTOR_H__
#define __FILESELECTOR_H__

typedef struct
{
	char name[32];
	char title[32];
	bool active;
	bool exist;
}colorSelect_t;

int fileSelect ( char path[128], const uint8_t nb, colorSelect_t * const color );

#endif