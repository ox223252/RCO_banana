#include "asservissementVitesse.h"

static float coeffPG = 1.;
static float coeffIG = 0.;
static float coeffDG = 0.;
static float coeffPD = 1.;
static float coeffID = 0.;
static float coeffDD = 0.;
static int16_t maxSpeed = 1500;

static int16_t erreurPreGauche = -1000;
static int16_t sommeErreurGauche = 0;
static int16_t erreurPreDroite = -1000;
static int16_t sommeErreurDroite = 0;

void razAsserv()
{
  erreurPreGauche   = -1000;
  erreurPreDroite   = -1000;
  sommeErreurGauche = 0;
  sommeErreurDroite = 0;
}

void initAsservissementVitesse(float _coeffPG, float _coeffIG, float _coeffDG,int16_t _maxSpeed,float _coeffPD, float _coeffID, float _coeffDD)
{
  coeffPG = _coeffPG;
  coeffIG = _coeffIG;
  coeffDG = _coeffDG;
  coeffPD = _coeffPD;
  coeffID = _coeffID;
  coeffDD = _coeffDD;
  maxSpeed = _maxSpeed;
}

int asservirVitesseGaucheDroite(int16_t consigneGauche, int16_t consigneDroite, int16_t vitesseGauche, int16_t vitesseDroite)
{
  int16_t diffErreurGauche;
  int16_t diffErreurDroite;
  int16_t vitesseGToSend = 0;
  int16_t vitesseDToSend = 0;
  int16_t erreurGauche = consigneGauche - vitesseGauche;
  int16_t erreurDroite = consigneDroite - vitesseDroite;

  if(erreurPreGauche != -1000 && erreurPreDroite != -1000)
  {
    diffErreurGauche = erreurGauche - erreurPreGauche;
    diffErreurDroite = erreurDroite - erreurPreDroite;
  }else
  {
    diffErreurGauche = 0;
    diffErreurDroite = 0;
  }

  erreurPreDroite = erreurDroite;
  erreurPreGauche = erreurGauche;

  sommeErreurGauche += erreurGauche;
  sommeErreurDroite += erreurDroite;

  vitesseGToSend = coeffPG * erreurGauche + coeffIG * sommeErreurGauche + coeffDG * diffErreurGauche;
  vitesseDToSend = coeffPD * erreurGauche + coeffID * sommeErreurGauche + coeffDD * diffErreurGauche;

  return envoiOrdreMoteur ( vitesseGToSend, vitesseDToSend, maxSpeed );
}
