#include "management.h"

char* listActionEnCours = "0,1,2,3,4,5,6";

void gestionAction(Action* listAction, Robot* robot)
{

}

void updateActionEnCours(Action* listAction, int nbAction)
{
  // Returns first token
  char* src;
  src = (char*)malloc((strlen(listActionEnCours)+1)*sizeof(char));

  strcpy(src,listActionEnCours);
  char *token = strtok(src, ",");

  // Keep printing tokens while one of the
  // delimiters present in str[].
  while (token != NULL)
  {

    printf("type : %d numero : %d : listFils : %s\n", listAction[atoi(token)].type, listAction[atoi(token)].numero, listAction[atoi(token)].listFils);
    token = strtok(NULL, ",");
  }
  printf("\n\n\n\n");
  printf("%d\n",getIndiceActionByIndice(listAction,1013,nbAction));
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
