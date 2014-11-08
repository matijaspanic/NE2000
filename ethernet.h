#ifndef _ETHERNET_H_
#define _ETHERNET_H_

#include "main.h"

struct Ethernet {
	uint8_t dest_mac[6];
	uint8_t src_mac[6];
	uint8_t type[2];
	uint8_t data[1];
};

struct ArpRecord {
	uint8_t mac[6];
	uint8_t ip[4];
};

#define NET_ARP_TABLE_SIZE 4

void net_SendEthernet(uint8_t *packet, uint16_t data_length, uint8_t *dest_mac);
void net_ProcessEthernet(uint8_t *packet, uint16_t length);
void net_Receive(void);

#endif