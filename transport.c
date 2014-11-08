#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>


#include "main.h"
#include "usart.h"
#include "transport.h"
#include "net_common.h"
#include "http.h"
#include "network.h"

//#define TCP_DEBUG

extern uint8_t net_packet[MTU_SIZE];

uint16_t net_ProcessICMP(uint8_t *packet, uint16_t length) {
	struct ICMP *icmp = (struct ICMP *)packet;

	#ifdef USART_DEBUG
	SendString_P(PSTR("Processing ICMP\n"));
	SendString_P(PSTR(" length:"));
	SendInt(length);
	SendString_P(PSTR("\n"));
	
	SendString_P(PSTR(" data:\n  "));
	uint16_t i;
	for (i = 0; i < length-4; i++) {
		SendHexByte(icmp->data[i]);
		if ((i+1)%16==0)
			SendString_P(PSTR("\n  "));
		else if ((i+1)%4==0)
			SendString_P(PSTR(", "));
		else
			SendString_P(PSTR(","));
	}
	SendString_P(PSTR("\n"));
	#endif
	
	/*SendString_P(PSTR("IP checksum: "));
	SendHexByte(icmp->checksum[0]);
	SendString_P(PSTR(", "));
	SendHexByte(icmp->checksum[1]);
	SendString_P(PSTR("\n"));
	
	icmp->checksum[0] = 0;
	icmp->checksum[1] = 0;
	
	uint8_t cs[2];
	net_CalcChecksum(packet, length, 0, cs);
	
	SendString_P(PSTR("My checksum: "));
	SendHexByte(cs[0]);
	SendString_P(PSTR(", "));
	SendHexByte(cs[1]);
	SendString_P(PSTR("\n"));*/
	
	if (icmp->type == 0x08 && icmp->subtype == 0x00) { // is it echo request?
		icmp->type = 0; // echo reply

		/*icmp->checksum[0] = 0;
		icmp->checksum[1] = 0;
		uint16_t cs = ~net_CalcChecksum((uint16_t *)packet, length, 0);
		icmp->checksum[0] = cs&0xff;
		icmp->checksum[1] = cs>>8;*/
		
		icmp->checksum[0] = 0;
		icmp->checksum[1] = 0;
		net_CalcChecksum(packet, length, 0, icmp->checksum);
		
		return length;
	}
	
	return 0;
}

uint8_t net_ProcessTCP(uint8_t *packet, uint16_t length) {
	struct TCP *tcp = (struct TCP *)packet;
	
	uint8_t data_offset = ((tcp->data_offset >> 4) & 0x0f) * 4;
	uint16_t data_length = length - data_offset;

	#ifdef TCP_DEBUG
	SendString_P(PSTR("Processing TCP\n packet:\n  "));
	uint16_t i;
	for (i = 0; i < length; i++) {
		SendHexByte(packet[i]);
		if ((i+1)%16==0)
			SendString_P(PSTR("\n  "));
		else if ((i+1)%4==0)
			SendString_P(PSTR(", "));
		else
			SendString_P(PSTR(","));
	}
	
	SendString_P(PSTR("\n Src port: "));
	SendInt(htons(tcp->src_port));
	SendString_P(PSTR("\n Dest port: "));
	SendInt(htons(tcp->dest_port));
	SendString_P(PSTR("\n seq number: "));
	SendLong(htonl(tcp->seq_number));
	SendString_P(PSTR("\n ack number: "));
	SendLong(htonl(tcp->ack_number));
	SendString_P(PSTR("\n length: "));
	SendInt(length);
	SendString_P(PSTR("\n data offset: "));
	SendInt(data_offset);
	SendString_P(PSTR("\n data length: "));
	SendLong(data_length);
	SendString_P(PSTR("\n"));

		
	SendString_P(PSTR(" TCP checksum: "));
	SendHexByte(tcp->checksum[0]);
	SendString_P(PSTR(","));
	SendHexByte(tcp->checksum[1]);
	SendString_P(PSTR("\n"));
	
	SendString_P(PSTR(" TCP checksum(c): "));
	SendHexByte(~tcp->checksum[0]);
	SendString_P(PSTR(","));
	SendHexByte(~tcp->checksum[1]);
	SendString_P(PSTR("\n"));
	
	tcp->checksum[0] = 0;
	tcp->checksum[1] = 0;
	
	uint8_t cs[2];
	net_CalcChecksum(packet, length, (data_offset + data_length), cs);
	
	SendString_P(PSTR(" My checksum: "));
	SendHexByte(cs[0]);
	SendString_P(PSTR(","));
	SendHexByte(cs[1]);
	SendString_P(PSTR("\n"));
	
	SendString_P(PSTR(" My checksum(c): "));
	SendHexByte(~cs[0]);
	SendString_P(PSTR(","));
	SendHexByte(~cs[1]);
	SendString_P(PSTR("\n"));
	
	SendString_P(PSTR(" Flags:"));
	if (tcp->control & FIN) {
		SendString_P(PSTR("FIN,"));
	}
	if (tcp->control & SYN) {
		SendString_P(PSTR("SYN,"));
	}
	if (tcp->control & RST) {
		SendString_P(PSTR("RST,"));
	}
	if (tcp->control & PSH) {
		SendString_P(PSTR("PSH,"));
	}
	if (tcp->control & ACK) {
		SendString_P(PSTR("ACK,"));
	}
	if (tcp->control & URG) {
		SendString_P(PSTR("URG"));
	}
	SendString_P(PSTR("\n"));
	#endif
	
	uint8_t return_tcp = 0;
	
	if ((tcp->control & SYN) && !(tcp->control & ACK)) {
		#ifdef TCP_DEBUG
		SendString_P(PSTR(" Type: Open connection request\n"));
		#endif
		tcp->ack_number = tcp->seq_number + htonl(1UL);
		tcp->seq_number = htonl(tcp->ack_number) / 2 + 123456;
		
		tcp->control = SYN | ACK;
		
		return_tcp = 1;
	}
	else if (tcp->control == ACK) {
		#ifdef TCP_DEBUG
		SendString_P(PSTR(" Type: Connection open response\n"));
		#endif
		
		return_tcp = 0;
	}
	else if (tcp->control == (ACK | PSH)) {
		#ifdef TCP_DEBUG
		SendString_P(PSTR(" Type: Data request\n"));
		#endif
		uint32_t ack_number = tcp->ack_number;
		tcp->ack_number = htonl(htonl(tcp->seq_number) + data_length);
		tcp->seq_number = ack_number;
		
		tcp->control = ACK | PSH;
	
		data_length = net_ProcessHTTP(packet+data_offset, data_length, packet+20);
		return_tcp = 1;
	}
	
	if (return_tcp) {
		tcp->dest_port = tcp->src_port;
		tcp->src_port = htons((uint16_t)80);
		
		tcp->data_offset = 0x50;
		
		tcp->checksum[0] = 0;
		tcp->checksum[1] = 0;
		net_CalcChecksum(packet, 20 + data_length, 1, tcp->checksum);
		
		#ifdef TCP_DEBUG		
		SendString_P(PSTR("Return TCP\n Src port: "));
		SendInt(htons(tcp->src_port));
		SendString_P(PSTR("\n Dest port: "));
		SendInt(htons(tcp->dest_port));
		SendString_P(PSTR("\n seq number: "));
		SendLong(htonl(tcp->seq_number));
		SendString_P(PSTR("\n ack number: "));
		SendLong(htonl(tcp->ack_number));
		data_offset = ((tcp->data_offset >> 4) & 0x0f) * 4;
		SendString_P(PSTR("\n data offset: "));
		SendInt(data_offset);
		SendString_P(PSTR("\n data length: "));
		SendLong(data_length);
		SendString_P(PSTR("\n"));
		
		SendString_P(PSTR(" TCP checksum: "));
		SendHexByte(tcp->checksum[0]);
		SendString_P(PSTR(","));
		SendHexByte(tcp->checksum[1]);
		SendString_P(PSTR("\n"));
		
		SendString_P(PSTR(" Flags:"));
		if (tcp->control & FIN) {
			SendString_P(PSTR("FIN,"));
		}
		if (tcp->control & SYN) {
			SendString_P(PSTR("SYN,"));
		}
		if (tcp->control & RST) {
			SendString_P(PSTR("RST,"));
		}
		if (tcp->control & PSH) {
			SendString_P(PSTR("PSH,"));
		}
		if (tcp->control & ACK) {
			SendString_P(PSTR("ACK,"));
		}
		if (tcp->control & URG) {
			SendString_P(PSTR("URG"));
		}
		SendString_P(PSTR("\n Packet:\n  "));
		uint16_t i;
		for (i = 0; i < 20 + data_length; i++) {
			SendHexByte(packet[i]);
			if ((i+1)%16==0)
				SendString_P(PSTR("\n  "));
			else if ((i+1)%4==0)
				SendString_P(PSTR(", "));
			else
				SendString_P(PSTR(","));
		}
		SendString_P(PSTR("\n"));
		#endif
		
		return 20 + data_length;
	}
	
	return 0;
}