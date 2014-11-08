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
#include "transport.h"
#include "net_common.h"

//#define ARP_DEBUG
//#define IP_DEBUG

//struct ArpRecord net_arp_table[NET_ARP_TABLE_SIZE];

extern uint8_t net_mac[6];

uint8_t PROGMEM net_ip[4] = { 192, 168, 1, 20 };

extern uint8_t net_packet[MTU_SIZE];

uint16_t id = 0;

/*void net_SendArpBroadcast (uint8_t *ip) {
	struct ARP *arp = (struct ARP *)net_packet+14;
	
	arp->hardware_type[0] = 0x00;
	arp->hardware_type[1] = 0x01;
	arp->protocol[0] = 0x08;
	arp->protocol[1] = 0x00;
	arp->hln = 0x06;
	arp->pln = 0x04;
	arp->op[0] = 0x00; 
	arp->op[1] = 0x01;
	
	uint8_t broadcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
	
	memcpy(arp->src_mac, net_mac, 6);
	memcpy(arp->dest_mac, broadcast_mac, 6);
	
	memcpy_P(arp->src_ip, net_ip, 4);
	memcpy(arp->dest_ip, ip, 4);
	
	net_SendEthernet((uint8_t *)arp, 46, broadcast_mac);
}

uint8_t net_SearchArp(uint8_t *mac, uint8_t *ip, uint8_t find) { // find: 1 = give mac, get ip    2 = give ip, get mac
	uint8_t i;
	for (i = 0; i < NET_ARP_TABLE_SIZE; i++) {
		if (find == 1 && memcmp(net_arp_table[i].mac, mac, 6) == 0) {
			memcpy(ip, net_arp_table[i].ip, 4);
			return 1;
		}
		else if (find == 2 && memcmp(net_arp_table[i].ip, ip, 4) == 0) {
			memcpy(mac, net_arp_table[i].mac, 6);
			return 1;
		}
	}
	
	return 0;
}

void net_NewArpRecord (uint8_t *mac, uint8_t *ip) {
	uint8_t i;
	for (i = 0; i < NET_ARP_TABLE_SIZE; i++) {
		if (net_arp_table[i].ip[0] == 0) {
			#ifdef IP_DEBUG
			SendString_P(PSTR("New Arp record"));
			#endif
			memcpy(net_arp_table[i].mac, mac, 6);
			memcpy(net_arp_table[i].ip, ip, 4);
			return;
		}
	}
	#ifdef IP_DEBUG
	if (i==NET_ARP_TABLE_SIZE)
		SendString_P(PSTR("ERROR: Arp table size too small"));
	#endif
}

uint8_t net_DiscoverMac(uint8_t *mac, uint8_t *ip) {
	if (net_SearchArp(mac, ip, 2)) {
		return 1;
	}
	else {
		net_SendArpBroadcast(ip);
		
		uint16_t time = 0;
		while (time > 2000) {
			net_Receive();
			
			if (net_SearchArp(mac, ip, 2))
				return 1;
			
			_delay_ms(1);
			time++;
		}
	}
	
	return 0;
}*/

uint16_t net_ProcessARP(uint8_t *packet, uint16_t length) {
	#ifdef ARP_DEBUG
	SendString_P(PSTR("Processing ARP\n"));
	#endif
	
	struct ARP *arp = (struct ARP *)packet;
	
	if (arp->hardware_type[0] == 0x00 && arp->hardware_type[1] == 0x01
		&& arp->protocol[0] == 0x08 && arp->protocol[1] == 0x00
		&& arp->hln == 0x06
		&& arp->pln == 0x04)
	{
		if (memcmp_P(arp->dest_ip, net_ip, 4)!=0) {
			#ifdef ARP_DEBUG
			SendString_P(PSTR(" Wrong IP\n"));
			#endif
			return 0;
		}
		
		if (arp->op[0] == 0x00 && arp->op[1] == 0x01) { // request
			arp->op[1] = 0x02;
			
			memcpy(arp->dest_mac, arp->src_mac, 6); // set dest mac
			memcpy_P(arp->src_mac, net_mac, 6); // set src mac
			
			memcpy(arp->dest_ip, arp->src_ip, 4); // set dest ip
			memcpy_P(arp->src_ip, net_ip, 4); // set src mac
			
			//net_NewArpRecord(arp->src_mac, arp->src_ip);
			
			return length;
		}
		/*else if (arp->op[0] == 0x00 && arp->op[1] == 0x02) { // reply
			net_NewArpRecord(arp->src_mac, arp->src_ip);
		
			return 0;
		}*/
	}
	
	return 0;
}

/*uint8_t net_SendIp(uint8_t *dest_ip) {
	struct IP *ip = (struct IP *)net_packet+14;
	
	if (net_DiscoverMac(ip->dest_ip, dest_ip)==0) {
		SendString ("Timeout: no such IP on network."));
		return 0;
	}
	
	return 1;
}*/
	
uint8_t net_ProcessIP(uint8_t *packet, uint16_t length) {
	struct IP *ip = (struct IP *)packet;
	
	#ifdef IP_DEBUG
	SendString_P(PSTR("Processing IP\n packet:\n  "));
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
	SendString_P(PSTR("\n"));
	
	SendString_P(PSTR(" Dest IP: "));
	for (i=0;i<4;i++) {
		if (i>0)
			SendByte('.');
		SendInt(ip->dest_ip[i]);
	}
	SendString_P(PSTR("\n Src IP: "));
	for (i=0;i<4;i++) {
		if (i>0)
			SendByte('.');
		SendInt(ip->src_ip[i]);
	}
	SendString_P(PSTR("\n"));
	#endif
	
	if (memcmp_P(ip->dest_ip, net_ip, 4)!=0) { // not my IP?
		#ifdef IP_DEBUG
		SendString_P(PSTR(" Not my IP.\n"));
		#endif
		return 0;
	}
	
	if ((ip->version & 0xf0) != 0x40) { // not IPv4?
		#ifdef IP_DEBUG
		SendString_P(PSTR(" Not IPv4.\n"));
		#endif
		return 0;
	}
	
	if (((ip->flags_and_offset[0] & 0xBF) != 0x00) || (net_packet[21] != 0x00)) {
		#ifdef IP_DEBUG
		SendString_P(PSTR("Fragmented IP packet."));
		#endif
		return 0;
	}
	
	uint16_t total_length = htons(*((uint16_t *)&ip->total_length));
	uint16_t header_length = (ip->version & 0b00001111) * 4;
	uint16_t data_length = total_length - header_length;

	#ifdef IP_DEBUG
	SendString_P(PSTR(" total length: "));
	SendInt(total_length);
	
	SendString_P(PSTR("\n header length: "));
	SendInt(header_length);
	
	SendString_P(PSTR("\n data length: "));
	SendInt(data_length);
	SendString_P(PSTR("\n"));
	
	SendString_P(PSTR(" Protocol: "));
	#endif
	
	if (ip->protocol == IP_PROTOCOL_ICMP)
	{
		#ifdef IP_DEBUG
		SendString_P(PSTR("ICMP\n"));
		#endif
		data_length = net_ProcessICMP(ip->data, data_length);
	}
	else if (ip->protocol == IP_PROTOCOL_TCP)
	{
		#ifdef IP_DEBUG
		SendString_P(PSTR("TCP\n"));
		#endif
		data_length = net_ProcessTCP(ip->data, data_length);
	}
	/*else if (ip->protocol == IP_PROTOCOL_UDP)
	{
		#ifdef IP_DEBUG
		SendString_P(PSTR("UDP\n"));
		#endif
		//data_length = net_ProcessUDP(ip->data, data_length);
	}*/
	
	
	if (data_length>0) {	
		memcpy(ip->dest_ip, ip->src_ip, 4); // set dest ip
		memcpy_P(ip->src_ip, net_ip, 4); // set src ip
		
		ip->identification = htons(id);
		id++;
		
		/*ip->flags_and_offset[0] = 1 << 5;
		ip->flags_and_offset[1] = 0;*/
		
		ip->ttl = 64;
		
		ip->total_length[1] = ((uint16_t)(20+data_length))&0xff;
		ip->total_length[0] = ((uint16_t)(20+data_length))>>8;
		
		ip->checksum[0] = 0;
		ip->checksum[1] = 0;
		net_CalcChecksum(packet, header_length, 0, ip->checksum);
		
		#ifdef IP_DEBUG
		SendString_P(PSTR("Return IP\n Dest IP: "));
		for (i=0;i<4;i++) {
			if (i>0)
				SendByte('.');
			SendInt(ip->dest_ip[i]);
		}
		SendString_P(PSTR("\n Src IP: "));
		for (i=0;i<4;i++) {
			if (i>0)
				SendByte('.');
			SendInt(ip->src_ip[i]);
		}
		
		uint16_t total_length2 = htons(*((uint16_t *)&ip->total_length));
		uint16_t header_length2 = (ip->version & 0b00001111) * 4;
		uint16_t data_length2 = total_length2 - header_length2;
	
		SendString_P(PSTR("\n total length: "));
		SendInt(total_length2);
		SendString_P(PSTR("\n header length:"));
		SendInt(header_length2);
		SendString_P(PSTR("\n data length:"));
		SendInt(data_length2);
		SendString_P(PSTR("\n Packet:\n  "));
		uint16_t i;
		for (i = 0; i < data_length+20; i++) {
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
		
		return data_length+20;
	}
	
	return 0;
}
