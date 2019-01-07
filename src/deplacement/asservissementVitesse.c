#include "asservissementVitesse.h"

static float _asservissementVitesse_coeffPG = 1.;
static float _asservissementVitesse_coeffIG = 0.;
static float _asservissementVitesse_coeffDG = 0.;
static float _asservissementVitesse_coeffPD = 1.;
static float _asservissementVitesse_coeffID = 0.;
static float _asservissementVitesse_coeffDD = 0.;
static int16_t _asservissementVitesse_maxSpeed = 800;

static int16_t _asservissementVitesse_erreurPreGauche = -1000;
static long _asservissementVitesse_sommeErreurGauche = 0;
static int16_t _asservissementVitesse_erreurPreDroite = -1000;
static long _asservissementVitesse_sommeErreurDroite = 0;

void razAsserv()
{
  _asservissementVitesse_erreurPreGauche   = -1000;
  _asservissementVitesse_erreurPreDroite   = -1000;
  _asservissementVitesse_sommeErreurGauche = 0;
  _asservissementVitesse_sommeErreurDroite = 0;
}

void initAsservissementVitesse(const float _coeffPG,const float _coeffIG,const float _coeffDG,const int16_t _maxSpeed,const float _coeffPD,const float _coeffID,const float _coeffDD)
{
  _asservissementVitesse_coeffPG = _coeffPG;
  _asservissementVitesse_coeffIG = _coeffIG;
  _asservissementVitesse_coeffDG = _coeffDG;
  _asservissementVitesse_coeffPD = _coeffPD;
  _asservissementVitesse_coeffID = _coeffID;
  _asservissementVitesse_coeffDD = _coeffDD;
  _asservissementVitesse_maxSpeed = _maxSpeed;
}

int asservirVitesseGaucheDroite(const int16_t consigneGauche,const int16_t consigneDroite,const int16_t vitesseGauche,const int16_t vitesseDroite)
{
  int16_t diffErreurGauche;
  int16_t diffErreurDroite;
  int16_t vitesseGToSend = 0;
  int16_t vitesseDToSend = 0;
  int16_t erreurGauche = consigneGauche - vitesseGauche;
  int16_t erreurDroite = consigneDroite - vitesseDroite;

  if(_asservissementVitesse_erreurPreGauche != -1000 &&
     _asservissementVitesse_erreurPreDroite != -1000)
  {
    diffErreurGauche = erreurGauche - _asservissementVitesse_erreurPreGauche;
    diffErreurDroite = erreurDroite - _asservissementVitesse_erreurPreDroite;
  }else
  {
    diffErreurGauche = 0;
    diffErreurDroite = 0;
  }

  _asservissementVitesse_erreurPreDroite = erreurDroite;
  _asservissementVitesse_erreurPreGauche = erreurGauche;

  _asservissementVitesse_sommeErreurGauche += erreurGauche;
  _asservissementVitesse_sommeErreurDroite += erreurDroite;

  vitesseGToSend =  _asservissementVitesse_coeffPG * erreurGauche +
                    _asservissementVitesse_coeffIG * _asservissementVitesse_sommeErreurGauche +
                    _asservissementVitesse_coeffDG * diffErreurGauche;

  vitesseDToSend =  _asservissementVitesse_coeffPD * erreurDroite +
                    _asservissementVitesse_coeffID * _asservissementVitesse_sommeErreurDroite +
                    _asservissementVitesse_coeffDD * diffErreurDroite;

printf("erreur %ld somme %ld\n",erreurGauche,_asservissementVitesse_sommeErreurGauche);
  return envoiOrdreMoteur ( vitesseGToSend, vitesseDToSend, _asservissementVitesse_maxSpeed );
}
