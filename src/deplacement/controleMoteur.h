#ifndef __CONTROLEMOTEUR_H__
#define __CONTROLEMOTEUR_H__

#include <stdint.h>
#include <stdbool.h>

#include "../lib/roboclaw/roboclaw.h"
#include "../struct/structRobot.h"

////////////////////////////////////////////////////////////////////////////////
/// \fn int initEngine ( const char * restrict const path, const uint32_t speed,
///     const float maxVoltage, const float minVolatge, const uint32_t delay,
///     struct roboclaw ** const ptr );
/// \param[ in ] path : path to the device /dev/ttyUSBx
/// \param[ in ] speed : bauderate of the uart
/// \param[ in ] maxVoltage : volateg limite for usage
/// \param[ in ] minVolatge : volateg limite for usage
/// \param[ in ] delay : minimum delay between two request too battery volatge
/// \param[ out ] ptr : pointer on roboClaw struc if needed outside of this
///     functions
////////////////////////////////////////////////////////////////////////////////
int initEngine ( const char * restrict const path, const uint32_t speed,
	const float maxVoltage, const float minVolatge, const uint32_t delay,
	struct roboclaw ** const ptr );

////////////////////////////////////////////////////////////////////////////////
/// \fn float getBattery ( void );
/// \return battery value
////////////////////////////////////////////////////////////////////////////////
float getBattery ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn int envoiOrdreMoteur ( int16_t left, int16_t right, int16_t limitSpeed);
////////////////////////////////////////////////////////////////////////////////
int envoiOrdreMoteur ( int16_t left, int16_t right, int16_t limitSpeed );

////////////////////////////////////////////////////////////////////////////////
/// \fn void initBoost ( const float voltage, const uint32_t delay );
/// \param[ in ] voltage : boost voltage level
/// \param[ in ] delay : boost duration
////////////////////////////////////////////////////////////////////////////////
void initBoost ( const float voltage, const uint32_t delay );

////////////////////////////////////////////////////////////////////////////////
/// \fn int requestBoost ( void );
/// \brief request a boost during, predefined delay
////////////////////////////////////////////////////////////////////////////////
int requestBoost ( bool flag );

int envoiOrdrePositionMoteurs(int accel_m1, int speed_m1, int decel_m1, int position_m1, int accel_m2, int speed_m2, int decel_m2, int position_m2);

#endif //__CONTROLEMOTEUR_H__
