
# ![flag](res/imgs/fr.jpg) RCO_banana
Le programme de RCO pour nos robots

RCO est une équipe de robotique et de bidouilleurs qui à pour vocation de concourir à la coupe de France de robotique

## Le récupérer:
```Shell
> git clone --recursive https://github.com/ox223252/RCO_Banana
```

## Compiler:
### Première foi:
```Shell
> ./Configure/configure
> make
```

### Après:
```Sehll
> make
```

Par défaut la compilation est faite pour arm-Linux (pour raspberry), si vous voulez compile pour une autre architecture allez voir ce [github](https://github.com/ox223252/Configure)

## Utilisation:
Vous pouvez lancer le programme comme suit :
```Shell
> ./bin/Banana
```

Pour avoir de l'aide :
```Shell
> ./bin/Banana -h
build date: 2018.12.04 19:52:20

parameter available for cmd line:
    usage : key value value2 key2 value3 ...
            key : key :  nb elements : help or type
         --help :  -h : bool : this window
        --green :  -g : bool : launch the green prog
          --red :  -r : bool : launch the red prog
            --q :  -q : bool : hide all trace point
        --debug :  -d : bool : display many trace point
        --color :  -c : bool : add color to debug traces
         --term : -lT : bool : add color to debug traces
         --file : -lF : bool : add color to debug traces
        --noArm : -nA : bool : use it to disable servo motor
      --armWait : (null) : bool : wait end of timeout before set action to done
      --armScan : (null) : bool : wait a key pressed to action to done
      --armDone : (null) : bool : automaticaly set action to done (default)
      --noDrive : -nD : bool : use it to disable drive power
    --driveWait : (null) : bool : wait end of timeout before set action to done
    --driveScan : (null) : bool : wait a key pressed to action to done
    --driveDone : (null) : bool : automaticaly set action to done (default)
     --MaxSpeed : -Ms :    1 : set max speed [ 1 ; 32767 ]
          --ini :  -i :    1 : xml initialisation file path
          --xml :  -x :    1 : xml action file path
         --time :  -t :    1 : game duration in seconds
  --linear_left : -ll :    1 : linear coef for left wheel
 --linear_right : -lr :    1 : linear coef for right wheel
   --angle_left : -al :    1 : angular coef for right wheel
  --angle_right : -ar :    1 : angular coef for right wheel
         --Vmax : -vM :    1 : maximum voltage that should provide systeme to engine
         --Vmin : -vm :    1 : minimum voltage that should provide systeme to engine
       --Vboost : -vB :    1 : maximum voltage that should provide systeme to engine during boost mode
       --tBoost : -tB :    1 : maximum delay for boost mode


parameter available in res/config.rco file:
    usage : key=value key2=value2
                 key : help or type
           PATH_DYNA : PATH to access to dynamixels
    PATH_MOTOR_BOARD : PATH to access to dynamixels
PATH_MOTOR_BOARD_UART_SPEED : UART speed for robocloaw board
        PCA9695_ADDR : pca9685 board i2c addr
     XML_ACTION_PATH : xml initialisation file path
       XML_INIT_PATH : xml action file path
         GLOBAL_TIME : game duration in seconds
    COEF_LINEAR_LEFT : linear coef for left wheel
   COEF_LINEAR_RIGHT : linear coef for right wheel
     COEF_ANGLE_LEFT : angular coef for right wheel
    COEF_ANGLE_RIGHT : angular coef for right wheel
       BATTERY_DELAY : delay min between two read of battery delay during engin control
         VOLATGE_MAX : maximum voltage that should provide systeme too engine
         VOLATGE_MIN : minimum voltage that should provide systeme too engine
       BOOST_VOLTAGE : maximum voltage that should provide systeme to engine during boost mode
          BOOST_TIME : maximum delay for boost mode

```

## debug :
Pour debogger en cas de problème ( si le système est compilé en MODE_DEBUG ), il existe 5 options :
 - `-q` : qui désactive toutes les sorties de debug (s'il n'est pas appelé le niveau verbose est activé par défaut ),
 - `-d` : qui active le debug de niveau supérieur,
 - `-c` : qui active la couleur pour plus de lisibilité,
 - `-lT` : qui active la sortie dans le terminal ( actif par défaut si aucune autre n'est demandé ),
 - `-lF` : qui active la sortie dans un fichier de log `log.txt` peut etre couplé à -lT.

Pour le contrôle du robot les flags noDrive et noArm vont respectivement désactiver les moteurs et les actionneurs, ils peuvent être complété par :
 - `--xxxWait` : attend le timeout pour valider l'action
 - `--xxxScan` : attend un appuis sur le clavier pour valider la action
 - `--xxxDone` : valide l'action par défaut

`--MaxSpeed` : représente la vitesse max du robot en m/s

`--ini` : est un xml d'initialisation
`--xml` : est le xml rincipal celui qui doit durer maximum `--time`
`--time` : est le temps maximum que doit durer la partie

`--linear_left` : coefficient utile que pour la mise au point doit être placé dans le fichier de config ( `COEF_LINEAR_LEFT` ) par la suite.
`--linear_right` : coefficient utile que pour la mise au point doit être placé dans le fichier de config ( `COEF_LINEAR_RIGHT` ) par la suite.
`--angle_left` : coefficient utile que pour la mise au point doit être placé dans le fichier de config ( `COEF_ANGLE_LEFT` ) par la suite.
`--angle_right` : coefficient utile que pour la mise au point doit être placé dans le fichier de config ( `COEF_ANGLE_RIGHT` ) par la suite.

`--Vmax` : tension maximum à envoyer au moteurs
`--Vmin` : tension minimum de la batterie pour autoriser le contrôle des moteurs
`--Vboost` : tension maximum de boost
`--tBoost` : temps maximum de boost, la durée de boost est forcement inférieur ou égale au temps `BATTERY_DELAY`

Certain paramètres tels que la durée de jeux sont existent dans le fichier de config ainsi que pour la ligne de commande, la ligne de commande sera prioritaire sur la configuration du fichier.

## [RobotClaw lib](https://github.com/michaelrsweet/mxml)

## [Dynamixel lib](https://github.com/ROBOTIS-GIT/DynamixelSDK)

## [Compatibilité de licences](https://www.gnu.org/licenses/license-list.fr.html)




# ![flag](res/imgs/en.jpg) RCO_banana
RCO's programme for our robots

RCO is a team of robotics and hackers who participate in the French Robotics Cup.

## Get it:
```Shell
> git clone --recursive https://github.com/ox223252/RCO_Banana
```

## To compile:
###  First time:
```Shell
> ./Configure/configure
> make
```

### Other time:
```Sehll
> make
```

by default compilation target is arm-Linux (for raspberry), if you want compile for other architecture see  this [github](https://github.com/ox223252/Configure)

## Usage:
You can start the software like that :
```Shell
> ./bin/Banana
```

To get help :
```Shell
> ./bin/Banana -h
build date: 2018.11.08 16:51:25

parameter available for cmd line:
    usage : key value value2 key2 value3 ...
            key : key :  nb elements : help or type
         --help :  -h : bool : this window
        --green :  -g : bool : launch the green prog
          --red :  -r : bool : launch the red prog
            --q :  -q : bool : hide all trace point
        --debug :  -d : bool : display many trace point
        --color :  -c : bool : add color to debug traces
        --noArm : -nA : bool : use it to disable servo motor
      --armWait : (null) : bool : wait end of timeout before set action to done
      --armScan : (null) : bool : wait a key pressed to action to done
      --armDone : (null) : bool : automaticaly set action to done (default)
      --noDrive : -nD : bool : use it to disable drive power
    --driveWait : (null) : bool : wait end of timeout before set action to done
    --driveScan : (null) : bool : wait a key pressed to action to done
    --driveDone : (null) : bool : automaticaly set action to done (default)
     --MaxSpeed : -Ms :    1 : set max speed [ 0 ; 32767 ]
          --ini :  -i :    1 : xml initialisation file path
          --xml :  -x :    1 : xml action file path
         --time :  -t :    1 : game duration in seconds

parameter available in res/config.rco file:
    usage : key=value key2=value2
                 key : help or type
           PATH_DYNA : PATH to access to dynamixels
    PATH_MOTOR_BOARD : PATH to access to dynamixels
PATH_MOTOR_BOARD_UART_SPEED : UART speed for robocloaw board
        PCA9695_ADDR : pca9685 board i2c addr
     XML_ACTION_PATH : xml initialisation file path
       XML_INIT_PATH : xml action file path
         GLOBAL_TIME : game duration in seconds
```

The boolean elements are flags without parameters. Note that, the armWait/armScan/armDone flags are valid only if noArm activated. driveWait/driveScan/driveDone are valid only if noDrive is activated.

The noDrive flag: deactivate propulsion, and noArm deactivate other actions.

Some parameters as time are available in config file and cmd line, the cmd line overwrite the file configuration.

## [RobotClaw lib](https://github.com/michaelrsweet/mxml)

## [Dynamixel lib](https://github.com/ROBOTIS-GIT/DynamixelSDK)

## [License compliance](https://www.gnu.org/licenses/license-list.en.html)

[![Watch me]](png)(youtube)