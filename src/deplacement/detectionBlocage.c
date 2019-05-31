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
  printf("RESET TAMERE\n");
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
  int toRet = 1;
  if(indiceTab == 200)indiceTab=0;

  for(int i=1;i<seuilDetection/10;i++)
  {
    indiceToCheck = indiceTab - i;
    if(indiceToCheck<0)indiceToCheck+=200;

    if(robot->vitesseDroiteToSend == 0 || abs(tabVitesseDroite[indiceToCheck]) > 15
        || robot->vitesseGaucheToSend == 0 || abs(tabVitesseGauche[indiceToCheck]) > 15)
    {

      toRet=0;
    }
  }  
    return toRet;
}
