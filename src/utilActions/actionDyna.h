#ifndef __ACTION_DYNA_H__
#define __ACTION_DYNA_H__

#include "../lib/dynamixel_sdk/dynamixel_sdk.h"
#include "../lib/log/log.h"

// Control table address
#define ADDR_MX_TORQUE_ENABLE           24                  // Control table address is different in Dynamixel model
#define ADDR_MX_GOAL_POSITION           30
#define ADDR_MX_MOVING_SPEED            32
#define ADDR_MX_PRESENT_POSITION        36

// Protocol version
#define PROTOCOL_VERSION                1.0

void setPortNum(int portNum);
int setVitesseDyna(int id, int vitesse);
int setPositionDyna(int id, int position);
int getPositionDyna(int id);

#endif
