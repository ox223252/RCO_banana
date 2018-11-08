#ifndef STRUCTACTION_H
#define STRUCTACTION_H

#include <stdint.h> //uint8_t, int16_t
typedef enum
{
  TYPE_SERVO              =     0,
  TYPE_DYNA               =     1,
  TYPE_CAPTEUR            =     2,
  TYPE_MOTEUR             =     3,
  TYPE_AUTRE              =     4,
  TYPE_POSITION           =     5,
  TYPE_ORIENTATION        =     6,
  TYPE_SEQUENCE           =     7,
  TYPE_ENTREE             =     9,
  TYPE_ATTENTE_SERVO      =     10,
  TYPE_ATTENTE_DYNA       =     11,
  TYPE_ATTENTE_TEMPS      =     12,
  TYPE_RETOUR_DEPLACEMENT =     13,
  TYPE_RETOUR_ORIENTATION =     14,
  TYPE_RETOUR_POSITION    =     15,
  TYPE_GPIO               =     16,
  TYPE_RETOUR_GPIO        =     17,
  TYPE_AND                =     18,
  TYPE_SET_VALEUR         =     19,
  TYPE_COURBE             =     20,
  TYPE_ATTENTE_BLOAGE     =     21,
  TYPE_DEPLACEMENT        =     22,
  TYPE_FIN                =     23,
  TYPE_SET_VARIABLE       =     24,
  TYPE_GET_VARIABLE       =     25


}
typeAction;

typedef struct
{
  typeAction type;
  int numero;
  int timeout;
  char* listPere;
  char* listFils;
  char* listTimeOut;

  uint8_t isDone;
  char** params;
  uint64_t heureCreation;
}Action;
#endif //STRUCTACTION_H
