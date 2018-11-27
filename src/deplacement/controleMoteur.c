#include <stdio.h>

#include "controleMoteur.h"

void envoiOrdreMoteur ( struct roboclaw* rc, Robot* robot )
{
	if ( roboclaw_speed_m1m2 ( rc,0x80, robot->vitesseGaucheToSend, robot->vitesseDroiteToSend ) != ROBOCLAW_OK )
	{
		printf ( "Send moteur failed !!!!!!! \n" );
	}
}
