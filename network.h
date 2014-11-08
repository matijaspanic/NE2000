#ifndef _NETWORK_H_
#define _NETWORK_H_

#include "main.h"

struct ARP {
	uint8_t hardware_type[2];
	uint8_t protocol[2];
	uint8_t hln;
	uint8_t pln;
	uint8_t op[2];
	uint8_t src_mac[6];
	uint8_t src_ip[4];
	uint8_t dest_mac[6];
	uint8_t dest_ip[4];
};

#define OP_ARP_REQUEST 0x01
#define OP_ARP_REPLY  0x02

struct IP {
	uint8_t version;
	uint8_t type_of_service;
	uint8_t total_length[2];
	uint16_t identification;
	uint8_t flags_and_offset[2];
	uint8_t ttl;
	uint8_t protocol;
	uint8_t checksum[2];
	uint8_t src_ip[4];
	uint8_t dest_ip[4];
	uint8_t data[1];
};

#define IP_PROTOCOL_ICMP 	0x01
#define IP_PROTOCOL_TCP		0x06
#define IP_PROTOCOL_UDP		0x11

uint8_t net_ProcessIP(uint8_t *packet, uint16_t length);
uint8_t net_SendIp(uint8_t *dest_ip);
uint16_t net_ProcessARP(uint8_t *packet, uint16_t length);
uint8_t net_DiscoverMac(uint8_t *mac, uint8_t *ip);

#endif