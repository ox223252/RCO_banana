#ifndef __ACTION_DYNA_H__
#define __ACTION_DYNA_H__

////////////////////////////////////////////////////////////////////////////////
/// \copiright RCO, 2019
///
/// This program is free software: you can redistribute it and/or modify it
///     under the terms of the GNU General Public License published by the Free
///     Software Foundation, either version 2 of the License, or (at your
///     option) any later version.
///
/// This program is distributed in the hope that it will be useful, but WITHOUT
///     ANY WARRANTY; without even the implied of MERCHANTABILITY or FITNESS FOR
///     A PARTICULAR PURPOSE. See the GNU General Public License for more
///     details.
///
/// You should have received a copy of the GNU General Public License along with
///     this program. If not, see <http://www.gnu.org/licenses/>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \file actionDyna.h
/// \brief action manager utility for dynamixels
/// \copyright GPLv2
/// \warning work in progress
////////////////////////////////////////////////////////////////////////////////

// Control table address
// Control table address is different in Dynamixel model
static const int ADDR_MX_TORQUE_ENABLE = 24; 
static const int ADDR_MX_GOAL_POSITION = 30;
static const int ADDR_MX_MOVING_SPEED = 32;
static const int ADDR_MX_PRESENT_POSITION = 36;

// Protocol version
static const int PROTOCOL_VERSION = 1.0;

//TODO add comments
void setPortNum(int portNum);
int setVitesseDyna(int id, int vitesse);
int setPositionDyna(int id, int position);
int getPositionDyna(int id);

#endif
