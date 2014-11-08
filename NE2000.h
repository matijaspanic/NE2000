#ifndef _NE2000_H_
#define _NE2000_H_

void ne2000_Init(void);
void ne2000_SendPacket(uint8_t *packet, uint16_t length);
uint16_t ne2000_ReceivePacket(uint8_t *packet, uint16_t max_size);

#endif