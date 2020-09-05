/**
 ******************************************************************************
  * File Name          : LWIP.c
  * Description        : This file provides initialization code for LWIP
  *                      middleWare.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#if defined ( __CC_ARM )  /* MDK ARM Compiler */
#include "lwip/sio.h"
#endif /* MDK ARM Compiler */
#include "ethernetif.h"

/* USER CODE BEGIN 0 */
#include "ip_addr.h"
#include "dns.h"
#include "mqtt_client.h"
#include "mbedtls/ssl.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"

/* USER CODE END 0 */
/* Private function prototypes -----------------------------------------------*/
/* ETH Variables initialization ----------------------------------------------*/
void Error_Handler(void);

/* DHCP Variables initialization ---------------------------------------------*/
uint32_t DHCPfineTimer = 0;
uint32_t DHCPcoarseTimer = 0;
/* USER CODE BEGIN 1 */

ip_addr_t mqtt_server_ip;
ip_addr_t dns0server;
ip_addr_t dns1server;
ip_addr_t dns2server;
uint8_t isStartMQTT     = 0;
uint8_t isStartGetHost  = 0;

mbedtls_ssl_context ssl;
mbedtls_ssl_config conf;
mbedtls_x509_crt cert;
mbedtls_ctr_drbg_context ctr_drbg;
mbedtls_entropy_context entropy;

static void dns_found(const char *name, const ip_addr_t *addr, void *arg)
{
  LWIP_UNUSED_ARG(arg);
  printf("dns_found => %s: %s\r\n", name, addr ? ipaddr_ntoa(addr) : "<not found>");
	if (addr->addr) {
		mqtt_server_ip.addr = addr->addr;
	}
}


/* USER CODE END 1 */

/* Variables Initialization */
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */

/**
  * LwIP initialization function
  */
void MX_LWIP_Init(void)
{
  /* Initilialize the LwIP stack without RTOS */
  lwip_init();

  /* IP addresses initialization with DHCP (IPv4) */
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;

  /* add the network interface (IPv4/IPv6) without RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);

  /* Registers the default network interface */
  netif_set_default(&gnetif);

  if (netif_is_link_up(&gnetif))
  {
    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
  }
  else
  {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
  }

  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(&gnetif, ethernetif_update_config);

  /* Create the Ethernet link handler thread */

  /* Start DHCP negotiation for a network interface (IPv4) */
  dhcp_start(&gnetif);

/* USER CODE BEGIN 3 */
  mbedtls_ssl_init(&ssl);
  mbedtls_ssl_config_init(&conf);
  mbedtls_x509_crt_init(&cert);
  mbedtls_ctr_drbg_init(&ctr_drbg);
  mbedtls_entropy_init( &entropy );
  
  dns_init();
	IP_ADDR4(&dns0server, 114, 114, 114, 114);
	IP_ADDR4(&dns1server, 4, 4, 4, 4);
	IP_ADDR4(&dns2server, 8, 8, 8, 8);
	dns_setserver(0, &dns0server);
	dns_setserver(1, &dns0server);
	dns_setserver(2, &dns0server);
    
/* USER CODE END 3 */
}

#ifdef USE_OBSOLETE_USER_CODE_SECTION_4
/* Kept to help code migration. (See new 4_1, 4_2... sections) */
/* Avoid to use this user section which will become obsolete. */
/* USER CODE BEGIN 4 */
/* USER CODE END 4 */
#endif

/**
 * ----------------------------------------------------------------------
 * Function given to help user to continue LwIP Initialization
 * Up to user to complete or change this function ...
 * Up to user to call this function in main.c in while (1) of main(void)
 *-----------------------------------------------------------------------
 * Read a received packet from the Ethernet buffers
 * Send it to the lwIP stack for handling
 * Handle timeouts if LWIP_TIMERS is set and without RTOS
 * Handle the llink status if LWIP_NETIF_LINK_CALLBACK is set and without RTOS
 */
void MX_LWIP_Process(void)
{
/* USER CODE BEGIN 4_1 */
/* USER CODE END 4_1 */
  ethernetif_input(&gnetif);

/* USER CODE BEGIN 4_2 */
/* USER CODE END 4_2 */
  /* Handle timeouts */
  sys_check_timeouts();

/* USER CODE BEGIN 4_3 */
  if (gnetif.ip_addr.addr) {
    if (!isStartGetHost) {
      isStartGetHost = 1;
      dns_gethostbyname(MQTT_SERVER_NAME, &mqtt_server_ip, dns_found, NULL);
    }
  }
  if (mqtt_server_ip.addr) {
    if (!isStartMQTT) {
      isStartMQTT = 1;
      mqtt_init(&mqtt_server_ip);
    }
  }

/* USER CODE END 4_3 */
}

#if defined ( __CC_ARM )  /* MDK ARM Compiler */
/**
 * Opens a serial device for communication.
 *
 * @param devnum device number
 * @return handle to serial device if successful, NULL otherwise
 */
sio_fd_t sio_open(u8_t devnum)
{
  sio_fd_t sd;

/* USER CODE BEGIN 7 */
  sd = 0; // dummy code
/* USER CODE END 7 */

  return sd;
}

/**
 * Sends a single character to the serial device.
 *
 * @param c character to send
 * @param fd serial device handle
 *
 * @note This function will block until the character can be sent.
 */
void sio_send(u8_t c, sio_fd_t fd)
{
/* USER CODE BEGIN 8 */
/* USER CODE END 8 */
}

/**
 * Reads from the serial device.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received - may be 0 if aborted by sio_read_abort
 *
 * @note This function will block until data can be received. The blocking
 * can be cancelled by calling sio_read_abort().
 */
u32_t sio_read(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 9 */
  recved_bytes = 0; // dummy code
/* USER CODE END 9 */
  return recved_bytes;
}

/**
 * Tries to read from the serial device. Same as sio_read but returns
 * immediately if no data is available and never blocks.
 *
 * @param fd serial device handle
 * @param data pointer to data buffer for receiving
 * @param len maximum length (in bytes) of data to receive
 * @return number of bytes actually received
 */
u32_t sio_tryread(sio_fd_t fd, u8_t *data, u32_t len)
{
  u32_t recved_bytes;

/* USER CODE BEGIN 10 */
  recved_bytes = 0; // dummy code
/* USER CODE END 10 */
  return recved_bytes;
}
#endif /* MDK ARM Compiler */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
