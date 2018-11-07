#include "management.h"

char* listActionEnCours = "0;";
int nbActionEnCours = 1;
void gestionAction(Action* listAction, Robot* robot)
{

}

void updateActionEnCours(Action* listAction, int nbAction)
{
  // Returns first token
  char* src;
  char* listCOPY;
  char* listFilsCOPY;
  char copyNumber[10];
  int nbBuffer;

  src = (char*)malloc((strlen(listActionEnCours)+1)*sizeof(char));
  listCOPY = (char*)malloc((strlen(listActionEnCours)+1)*sizeof(char));
  int indiceToken = 0;
  strcpy(src,listActionEnCours);
  strcpy(listCOPY,listActionEnCours);
  char *token = strtok(src, ";");

  // Keep printing tokens while one of the
  // delimiters present in str[].
  while (token != NULL)
  {
    if(isDone(&(listAction[atoi(token)]))==1)
    {
      printf("Avant %s \n",listCOPY);
      listFilsCOPY = (char*)malloc((strlen(listAction[atoi(token)].listFils)+1)*sizeof(char));
      strcpy(listFilsCOPY,listAction[atoi(token)].listFils);

      char *tokenBis = strtok(listFilsCOPY, ";");
      while (tokenBis != NULL)
      {

        listCOPY = (char *) realloc(listCOPY, strlen(listCOPY)+1);
        sscanf(tokenBis, "%d", &nbBuffer);

        printf("Token bis : %s %d \n",tokenBis, getIndiceActionByIndice(listAction, nbBuffer,nbAction));
        sprintf(copyNumber, "%d", getIndiceActionByIndice(listAction, nbBuffer,nbAction));


        strcat(listCOPY, copyNumber);
        strcat(listCOPY, ";");
        nbActionEnCours++;
        tokenBis = strtok(NULL, ";");
      }

      listCOPY = (char *) realloc(listCOPY, strlen(listCOPY)+strlen(listAction[atoi(token)].listFils));
      strcat(listCOPY, listAction[atoi(token)].listFils);

      for(int i=2*indiceToken+1;i<strlen(listCOPY)-2;i++)
      {
        listCOPY[i]=listCOPY[i+1];
      }

      indiceToken--;
      nbActionEnCours--;

      printf("Apres %s %d\n",listCOPY,nbActionEnCours);
    }
    indiceToken++;
    token = strtok(NULL, ";");
  }
  printf("\n\n\n\n");


  free(listCOPY);
  free(src);
  free(listFilsCOPY);
}

int getIndiceActionByIndice(Action* listAction, int indiceAction, int nbAction)
{
  for(int i=0;i<nbAction;i++)
  {
    if(listAction[i].numero == indiceAction)
    {
      return i;
    }
  }
  return -1;
}
