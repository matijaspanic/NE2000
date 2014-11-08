#ifndef _ISA_H_
#define _ISA_H_

void ISA_Init(void);
void ISA_HardwareReset(void);
void ISA_Write(uint8_t address, uint8_t data);
uint8_t ISA_Read(uint8_t address);

#endif