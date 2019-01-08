#include "mqtt.h"
#include <string.h>
#include <pthread.h>

struct mosquitto *mosq = NULL;
char ** listTopic;
int nbTopic = 0;
pthread_t* thread1;
char* topicInform;
char* topicRequest;


void my_message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
  printf("Topic : %s payload : %s",message->topic,message->payload);
}

void my_connect_callback(struct mosquitto *mosq, void *obj, int result)
{
  for(int i=0;i<nbTopic;i++)
  {
    mqtt_subscribe(NULL,listTopic[i],1);
  }
}

void my_subscribe_callback(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{

}

void *thread_1(void *arg)
{
  mosquitto_loop_forever(mosq, 0, 1);
  /* Pour enlever le warning */
  (void) arg;
  pthread_exit(NULL);
}

int mqtt_init(const char* name, const char* ipAddress, int port)
{
  thread1 = (pthread_t*)malloc(sizeof(pthread_t));
  int rc;
  mosquitto_lib_init();
  mosq = mosquitto_new(name, true, NULL);
  if(!mosq)
  {
    switch(errno){
      case ENOMEM:
      fprintf(stderr, "Error: Out of memory.\n");
      break;
      case EINVAL:
      fprintf(stderr, "Error: Invalid id.\n");
      break;
    }
    mosquitto_lib_cleanup();


    return 1;
  }

  mosquitto_subscribe_callback_set(mosq, my_subscribe_callback);
  mosquitto_message_callback_set(mosq, my_message_callback);
  mosquitto_connect_callback_set(mosq, my_connect_callback);
  rc = mosquitto_connect(mosq, ipAddress,port,1);

  if(rc) return rc;

  if(pthread_create(thread1, NULL, thread_1, NULL) == -1)
  {
    perror("pthread_create");
    return EXIT_FAILURE;
  }
  return rc;
}

int erase_sub_topic(char* topic)
{
  for(int i=0;i<nbTopic;i++)
  {
    if(strcmp(topic,listTopic[i])==0)
    {
      mosquitto_unsubscribe(mosq, NULL,topic);
      for(int j=i;j<nbTopic-1;j++)
      {
        free(listTopic[j]);
        listTopic[j] = malloc(strlen(listTopic[j+1])*sizeof(char));
        strcpy(listTopic[j],listTopic[j+1]);
      }
      nbTopic--;
      return 0;
    }
  }
  return 1;
}

int add_sub_topic(char* topic)
{
  char **ptr_realloc = realloc(listTopic, sizeof(char*)*nbTopic+1);

  if (ptr_realloc != NULL)
  {
    listTopic = ptr_realloc;
    listTopic[nbTopic] = malloc(strlen(topic)*sizeof(char));
    strcpy(listTopic[nbTopic],topic);
    nbTopic++;
    mqtt_subscribe(NULL,topic,1);
    return 0;
  }
  /* Même si ptr_realloc est nul, on ne vide pas la mémoire. On laisse l'initiative au programmeur. */
  return -1;
}

int mqtt_subscribe(int* mid, const char* topic, int qos)
{
  return mosquitto_subscribe(mosq, mid, topic, qos);
}

int mqtt_publish(int*	mid, const char* topic,int payloadlen, const void* payload, int qos, bool	retain)
{
  return mosquitto_publish(mosq, mid, topic, payloadlen, payload, qos, retain);
}
