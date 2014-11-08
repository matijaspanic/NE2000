#include <avr/io.h>
#include <string.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#define LCD_DELAY   asm volatile ("nop\n nop\n nop\n nop\n nop\n nop\n nop\n");

#define E 0x04

#define PCM 0b00011111

#define RS 0x10
#define DB4 0x08
#define DB5 0x04
#define DB6 0x02
#define DB7 0x01

void write (uint8_t bits) {
	PORTD |= E;
	//_delay_ms(1);
	PORTC = (PORTC & ~PCM) | bits;
	_delay_ms(1);
	PORTD &= ~E;
	_delay_ms(1);
}

void LcdInit (void) {
	DDRD |= E;
	DDRC |= 0b00011111;
	
	// clear all bits
	PORTC &= ~PCM;
	
	_delay_ms(20);
	
	write(DB4 | DB5);
	write(DB4 | DB5);
	write(DB4 | DB5);
	
	write(DB5);
	
	// Specify display lines and character font
	write(DB5);
	write(DB7);
	
	// Turns on display and cursor
	write(0);
	write(DB5 | DB6 | DB7);
	
	// Sets mode to increment the address by one and to shift the cursor to the right at the time of write to the DD/CGRAM. Display is not shifted.
	/*write(0);
	write(DB5 | DB6);*/
	
	// clear display
	write(0);
	write(DB4);
}

void LcdDisplay (char *text) {
	// clear display
	write(0);
	write(DB4);
	
	// set cursor to start
	write(0);
	write(DB5);

	unsigned int i = 0;
	for (; i < strlen(text); i++ ) {
		if (text[i] == '\n') {
			write(DB7 | DB6);
			write(0);
		}
		else
		{
			uint8_t c = text[i];
			write(RS | (c & 0x10 ? DB4 : 0) | (c & 0x20 ? DB5 : 0) | (c & 0x40 ? DB6 : 0) | (c & 0x80 ? DB7 : 0));
			write(RS | (c & 0x01 ? DB4 : 0) | (c & 0x02 ? DB5 : 0) | (c & 0x04 ? DB6 : 0) | (c & 0x08 ? DB7 : 0));
		}			
	}
}