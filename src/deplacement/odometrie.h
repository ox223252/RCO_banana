#ifndef __ODOMETRIE_H__
#define __ODOMETRIE_H__

#include "../lib/roboclaw/roboclaw.h"
#include "../struct/structRobot.h"

////////////////////////////////////////////////////////////////////////////////
/// \fn int initOdometrie ( struct roboclaw* rc, Robot* robot );
/// \param[ in ] rc : roboclow bus descriptor
/// \param[ in/out ] robot : structure who contain robot env variables
/// \brief init robot struct
/// \retrun it 0 then OK else see errno for more details
////////////////////////////////////////////////////////////////////////////////
int initOdometrie ( struct roboclaw* rc, Robot* robot );

////////////////////////////////////////////////////////////////////////////////
/// \fn int calculPosition ( struct roboclaw* rc, Robot* robot );
/// \param[ in ] rc : roboclow bus descriptor
/// \param[ in/out ] robot : structure who contain robot env variables
/// \brief calc dX, dY, angle between previous position and now
/// \retrun it 0 then OK else see errno for more details
////////////////////////////////////////////////////////////////////////////////
int calculPosition ( struct roboclaw* rc, Robot* robot );

#endif
