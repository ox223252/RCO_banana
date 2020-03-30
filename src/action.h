#ifndef __ACTION_H__
#define __ACTION_H__

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
/// \file action.h
/// \brief action manager file
/// \copyright GPLv2
/// \warning work in progress
////////////////////////////////////////////////////////////////////////////////

#include <stdint.h>

#include "struct/structRobot.h"
#define aT(type) ACTION_TYPE_##type

enum {
	aT(none),
	aT(servo),
	aT(pause),
	aT(get_var),
	aT(set_var),
	aT(set_dyna),
	aT(get_dyna),
	aT(timeout),
	aT(start),
	aT(sequence),
	aT(position),
	aT(orientation),
	aT(stopMove),
	aT(blocked),
	aT(pick),
	aT(place),
	aT(move),
	aT(getGpio),
	aT(setGpio),
	aT(setPasAPas),
	aT(end),
	aT(last)
};

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerInit ( const char * const file );
/// \param [in] file : file name that contain the json
/// \brief parse the json and create needed elements
/// \return 0 if Ok, else see errno for more details
////////////////////////////////////////////////////////////////////////////////
int actionManagerInit ( const char * const file , Robot* robot);

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerDeInit ( void );
/// \param [in] file : file name that contain the json
/// \brief free all elements created by action* functions
/// \return 0 if Ok, else see errno for more details
////////////////////////////////////////////////////////////////////////////////
int actionManagerDeInit ( void );



////////////////////////////////////////////////////////////////////////////////
/// \fn int actionStartStep ( void );
/// \brief get Next step to treat and init the FIRST action named "Départ"
/// \return > 0 the current step, < 0 see errno for mer details
////////////////////////////////////////////////////////////////////////////////
int actionStartStep ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerUpdate ( void );
/// \brief manage actions / timeouts / blocking actions
/// \return 0
////////////////////////////////////////////////////////////////////////////////
int actionManagerUpdate ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerCurrentIndex ( void );
/// \biref return a value that change on each change of the environement
////////////////////////////////////////////////////////////////////////////////
uint32_t actionManagerCurrentIndex ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerCurrentNumber ( uint32_t step );
/// \param [in] step : step for what we want to know size ( provided by
///     actionStartStep )
/// \retrun the number of action remainig for the deffined step
////////////////////////////////////////////////////////////////////////////////
int actionManagerCurrentNumber ( const uint32_t step );

////////////////////////////////////////////////////////////////////////////////
/// \fn void actionManagerSetFd ( const int mcpFd, const int pcaFd,
///     const int dynaFd );
/// \param [ in ] mcpFd
/// \param [ in ] pcaFd
/// \param [ in ] dynaFd
////////////////////////////////////////////////////////////////////////////////
void actionManagerSetFd ( const int mcpFd, const int pcaFd, const int dynaFd );

////////////////////////////////////////////////////////////////////////////////
/// \fn int actionManagerExec ( void );
/// \biref make the acts for the currents actions actives
/// \retrun 0 ok, else see errno
////////////////////////////////////////////////////////////////////////////////
int actionManagerExec ( void );



////////////////////////////////////////////////////////////////////////////////
/// \fn void actionManagerPrint ( void );
/// \brief print the full json action
////////////////////////////////////////////////////////////////////////////////
void actionManagerPrint ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn void actionManagerPrintCurrent ( void );
/// \brief print the current actions actives
////////////////////////////////////////////////////////////////////////////////
void actionManagerPrintCurrent ( void );

////////////////////////////////////////////////////////////////////////////////
/// \fn void actionManagerPrintEnv ( void );
/// \brief print the current env variables
////////////////////////////////////////////////////////////////////////////////
void actionManagerPrintEnv ( void );

#endif
