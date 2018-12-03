#include <stdio.h>

#include "controleMoteur.h"

void envoiOrdreMoteur ( struct roboclaw* rc, Robot* robot )
{
	int vitesseToSendG = -1*robot->vitesseGaucheToSend / 1500 * 32767;
	int vitesseToSendD = -1*robot->vitesseDroiteToSend / 1500 * 32767;

	if(vitesseToSendG > 32767)vitesseToSendG=32767;
	if(vitesseToSendG < -32767)vitesseToSendG=-32767;
	if(vitesseToSendD > 32767)vitesseToSendD=32767;
	if(vitesseToSendD < -32767)vitesseToSendD=-32767;
	// 1500 = 32767
	//x =
	if ( roboclaw_duty_m1m2 ( rc,0x80, vitesseToSendG,vitesseToSendD)  != ROBOCLAW_OK )
	{
		printf ( "Send moteur failed !!!!!!! \n" );
	}
}
