#ifndef __LIDAR_STRUCT_H__
#define __LIDAR_STRUCT_H__

#define DISTANCE_MIN ((float)200.0)

typedef enum
{
	DIR_NONE = 0x00,
	DIR_FORWARD = 0x01,
	DIR_BACKWARD = 0x02,
	DIR_LEFT = 0x04,
	DIR_RIGHT = 0x08
}
direction_e;

typedef struct
{
	direction_e dir;
	float distance;
}
detection_t;

#endif