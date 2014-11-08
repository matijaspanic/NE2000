#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

#include "main.h"

struct ICMP {
	uint8_t type;
	uint8_t subtype;
	uint8_t checksum[2];
	uint8_t data[1];
};

struct TCP {
	uint16_t src_port;
	uint16_t dest_port;
	uint32_t seq_number;
	uint32_t ack_number;
	uint8_t data_offset;
	uint8_t control;
	uint16_t window;
	uint8_t checksum[2];
	uint16_t urgent_pointer;
};

#define FIN 0x01
#define SYN 0x02
#define RST 0x04
#define PSH 0x08
#define ACK 0x10
#define URG 0x20

struct TCB {
	
};

uint8_t net_ProcessTCP(uint8_t *packet, uint16_t length);
uint16_t net_ProcessICMP(uint8_t *packet, uint16_t length);
	
#endif