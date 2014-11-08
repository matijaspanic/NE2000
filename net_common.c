#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "net_common.h"
#include "usart.h"

/*uint16_t net_CalcChecksum2(uint16_t *packet, uint16_t length, uint8_t ippseudo) {
	uint16_t sum = 0;
  
	if (ippseudo==1) {
		sum += htons(length+(uint16_t)6);
		length += 8;
		packet -= 4;
	}
	
	for(; length > 1; length -= 2) {
		sum += *packet;
		if(sum < *packet) {
			sum++;
		}
		packet++;
	}

	if(length == 1) {
		sum += htons(((uint16_t)(*(uint8_t *)packet)) << 8);
		if(sum < htons(((uint16_t)(*(uint8_t *)packet)) << 8)) {
			sum++;
    	}
  	}
  
	return sum;
}*/

void net_CalcChecksum(uint8_t *packet, uint16_t length, uint8_t ippseudo, uint8_t *out) {
	uint8_t sum[2];
	sum[0] = 0;
	sum[1] = 0;
	
	uint16_t i = 0;
	
	if (ippseudo>0) {
		sum[0] = length / 256;
		sum[1] = length % 256;
		sum[1] += 6;
		if (sum[1] == 0)
			sum[0]++;
		packet -= 8;
		length += 8;
	}
	
	for(; i < length; i+=2) {
		/*SendString_P(PSTR("Sum: "));
		SendInt(sum[0]);
		SendString_P(PSTR(","));
		SendInt(sum[1]);
		SendString_P(PSTR(" + "));
		SendInt(packet[i+0]);
		SendString_P(PSTR(","));
		SendInt(packet[i+1]);
		SendString_P(PSTR(" = "));*/
		
		sum[1] += packet[i+1];
		if (sum[1] < packet[i+1]) {
			sum[0]++;
			
			if (sum[0]==0) {
				sum[1]++;
				if (sum[1]==0)
					sum[0]++;
			}
		}
						
		sum[0] += packet[i];
		if (sum[0] < packet[i]) {
			sum[1]++;
			if (sum[1] == 0) {
				sum[0]++;
				if (sum[0] == 0)
					sum[1]++;
			}
		}
		
		/*SendInt(sum[0]);
		SendString_P(PSTR(","));
		SendInt(sum[1]);
		SendString_P(PSTR("\n"));*/
	}
	
	if (length % 2 == 1)
	{
		/*SendString_P(PSTR("Sum: "));
		SendInt(sum[0]);
		SendString_P(PSTR(","));
		SendInt(sum[1]);
		SendString_P(PSTR(" + "));
		SendInt(packet[i+0]);
		SendString_P(PSTR(","));
		SendInt(packet[i+1]);
		SendString_P(PSTR(" = "));*/
		
		sum[0] += packet[length-1];
		if (sum[0] < packet[length-1]) {
			sum[1]++;
			if (sum[1] == 0) {
				sum[0]++;
				if (sum[0] == 0)
					sum[1]++;
			}
		}
		
		/*SendInt(sum[0]);
		SendString_P(PSTR(","));
		SendInt(sum[1]);
		SendString_P(PSTR("\n"));*/
	}
	
	out[0] = ~sum[0];
	out[1] = ~sum[1];
}