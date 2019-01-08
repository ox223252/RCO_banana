#ifndef MQTT_H
#define MQTT_H

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include "mosquitto.h"

int mqtt_init ( const char * const restrict name, const char * ipAddress, int port );
int mqtt_subscribe ( int * mid, const char * const restrict topic, int qos );
int mqtt_publish ( int * mid, const char * const restrict topic, int payloadlen, const void* payload, int qos, bool	retain );
int add_sub_topic ( const char * const restrict topic );
int erase_sub_topic ( const char * const restrict topic );

#endif //MQTT_H
