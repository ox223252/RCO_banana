#include <stdlib.h>

#include "detectionBlocage.h"

static long tabVitesseGauche[200];
static long tabVitesseDroite[200];
static int indiceTab;

void initDetectionBlocage()
{
  indiceTab = 0;
  for(int i=0;i<200;i++)
  {
    tabVitesseGauche[i]=200;
    tabVitesseDroite[i]=200;
  }
}

void resetBlocage()
{
  indiceTab = 0;
  for(int i=0;i<200;i++)
  {
    tabVitesseGauche[i]=200;
    tabVitesseDroite[i]=200;
  }
}

int detectBlocage(Robot* robot, int seuilDetection)
{
  tabVitesseGauche[indiceTab] = robot->vitesseGauche;
  tabVitesseDroite[indiceTab] = robot->vitesseDroite;
  indiceTab++;
  int indiceToCheck;
  if(indiceTab == 200)indiceTab=0;

  for(int i=0;i<seuilDetection;i++)
  {
    indiceToCheck = indiceTab - i;
    if(indiceToCheck<0)indiceToCheck+=200;

    if(robot->vitesseDroiteToSend == 0 || tabVitesseDroite[indiceToCheck] > 20
        || robot->vitesseGaucheToSend == 0 || tabVitesseGauche[indiceToCheck] > 20)
    {
      resetBlocage();
      return 0;
    }
  }  
    return 1;
}
