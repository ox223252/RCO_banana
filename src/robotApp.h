#ifndef ROBOTAPP_H
#define ROBOTAPP_H

#include <stdio.h>
#include <stdlib.h>
#include "lib/roboclaw/roboclaw.h"
#include "lib/parserXML/loadXML.h"
// #include "deplacement/odometrie.h"
#include "struct/structRobot.h"
#include "struct/structAction.h"
#include <signal.h>

void launch();
void init();

#endif //ROBOTAPP_H
