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

#include "actionDyna.h"

#include "../lib/dynamixel_sdk/dynamixel_sdk.h"
#include "../lib/log/log.h"

static int port_Num;

void setPortNum ( int portNum )
{
	port_Num = portNum;
}

int setVitesseDyna ( int id, int vitesse )
{
	uint8_t dxl_error = 0;
	int dxl_comm_result = COMM_TX_FAIL;						 // Communication result

	write2ByteTxRx ( port_Num, PROTOCOL_VERSION, id, ADDR_MX_MOVING_SPEED, vitesse );
	if ( ( dxl_comm_result = getLastTxRxResult ( port_Num, PROTOCOL_VERSION ) ) != COMM_SUCCESS )
	{
		logDebug ( "%s\n", getTxRxResult ( PROTOCOL_VERSION, dxl_comm_result ) );
		return ( __LINE__ );
	}
	else if ( ( dxl_error = getLastRxPacketError ( port_Num, PROTOCOL_VERSION ) ) != 0)
	{
		logDebug ( "%s\n", getRxPacketError ( PROTOCOL_VERSION, dxl_error ) );
		return ( __LINE__ );
	}
	return ( 0 );
}

int setPositionDyna ( int id, int position )
{
	uint8_t dxl_error = 0;
	int dxl_comm_result = COMM_TX_FAIL;						 // Communication result

	write2ByteTxRx ( port_Num, PROTOCOL_VERSION, id, ADDR_MX_GOAL_POSITION, position );
	if ( ( dxl_comm_result = getLastTxRxResult ( port_Num, PROTOCOL_VERSION ) ) != COMM_SUCCESS )
	{
		logDebug ( "%s\n", getTxRxResult ( PROTOCOL_VERSION, dxl_comm_result ) );
		return ( __LINE__ );
	}
	else if ( ( dxl_error = getLastRxPacketError ( port_Num, PROTOCOL_VERSION ) ) != 0 )
	{
		logDebug ( "%s\n", getRxPacketError ( PROTOCOL_VERSION, dxl_error ) );
		return ( __LINE__ );
	}
	return ( 0 );
}

int getPositionDyna ( int id )
{
	uint16_t dxl_present_position = 0;
	uint8_t dxl_error = 0;
	int dxl_comm_result = COMM_TX_FAIL;						 // Communication result

	dxl_present_position = read2ByteTxRx ( port_Num, PROTOCOL_VERSION, id, ADDR_MX_PRESENT_POSITION );

	if ( ( dxl_comm_result = getLastTxRxResult ( port_Num, PROTOCOL_VERSION ) ) != COMM_SUCCESS )
	{
		logDebug ( "%s\n", getTxRxResult ( PROTOCOL_VERSION, dxl_comm_result ) );
		return -1;
	}
	else if ( ( dxl_error = getLastRxPacketError ( port_Num, PROTOCOL_VERSION ) ) != 0 )
	{
		logDebug ( "%s\n", getRxPacketError ( PROTOCOL_VERSION, dxl_error ) );
		return -1;
	}
	return ( dxl_present_position );
}
