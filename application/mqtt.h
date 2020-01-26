#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "MQTTClient.h"

void publish(MQTTClient client, char* topic, char* payload);
int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message); 
