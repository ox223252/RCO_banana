#ifndef ACTION_H
#define ACTION_H

#include "../struct/structAction.h"
#include "../struct/structRobot.h"

#include "../lib/dynamixel_sdk/dynamixel_sdk.h"
#include "../lib/log/log.h"
// Control table address
#define ADDR_MX_TORQUE_ENABLE           24                  // Control table address is different in Dynamixel model
#define ADDR_MX_GOAL_POSITION           30
#define ADDR_MX_MOVING_SPEED            32
#define ADDR_MX_PRESENT_POSITION        36

// Protocol version
#define PROTOCOL_VERSION                1.0                 // See which protocol version is used in the Dynamixel

void setArmDesabledState ( bool disabled );
int isDone(Action* act);

void setPortNum(int portNum);
int setVitesseDyna(int id, int vitesse);
int setPositionDyna(int id, int position);
int getPositionDyna(int id);




#endif //ACTION_H
