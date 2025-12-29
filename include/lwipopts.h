#pragma once

#define NO_SYS 1

#define LWIP_SOCKET 0
#define LWIP_NETCONN 0

#define LWIP_UDP 1
#define LWIP_TCP 0
#define LWIP_ICMP 1
#define LWIP_DHCP 1
#define LWIP_DNS 1

#define MEM_ALIGNMENT 4
#define MEM_SIZE 4000
#define MEMP_NUM_PBUF 16
#define PBUF_POOL_SIZE 8

#define CHECKSUM_GEN_IP 1
#define CHECKSUM_GEN_UDP 1
#define CHECKSUM_CHECK_IP 1
#define CHECKSUM_CHECK_UDP 1
