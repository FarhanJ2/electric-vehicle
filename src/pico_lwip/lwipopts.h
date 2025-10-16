#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

/* Use the NO_SYS=1 mode (no OS) */
#define NO_SYS 1

/* Enable socket API */
#define LWIP_SOCKET 1

/* Enable TCP, UDP and DHCP */
#define LWIP_TCP 1
#define LWIP_UDP 1
#define LWIP_DHCP 1

/* Enable ICMP (ping) */
#define LWIP_ICMP 1

/* Memory options */
#define MEM_ALIGNMENT 4
#define MEM_SIZE 1600
#define MEMP_NUM_TCP_PCB 8
#define MEMP_NUM_UDP_PCB 4
#define MEMP_NUM_SYS_TIMEOUT 8

/* Enable LWIP debugging */
#define LWIP_DEBUG 0
#define TCP_DEBUG LWIP_DBG_OFF
#define DHCP_DEBUG LWIP_DBG_OFF

#endif /* __LWIPOPTS_H__ */
