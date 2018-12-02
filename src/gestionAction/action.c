#include "action.h"

static int port_Num;

static bool _action_armDesabled = false;

void setArmDesabledState ( bool disabled )
{
	_action_armDesabled = disabled;
}

int isDone ( Action* act )
{
	return act->isDone;
}

void setPortNum ( int portNum )
{
	port_Num = portNum;
}

int setVitesseDyna ( int id, int vitesse )
{
	uint8_t dxl_error = 0;
	int dxl_comm_result = COMM_TX_FAIL;						 // Communication result
	
	if ( _action_armDesabled )
	{
		return ( 0 );
	}

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
	
	if ( _action_armDesabled )
	{
		return ( 0 );
	}

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
	
	if ( _action_armDesabled )
	{
		return ( 0 );
	}

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
