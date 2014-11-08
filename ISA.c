#include <avr/io.h>
#include <compat/ina90.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "ISA.h"

void ISA_Init(void) {
	// reset
	DDRC |= 0b00100000;
	
	// address
	DDRC |= 0b00011111;
	
	// read & write
	DDRB |= 0b11000000; 
	
	PORTB |= 0b10000000; // read = 1
	PORTB |= 0b01000000; // write= 1
}

void ISA_HardwareReset(void) {
	uint16_t i;
	
	PORTC |= 0b00100000;
	
	for (i = 0; i < 10; i++ ) {
		_delay_ms(10);
	}
	
	PORTC &= ~0b00100000;
}

void ISA_Write(uint8_t address, uint8_t data) {
	PORTC = address | (PORTC & ~0b00011111);
	
	// set data bits
	DDRB |= 0b00000111;
	PORTB &= 0b11111000;
	PORTB |= (data & 0b00000111);
	
	DDRD |= 0b11111000;
	PORTD &= 0b00000111;
	PORTD |= (data & 0b11111000);
	
	PORTB &= ~0b01000000; // write = 0
	
	_NOP();
	
	PORTB |= 0b01000000; // write = 1
}

uint8_t ISA_Read(uint8_t address) {
	uint8_t data = 0x00;
		
	DDRB &= ~0b00000111;
	PORTB |= 0b00000111;
	
	DDRD &= ~0b11111000;
	PORTD |= 0b11111000;

	PORTC = address | (PORTC & ~0b00011111);
	
	PORTB &= ~0b10000000; // read = 0
	
	_NOP();

	data |= (PINB & 0b00000111);
	data |= (PIND & 0b11111000);

	PORTB |= 0b10000000; // read = 1
	
	return data;
}