
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

Les éléments de type bool sont des flags n'acceptant pas de paramètres. Ils est à noter que les flags armWait/armScan/armDone ne sont valide que si noArm est lui même activé. De même que driveWait/driveScan/driveDone ne sont valide que si noDrive est lui même actif.

Le flag noDrive : désactive tout ce qui est relatif à la propulsion, tandis que noArm désactivera tout ce qui est relatif au autres actionneurs.

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