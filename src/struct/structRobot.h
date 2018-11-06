#ifndef STRUCTROBOT_H
#define STRUCTROBOT_H

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

}Robot;
#endif //STRUCTROBOT_H
