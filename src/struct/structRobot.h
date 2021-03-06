#ifndef __STRUCTROBOT_H__
#define __STRUCTROBOT_H__

#include <stdint.h> //uint8_t, int16_t
#include "structDetection.h"
typedef struct
{
	float xCible;
	float yCible;
	float vitesseMax;
	float acc;
	float dec;
	float sens;
	float precision;
}
Point;


typedef struct Robot
{
	int32_t codeurGauche;
	int32_t codeurDroit;
	Point cible;
	uint8_t blocageVoulu;

	float vitesseGauche;
	float vitesseDroite;
	float vitesseAngulaire;

	float vitesseGaucheToSend;
	float vitesseDroiteToSend;

	float vitesseGaucheDefault;
	float vitesseDroiteDefault;

	float xRobot;
	float yRobot;
	float orientationRobot;

	float orientationVisee;

	float distanceParcourue;

	float setDetection;

	float coeffLongG;
	float coeffLongD;
	float coeffAngleG;
	float coeffAngleD;

	// shared memory used by lidar to get detection
	detection_t * detection;
	uint32_t memKey;
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
		driveDone:1;  // 0x80
}
ActionFlag;
#endif //STRUCTROBOT_H
