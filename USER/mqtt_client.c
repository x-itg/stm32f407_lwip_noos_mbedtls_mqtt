#include "mqtt_client.h"
#include "lwip/pbuf.h"
#include "lwip/apps/mqtt.h"
#include "lwip/apps/mqtt_priv.h"
#include "lwip/netif.h"
#include "string.h"
#include "lwip/altcp_tls.h"

#include "cert.h"

extern uint8_t gMacAddress[6];
extern ip_addr_t mqtt_server_ip;

char resp_topic[20];
static mqtt_client_t mqtt_client;
static void do_connect(mqtt_client_t *client, ip_addr_t *serverip);
static struct mqtt_connect_client_info_t ci; 

/* Called when publish is complete either with success or failure */
//static void mqtt_pub_request_cb(void *arg, err_t result)
//{
//  if(result != ERR_OK) {
//    printf("Publish result: %d\r\n", result);
//  }
//}

//static void __do_mqtt_publish(mqtt_client_t *client, char *payload)
//{
//  err_t err;
////  err = mqtt_publish(client, "/stm32_topic", pub_payload, strlen(pub_payload), qos, retain, mqtt_pub_request_cb, arg);
//  err = mqtt_publish(client, resp_topic, payload, strlen(payload), 0, 0, NULL, NULL);
//  if(err != ERR_OK) {
//    printf("Publish err: %d\r\n", err);
//  }
//}

//static void mqtt_incoming_publish_cb(void *arg, const char *topic, u32_t tot_len)
//{
//  printf("Incoming publish at topic %s with total length %u\r\n", topic, (unsigned int)tot_len);
//  
//  /* Decode topic string into a user defined reference */
//
//}

static void mqtt_incoming_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags)
{
    u8_t *data_ptr = (u8_t *)data;
//  printf("Incoming publish payload with length %d, flags %u\r\n", len, (unsigned int)flags);
  if(flags & MQTT_DATA_FLAG_LAST) {
    /* Last fragment of payload received (or whole part if payload fits receive buffer
    See MQTT_VAR_HEADER_BUFFER_LEN)  */  
    
    /* Don't trust the publisher, check zero termination */
    if(data_ptr[len] == 0) {
//      printf("mqtt_incoming_data_cb:[%s]\r\n", (const char *)data);
      if (strncmp((char *)data_ptr, "ctrl", 4) == 0) {
          //do_analysis_data((char *)(data_ptr + 4 + 1));
      }
    }
    memset((char *)data_ptr, 0, len);
  } else {
    /* Handle fragmented payload, store in buffer, write to file or whatever */
  }
}

static void mqtt_sub_request_cb(void *arg, err_t result)
{
  /* Just print the result code here for simplicity,
  normal behaviour would be to take some action if subscribe fails like
  notifying user, retry subscribe or disconnect from server */
  printf("Subscribe result: %d\r\n", result);
}

static void mqtt_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status)
{
//  err_t err;
  if(status == MQTT_CONNECT_ACCEPTED) {
    printf("mqtt_connection_cb: Successfully connected\r\n");
    /* Setup callback for incoming publish requests */
//    mqtt_set_inpub_callback(client, mqtt_incoming_publish_cb, mqtt_incoming_data_cb, arg);
    mqtt_set_inpub_callback(client, NULL, mqtt_incoming_data_cb, arg);
    
    /* Subscribe to a topic named "subtopic" with QoS level 1, call mqtt_sub_request_cb with result */
    mqtt_subscribe(client, "/door_ctrl", 0, mqtt_sub_request_cb, arg);
    mqtt_publish(client, "/status/0080e1000001", "online", strlen("online"), 0, 0, NULL, NULL);
    
  } else {
    /* Its more nice to be connected, so try to reconnect */
    printf("mqtt_connection_cb: Disconnected, reason: %d ip:%s\r\n", status, ipaddr_ntoa((ip_addr_t *)arg));
    altcp_tls_free_config(ci.tls_config);
    do_connect(client, (ip_addr_t *)arg);
  }
}

static void do_connect(mqtt_client_t *client, ip_addr_t *serverip)
{
  
  err_t err;
  //2*6 + 1
  char client_id[13];
  //2*6 + 1 + 6 + 1 + 1
  char lastwill_topic[21];
  
  /* Setup an empty client info structure */
  memset(&ci, 0, sizeof(ci));
  
//  snprintf(client_id, 13, "%02x%02x%02x%02x%02x%02x", 
//           macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
//  snprintf(lastwill_topic, 21, "/status/%02x%02x%02x%02x%02x%02x",
//           macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
  snprintf(resp_topic, 20, "/resp/%02x%02x%02x%02x%02x%02x",
           gMacAddress[0], gMacAddress[1], gMacAddress[2], gMacAddress[3], gMacAddress[4], gMacAddress[5]);

  memcpy(client_id, "random", 6);
  memcpy(lastwill_topic, "/status/024251720577", strlen("/status/024251720577"));
  
  ci.client_id = client_id;
  ci.will_topic = lastwill_topic;
  uint16_t mqtt_server_port = MQTT_SERVER_PORT;
  
  /* Minimal amount of information required is client identifier, so set it here */
  ci.will_qos = 0;
  ci.will_retain = 1;
  ci.will_msg = "32offline";
  ci.keep_alive = 60;
  ci.client_user = "test_001";
  ci.client_pass = "NjBlNjY3ZWRlZ";
  
  ci.tls_config = altcp_tls_create_config_client(certfile, certlen);
  
  /* Initiate client and connect to server, if this fails immediately an error code is returned
  otherwise mqtt_connection_cb will be called with connection result after attempting
  to establish a connection with the server.
  For now MQTT version 3.1.1 is always used */

  err = mqtt_client_connect(client, serverip, mqtt_server_port, mqtt_connection_cb, (void *)serverip, &ci);

  /* For now just print the result code if something goes wrong */
  if(err != ERR_OK) {
    printf("mqtt_connect return %d\r\n", err);
  } else {
//    mqtt_publish(client, ci.will_topic, "32online", 9, 0, 1, NULL, NULL);
//    if(err != ERR_OK) {
//      printf("online publish err: %d\r\n", err);
//    }
  }
}

void mqtt_init(ip_addr_t *serverip) {
  
  do_connect(&mqtt_client, serverip);
}

void do_mqtt_publish(char *pub_payload) {
  err_t err;
  if (mqtt_client_is_connected(&mqtt_client)) {
    err = mqtt_publish(&mqtt_client, resp_topic, pub_payload, strlen(pub_payload), 0, 0, NULL, NULL);
    if(err != ERR_OK) {
      printf("Publish err: %d\r\n", err);
    }
  } else {
    printf("mqtt not connected!\r\n");
  }
}

