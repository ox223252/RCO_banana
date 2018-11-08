#ifndef __STRUCTROBOT_H__
#define __STRUCTROBOT_H__

#include <stdint.h> //uint8_t, int16_t

typedef struct Robot
{
	int32_t codeurGauche;
	int32_t codeurDroit;

	float vitesseGauche;
	float vitesseDroite;

	float xRobot;
	float yRobot;
	float orientationRobot;

	float distanceParcourue;

	float coeffLongG;
	float coeffLongD;
	float coeffAngleG;
	float coeffAngleD;
}
Robot;

typedef struct
{
	uint8_t noArm:1,  // 0x01
		armWait:1,    // 0x02
		armScan:1,    // 0x04
		armDone:1,    // 0x08
		noDrive:1,    // 0x10
		driveWait:1,  // 0x20
		driveScan:1,  // 0x40
		driveDone:8;  // 0x80
}
ActionFlag;
#endif //STRUCTROBOT_H
