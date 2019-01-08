#include "detectionBlocage.h"

static long* tabErreurGauche;
static long* tabErreurDroite;
static int indiceTab;

void initDetectionBlocage()
{
  indiceTab = 0;
  tabErreurGauche = calloc(68,sizeof(long));
  tabErreurDroite = calloc(68,sizeof(long));
}

void resetBlocage()
{
  indiceTab = 0;
  tabErreurGauche = calloc(68,sizeof(long));
  tabErreurDroite = calloc(68,sizeof(long));
}

int detectBlocage(Robot* robot, int seuilDetection)
{
  float sommeErreurGauche=0,sommeErreurDroite=0;
  tabErreurGauche[indiceTab] = robot->vitesseGaucheToSend - robot->vitesseGauche;
  tabErreurDroite[indiceTab] = robot->vitesseDroiteToSend - robot->vitesseDroite;
  indiceTab++;
  if(indiceTab == 68)indiceTab=0;

  for(int i=0;i<68;i++)
  {
    sommeErreurGauche+=tabErreurGauche[i];
    sommeErreurDroite+=tabErreurGauche[i];
  }

  if(sommeErreurGauche > seuilDetection*robot->vitesseGaucheToSend ||
    sommeErreurDroite > seuilDetection*robot->vitesseDroiteToSend)
    {
      return 1;
    }
    return 0;
  }
