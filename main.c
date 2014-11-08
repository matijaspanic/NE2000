#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "main.h"

#include "usart.h"
#include "ne2000.h"
#include "transport.h"
#include "network.h"
#include "ethernet.h"
#include "net_common.h"
#include "lcd.h"

//////////////
//// DATA ////
//////////////

uint8_t net_packet[MTU_SIZE];

char command[32] = "";
uint8_t command_length = 0;

void ReceiveCommand (void) {
	uint8_t byte;
	if (ReceiveByte(&byte))	{	
		if (byte=='\n') {
			if (strcmp(command, "ping")==0) {
				/*SendString_P(PSTR("Pinging:"));
				uint8_t dest_ip[4] = {192, 168, 1, 5};
				net_SendIp(dest_ip);*/
			}
			
			strcpy(command, "");
			command_length = 0;
		}
		else {
			command[command_length++] = byte;
			command[command_length] = '\0';
		}
	}
}

int main(void)
{	
	USARTInit(12); // 38400
	
	_delay_ms(10);
	SendString_P(PSTR("\nMicro started.\n"));
		
	LcdInit();
	ne2000_Init();
	
	LcdDisplay("ATmega8 mini Web\nserver");
	
	while(1) {
		ReceiveCommand();
		net_Receive();
	}

	return 0;
}
