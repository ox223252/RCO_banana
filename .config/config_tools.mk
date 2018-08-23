# flags for alls targets
FLAGS+= -D'BY_LINUX_MAKEFILE' -DFOE_WITH_THREAD -g -DTIMER_WITH_FOE

# tools / flags / libs only for native target
CC=gcc
CXX=g++
natif_FLAGS=
natif_LIBS=
natif_EXEC_AFTER=

# tools / flags / libs only for arm Linux target
LABEL=arm
arm_CROSS_CC=arm-linux-gnueabi-gcc
arm_CROSS_CXX=arm-linux-gnueabi-g++
arm_FLAGS=
arm_LIBS=
arm_EXEC_AFTER=
