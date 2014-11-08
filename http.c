#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include "http.h"
#include "usart.h"
#include "lcd.h"

#define HTTP_DEBUG

//char PROGMEM content[256] = "Ovo je ATmega8 Web server....abcdefghabcdefghabcdefghabcdefghabcdefgh";
//char PROGMEM content[256] = "<html><head></head><body>Ovo je ATmega8 Web server bla bla bla bla bla .<br><form method='post'>Text: <input type='text' name='t'></form></body></html>";
//char PROGMEM content[256] = "<html><head><title>ATmega8</title></head><body>Web page  </body></html>";
//char PROGMEM content[256] = "<html><body><form>Text<input type='text' name='a'></form></body></html>";
//char PROGMEM content[256] = "<form >Text: <input type='text' name='a'><br><input type='submit' value='Display'></form>";
char PROGMEM content[256] = "<form>Unesite text:<input type='text' name='a'><input type='submit' value='Display'></form>";

uint16_t net_ProcessHTTP(uint8_t *data_in, uint16_t in_length, uint8_t *data_out) {
	#ifdef HTTP_DEBUG
	SendString_P(PSTR("Processing HTTP:\n"));
	uint16_t i;
	for (i = 0; i < in_length; i++) {
		SendByte(data_in[i]);
	}
	SendString_P(PSTR("\n"));
	#endif
	
	if (*(data_in+5) == '?') {
		uint8_t i = 0;
		char diplay_string[32];
		uint8_t *a = data_in+8;
		while (*a != ' ') {
			if (i==16) {
				diplay_string[i++] = '\n';
			}
			if (*a == '+')
				diplay_string[i] = ' ';
			else
				diplay_string[i] = *a;
			a++;
			i++;
		}
		
		diplay_string[i] = '\0';
		//SendString(diplay_string);
		LcdDisplay(diplay_string);
	}
	
	sprintf_P((char *)data_out, PSTR("HTTP/1.1 200 OK\r\nContent-Length: %i\r\nConnection: Close\r\nContent-Type: text/html; charset=iso-8859-1\r\n\r\n"), (int)strlen_P(content));
	strcat_P((char *)data_out, content);
		
	#ifdef HTTP_DEBUG
	SendString_P(PSTR("\nResponse:\n"));
	SendString((char *)data_out);
	SendString_P(PSTR("\n"));
	#endif

	SendInt(strlen((char *)data_out));
	return strlen((char *)data_out);
}