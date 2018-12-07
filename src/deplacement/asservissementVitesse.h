
#ifndef __ASSERVISSEMENTVITESSE_H__
#define __ASSERVISSEMENTVITESSE_H__

#include <stdint.h>
#include <stdio.h>
#include "controleMoteur.h"

int asservirVitesseGaucheDroite(int16_t consigneGauche, int16_t consigneDroite, int16_t vitesseGauche, int16_t vitesseDroite);
void initAsservissementVitesse(float _coeffPG, float _coeffIG, float _coeffDG,int16_t _maxSpeed,float _coeffPD, float _coeffID, float _coeffDD);
#endif //__ASSERVISSEMENTVITESSE_H__
