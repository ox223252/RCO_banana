#ifndef __ACTIONEXTRACTDEFINES_H__
#define __ACTIONEXTRACTDEFINES_H__

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
/// \file actionExtractDefines.h
/// \brief action manager utility defines, this file shoud be in accordance with
///     IHM json generator
/// \copyright GPLv2
////////////////////////////////////////////////////////////////////////////////

static const char CONDITINON[] = "condition";
static const char PARAMETER[] = "param";
static const char VALUE[] = "valeur";
static const char RATE[] = "taux";
static const char RATE_ARRAY[] = "TauxArray";

typedef enum {
	UT, // up than
	EQ, // equal
	LT // less than
}
condition_t;

typedef enum {
	TIME,
	CUPS
}
param_t;

#endif