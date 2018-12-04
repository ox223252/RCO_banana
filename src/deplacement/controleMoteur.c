#include <stdio.h>

#include "controleMoteur.h"

void envoiOrdreMoteur ( struct roboclaw* rc, Robot* robot, int16_t limitSpeed )
{
	int16_t vitesseToSendG = -1 * robot->vitesseGaucheToSend / 1500 * 32767;
	int16_t vitesseToSendD = -1 * robot->vitesseDroiteToSend / 1500 * 32767;

	if ( vitesseToSendG > limitSpeed )
	{
		vitesseToSendG = limitSpeed;
	}
	if ( vitesseToSendG < -limitSpeed )
	{
		vitesseToSendG = -limitSpeed;
	}
	if ( vitesseToSendD > limitSpeed )
	{
		vitesseToSendD = limitSpeed;
	}
	if ( vitesseToSendD < -limitSpeed )
	{
		vitesseToSendD = -limitSpeed;
	}

	if ( roboclaw_duty_m1m2 ( rc, 0x80, vitesseToSendG,vitesseToSendD) != ROBOCLAW_OK )
	{
		printf ( "Send moteur failed !!!!!!! \n" );
	}
}
