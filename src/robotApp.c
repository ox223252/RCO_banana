// #include "robotApp.h"
// #include <unistd.h>

// //Drapeau d'arrêt du programme. Sa valeur d'origine est 0.
// unsigned short stop = 0;
// roboclaw *rc;
// Robot robot1;
// Action* tabActionTotal;
// int nbAction;

// void launch()
// {
//   init();
//   while(!stop)
//   {
//     //calculPosition(rc,&robot1);
//     //printf("%f %f %f %d %d\n",robot1.xRobot,robot1.yRobot,robot1.orientationRobot,robot1.codeurGauche, robot1.codeurDroit);
//     usleep(1000*500);
//   }
//   printf("Good bye \n");
// }



// /***********************************
// * Fonction de reception des signaux.
// * Si le signal reçu est SIGINT Ctl^c, alors on increment le nombre de signaux de ce type reçu.
// * Quand ce nombre est egal a 5 on met stop à 1.
// ************************************/
// void signalHandler(int signal){
//   if(signal==SIGINT){
//     stop=1;
//   }
// }


// /*
// Ici, on va initialiser tout ce qui doit être initialiser : fichier, structure... TOUT.
// On met tout au même endroit, ça nous évitera de chercher partout si on en a pas raté une.

// init 1 : Initialisation du signal CTRL+C
// init 2 : Roboclaw
// init 3 : Odom
// */
// void init()
// {

//   //Init 1 :

//   //Structure pour l'enregistrement d'une action déclenchée lors de la reception d'un signal.
//   struct sigaction action, oldAction;

//   action.sa_handler = signalHandler;	//La fonction qui sera appellé est signalHandler

//   //On vide la liste des signaux bloqués pendant le traitement
//   //C'est à dire que si n'importe quel signal (à part celui qui est traité)
//   //est declenché pendant le traitement, ce traitement sera pris en compte.
//   sigemptyset(&action.sa_mask);

//   //Redémarrage automatique Des appels systêmes lents interrompus par l'arrivée du signal.
//   action.sa_flags = SA_RESTART;

//   //Mise en place de l'action pour le signal SIGINT, l'ancienne action est sauvegardé dans oldAction
//   sigaction(SIGINT, &action, &oldAction);

//   //initialize at supplied terminal at specified baudrate
//   /*rc=roboclaw_init("/dev/ttyACM0", 115200);
//   if( rc == NULL )
//   {
//     perror("unable to initialize roboclaw");
//     exit(1);
//   }
//   robot1.xRobot = 0.;
//   robot1.yRobot = 0.;
//   robot1.orientationRobot = 0.;
//   initOdometrie(rc,&robot1);*/
//   tabActionTotal = ouvrirXML(&nbAction);

//   printf("Nb Action : %d %d\n",nbAction,tabActionTotal[28].type);
// }
