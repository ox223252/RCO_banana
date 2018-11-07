#include "management.h"

char* listActionEnCours = "0";

void gestionAction(Action* listAction, Robot* robot)
{

}

void updateActionEnCours(Action* listAction)
{
  // Returns first token
  char src[100];
  strcpy(src,listActionEnCours);
    char *token = strtok(src, ",");

    // Keep printing tokens while one of the
    // delimiters present in str[].
    while (token != NULL)
    {
        printf("%s\n", token);
        token = strtok(NULL, ",");
    }
}
