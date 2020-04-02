#ifndef __FILESELECTOR_H__
#define __FILESELECTOR_H__

////////////////////////////////////////////////////////////////////////////////
/// \copiright RCO, 2020
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
/// \file fileselector.h
/// \brief file selector manager, provide a function to manage the selection of
///     war plan
/// \copyright GPLv2
////////////////////////////////////////////////////////////////////////////////

/// \typedef colorSelect_t
/// \biref sturct used to defined available color in file selector for game plan
typedef struct
{
	char name[32]; ///< str in the name of file
	char title[32]; ///< str in the displayed menu
	bool active; ///< is the color selected
	bool exist; ///< is the file for this color exist
}colorSelect_t;

////////////////////////////////////////////////////////////////////////////////
/// \fn int fileSelect ( char path[128], const uint8_t nb, 
///     colorSelect_t * const color );
/// \param [in/out] path : of the file opened
/// \param [in] nb : nb of color in the color array
/// \param [in] color : table of available colors
/// \biref test if file with path exist or file-color[x].json exist, in the 
///     second case will display a menu anser to select the right file
/// \return 0 if ok else see errno
////////////////////////////////////////////////////////////////////////////////
int fileSelect ( char path[128], const uint8_t nb, colorSelect_t * const color );

#endif