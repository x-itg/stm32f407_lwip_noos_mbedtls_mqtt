#ifndef __MQTT_CLIENT_MY_H__
#define __MQTT_CLIENT_MY_H__

#include "ip_addr.h"

#define MQTT_SERVER_NAME	"mqtt.iotwonderful.cn"
#define MQTT_SERVER_PORT	8883

void mqtt_init(ip_addr_t *serverip);
void do_mqtt_publish(char *pub_payload);

#endif

