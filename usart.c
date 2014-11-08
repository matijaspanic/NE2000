#include <stdlib.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

void USARTInit(uint16_t ubrr_value)
{
	//Set Baud rate
	UBRRL = ubrr_value;
	UBRRH = (ubrr_value>>8);

	//Set Frame Format: Asynchronous mode, No Parity, 1 StopBit, char size 8
	UCSRC=(1<<URSEL)|(3<<UCSZ0);
	UCSRB=(1<<RXEN)|(1<<TXEN);
}

void SendByte (uint8_t data) {
	while(!(UCSRA & (1<<UDRE)));
	UDR=data;
}

void SendString (char *data) {
	while (*data != 0) {
		SendByte(*data++);
	}
}

void SendString_P (const char *data) {
	while (pgm_read_byte(data) != '\0') {
		SendByte(pgm_read_byte(data++));
	}
}


void SendHexByte (uint8_t data) {
	char a[3];
	itoa(data, a, 16);
	if (a[1] == '\0') {
		a[2] = '\0';
		a[1] = a[0];
		a[0] = '0';
	}
	SendString(a);
}

void SendInt (uint16_t data) {
	char a[6];
	itoa(data, a, 10);
	SendString(a);
}

void SendLong (unsigned long data) {
	char a[11];
	ultoa(data, a, 10);
	SendString(a);
}

void SendBits (uint8_t data) {
	char a[9];
	itoa(data, a, 2);
	SendString(a);
}

uint8_t ReceiveByte (uint8_t *byte) {
	if (UCSRA & (1<<RXC)) {
		*byte = UDR;
		return 1;
	}

	return 0;
}
