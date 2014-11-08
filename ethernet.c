#include <stdlib.h>
#include <stdio.h>
#include <avr/io.h>
#include <string.h>
#include <avr/pgmspace.h>


#include "main.h"
#include "usart.h"
#include "ne2000.h"
#include "network.h"
#include "ethernet.h"

uint8_t PROGMEM net_mac[6] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 };
extern uint8_t net_packet[MTU_SIZE];

void net_SendEthernet(uint8_t *packet, uint16_t data_length, uint8_t *dest_mac) {

}

void net_ProcessEthernet(uint8_t *packet, uint16_t length) {
	struct Ethernet *frame = (struct Ethernet *)packet;
		
	if ((length >= 64) && ((frame->src_mac[0] & NET_MCASTBIT) == 0)) { 
		if ((memcmp_P(frame->dest_mac, net_mac, 6)==0)
		 	|| (frame->dest_mac[0] == 0xff && frame->dest_mac[1] == 0xff && frame->dest_mac[2] == 0xff 
		 	&& frame->dest_mac[3] == 0xff && frame->dest_mac[4] == 0xff && frame->dest_mac[5] == 0xff))
		{
			uint16_t data_length = length - 18;
		
			#ifdef USART_DEBUG
			uint16_t i = 0;
			SendString_P(PSTR("\nReceived packet:\n Dest MAC: ");
			for (i = 0; i < 6; i++ ) {
				if (i>0)
					SendByte(':');
				SendHexByte(frame->dest_mac[i]);
			}
			SendString_P(PSTR("\n Src MAC:  ");
			for (i = 0; i < 6; i++ ) {
				if (i>0)
					SendByte(':');
				SendHexByte(frame->src_mac[i]);
			}
			SendString_P(PSTR("\n Length: ");
			SendInt(length);
			SendString_P(PSTR("\n Type: ");
			
			if (frame->type[0] == 0x08 && frame->type[1] == 0x06) { // Protocol ARP
				SendString_P(PSTR("ARP");
			}
			else if (frame->type[0] == 0x08 && frame->type[1] == 0x00) { // Protocol IP
				SendString_P(PSTR("IP");
			}
			SendString_P(PSTR("\n");
		
			SendString_P(PSTR(" data:\n  ");
			for (i = 0; i < data_length; i++) {
				SendHexByte(frame->data[i]);
				if ((i+1)%16==0)
					SendString_P(PSTR("\n  ");
				else if ((i+1)%4==0)
					SendString_P(PSTR(", ");
				else
					SendString_P(PSTR(",");
			}
			SendString_P(PSTR("\n");
			
			SendString_P(PSTR(" crc: ");
			for (i = data_length; i < data_length+4; i++) {
				SendHexByte(frame->data[i]);
				SendString_P(PSTR(",");
			}
			SendString_P(PSTR("\n");
			#endif
			
			if (frame->type[0] == 0x08 && frame->type[1] == 0x06) { // Protocol ARP
				data_length = net_ProcessARP(frame->data, data_length);
			} 
			else if (frame->type[0] == 0x08 && frame->type[1] == 0x00) { // Protocol IP
				data_length = net_ProcessIP(frame->data, data_length);
			}
			
			if (data_length>0) {
				memcpy(frame->dest_mac, frame->src_mac, 6);
				memcpy_P(frame->src_mac, net_mac, 6);
				
				#ifdef USART_DEBUG
					uint16_t i = 0;
					SendString_P(PSTR("Sending packet:\n Dest MAC: ");
					for (i = 0; i < 6; i++ ) {
						if (i>0)
							SendByte(':');
						SendHexByte(frame->dest_mac[i]);
					}
					SendString_P(PSTR("\n Src MAC:  ");
					for (i = 0; i < 6; i++ ) {
						if (i>0)
							SendByte(':');
						SendHexByte(frame->src_mac[i]);
					}
					SendString_P(PSTR("\n Length: ");
					SendInt(18+data_length);
					SendString_P(PSTR("\n Type: ");
					
					if (frame->type[0] == 0x08 && frame->type[1] == 0x06) { // Protocol ARP
						SendString_P(PSTR("ARP");
					}
					else if (frame->type[0] == 0x08 && frame->type[1] == 0x00) { // Protocol IP
						SendString_P(PSTR("IP");
					}
					SendString_P(PSTR("\n");
					
					SendString_P(PSTR(" data:\n  ");
					for (i = 0; i < data_length; i++) {
						SendHexByte(frame->data[i]);
						if ((i+1)%16==0)
							SendString_P(PSTR("\n  ");
						else if ((i+1)%4==0)
							SendString_P(PSTR(", ");
						else
							SendString_P(PSTR(",");
					}
					SendString_P(PSTR("\n");
					
					SendString_P(PSTR(" crc: ");
					for (i = data_length; i < data_length+4; i++) {
						SendHexByte(frame->data[i]);
						SendString_P(PSTR(",");
					}
					SendString_P(PSTR("\n");
				#endif
			
				ne2000_SendPacket(net_packet, 18+data_length);
			}
		}
	}
}

void net_Receive(void) {
	uint16_t received_bytes = 0;
	if ((received_bytes = ne2000_ReceivePacket(net_packet, MTU_SIZE)) > 0) {
		net_ProcessEthernet(net_packet, received_bytes);
	}
}