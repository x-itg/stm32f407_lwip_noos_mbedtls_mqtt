# stm32f407_lwip_noos_mbedtls_mqtt

#### 本工程使用stm32cube 6.0.1版本工具生成代码
#### lwip版本:2.1.2
#### mbedtls版本:2.23.0
    mbedtls是自己从网上下载的资源，并不是通过cube生成的。


## lwipopts.h 

以下部分是自己添加的配置

```
#define LWIP_ALTCP                  1
#define LWIP_ALTCP_TLS              1
#define LWIP_ALTCP_TLS_MBEDTLS      1
#define MEMP_NUM_SYS_TIMEOUT            (LWIP_NUM_SYS_TIMEOUT_INTERNAL + 2)

#define MEM_SIZE                  (60 * 1024)  
#define MEMP_NUM_TCP_PCB          20
#define MEMP_NUM_TCP_PCB_LISTEN   10
#define MEMP_NUM_TCP_SEG          20
#define PBUF_POOL_SIZE            31
//for mqtt tls
#define TCP_WND               (31 * TCP_MSS)
//#define TCP_SND_BUF           (10 * TCP_MSS)

//#define PBUF_POOL_SIZE        32  
#define MEMP_NUM_ALTCP_PCB    (MEMP_NUM_TCP_PCB)

#define LWIP_DEBUG            1
#define DNS_DEBUG             LWIP_DBG_ON
#define DHCP_DEBUG            LWIP_DBG_ON
#define MQTT_DEBUG            LWIP_DBG_ON
#define ALTCP_MBEDTLS_DEBUG   LWIP_DBG_ON
#define TCP_DEBUG             LWIP_DBG_ON
#define MEM_DEBUG             LWIP_DBG_ON

```

## mbedtls/config.h

在该文件中在使能了`#define MBEDTLS_PLATFORM_MEMORY`之后，mbedtls申请内存的相关代码才不再出现错误提示。

## MAC地址

stm32f407的mac地址必须保持前三位为 `0x00 0x80 0xe1`

## cert文件的加载

该加载代码为

```
ci.tls_config = altcp_tls_create_config_client(certfile, certlen);
```

该部分代码经过调试发现必须要放在connect部分，即每次链接都需要临时申请，否则就会出错。

在对应的`disconnect`中调用`altcp_tls_free_config`来释放得到的`tls_config`结构体,所以需要将该变量设置为全局变量。

## 在lwip.c中执行了mbedtls的初始化部分代码.

```
    ...
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
    ...
```
