#include "action.h"

int port_Num;

int isDone(Action* act)
{
  return act->isDone;
}

void setPortNum(int portNum)
{
  port_Num = portNum;
}

int setVitesseDyna(int id, int vitesse)
{
  uint8_t dxl_error = 0;
  int dxl_comm_result = COMM_TX_FAIL;             // Communication result


  write2ByteTxRx(port_Num, PROTOCOL_VERSION, id, ADDR_MX_MOVING_SPEED, vitesse);
  if ((dxl_comm_result = getLastTxRxResult(port_Num, PROTOCOL_VERSION)) != COMM_SUCCESS)
  {
    printf("%s\n", getTxRxResult(PROTOCOL_VERSION, dxl_comm_result));
    return -1;
  }
  else if ((dxl_error = getLastRxPacketError(port_Num, PROTOCOL_VERSION)) != 0)
  {
    printf("%s\n", getRxPacketError(PROTOCOL_VERSION, dxl_error));
    return -1;
  }
  return 1;
}

int setPositionDyna(int id, int position)
{
  uint8_t dxl_error = 0;
  int dxl_comm_result = COMM_TX_FAIL;             // Communication result

  printf("\nangle : %d\n\n",position);
  write2ByteTxRx(port_Num, PROTOCOL_VERSION, id, ADDR_MX_GOAL_POSITION, position);
  if ((dxl_comm_result = getLastTxRxResult(port_Num, PROTOCOL_VERSION)) != COMM_SUCCESS)
  {
    printf("%s\n", getTxRxResult(PROTOCOL_VERSION, dxl_comm_result));
    return -1;
  }
  else if ((dxl_error = getLastRxPacketError(port_Num, PROTOCOL_VERSION)) != 0)
  {
    printf("%s\n", getRxPacketError(PROTOCOL_VERSION, dxl_error));
    return -1;
  }
  return 1;
}

int getPositionDyna(int id)
{
  uint16_t dxl_present_position = read2ByteTxRx(port_Num, PROTOCOL_VERSION, id, ADDR_MX_PRESENT_POSITION);
  uint8_t dxl_error = 0;
  int dxl_comm_result = COMM_TX_FAIL;             // Communication result

  if ((dxl_comm_result = getLastTxRxResult(port_Num, PROTOCOL_VERSION)) != COMM_SUCCESS)
  {
    printf("%s\n", getTxRxResult(PROTOCOL_VERSION, dxl_comm_result));
    return -1;
  }
  else if ((dxl_error = getLastRxPacketError(port_Num, PROTOCOL_VERSION)) != 0)
  {
    printf("%s\n", getRxPacketError(PROTOCOL_VERSION, dxl_error));
    return -1;
  }
  return dxl_present_position;
}
