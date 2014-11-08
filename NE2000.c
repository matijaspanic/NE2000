#include <stdlib.h>
#include <stdio.h>
#include <avr/pgmspace.h>

#include "main.h"
#include "usart.h"
#include "ISA.h"

//////////////////////
// NE2000 definitions
//////////////////////

// REGISTERS
#define CR				0x00 // Command Register
#define RDMA			0x10 // Remote DMA Port
#define RSTPORT 		0x18 // Reset Port
					
// page 0
#define PSTART			0x01 // Page Start Register (W) - read in page2
#define PSTOP			0x02 // Page Stop Register (W) - read in page 2
#define BNRY			0x03 // Boundary Register (R/W)
#define TSR 			0x04 // Transmit Status Register (R)
#define TPSR			0x04 // Transmit Page Start Register (W) - read in page 2
#define TBCR0			0x05 // Transmit Byte Count Registers (W)
#define TBCR1			0x06 // Transmit Byte Count Registers (W)
#define ISR 			0x07 // Interrupt Status Register (R/W)
#define CRDA0			0x08 // Current Remote DMA Address registers (R)
#define CRDAl			0x09 // Current Remote DMA Address registers (R)
#define RSAR0			0x08 // Remote Start Address Registers (W)
#define RSAR1			0x09 // Remote Start Address Registers (W)
#define RBCR0			0x0A // Remote Byte Count Registers (W)
#define RBCR1			0x0B // Remote Byte Count Registers (W)
#define RSR 			0x0C // Receive Status Register (R)
#define RCR 			0x0C // Receive Configuration Register (W) - read in page 2
#define CNTR0			0x0D // Frame Alignment Error Tally Counter Register (R)
#define TCR 			0x0D // Transmit Configuration Register (W) - read in page 2
#define CNTR1			0x0E // CRC Error Tally Counter Register (R)
#define DCR 			0x0E // Data Configuration Register (W) - read in page 2
#define CNTR2			0x0F // Missed Packet Tally Counter Register (R)
#define IMR 			0x0F // Interrupt Mask Register (W) - read in page 2

// page 1				
#define PAR0			0x01 // Physical Address Registers (R/W)
#define PAR1			0x02
#define PAR2			0x03
#define PAR3			0x04
#define PAR4			0x05
#define PAR5			0x06
#define CURR			0x07 // Current Page Register (R/W)
							 // This register points to the page address of the first receive buffer page to be used for a packet reception.


// COMMAND REGISTER BITS
#define CR_STOP 		0x01	// Stop (Reset) NIC
#define CR_START		0x02	// Start NIC
#define CR_TRANSMIT 	0x04	// Must be 1 to transmit packet
// only one of next four can be used at the same time
#define CR_DMAREAD		0x08	// remote DMA read
#define CR_DMAWRITE 	0x10	// remote DMA write */
#define CR_SENDPACKET	0x18	// send packet
#define CR_NODMA		0x20	/* abort/complete remote DMA */
// 
#define CR_PAGE0		0x00	/* select register page 0 */
#define CR_PAGE1		0x40	/* select register page 1 */
#define CR_PAGE2		0x80	/* select register page 2 */

// ISR Register Bits
#define ISR_PRX 		0x01	// indicates packet received with no errors   
#define ISR_PTX 		0x02	// indicates packet transmitted with no error
#define ISR_RXE 		0x04	// set when a packet received with one or more of the following errors: CRC error, Frame alignment error, Missed packet 
#define ISR_TXE 		0x08	// set when a packet transmission is aborted due to excessive collisions
#define ISR_OVW 		0x10	// set when the receive buffer has been exhausted
#define ISR_CNT 		0x20	// Set when MSB of one or more of the network tally counters has been set
#define ISR_RDC 		0x40	// Set when remote DMA operation has been completed
#define ISR_RST 		0x80	// set when NIC enters reset state and is cleared when a start command is issued to the CR
								// It is also set when receive buffer overflows and is cleared when one or more packets have been read from the buffer

/* For the Data Control Register */
#define DCR_BYTEDMA 0x00
#define DCR_WORDDMA 0x01
#define DCR_NOLPBK	0x08
#define DCR_ARM 	0x10
#define DCR_FIFO2	0x00
#define DCR_FIFO4	0x20
#define DCR_FIFO8	0x40
#define DCR_FIFO12	0x60

/* For the Receive Control Register */
#define RCR_AB		0x04
#define RCR_AM		0x08
#define RCR_PROMISCUOUS 0x10
#define RCR_MONITOR 0x20

/* For the Transmission Control Register */
#define TCR_NOLPBK	0x00
#define TCR_INTLPBK 0x02
#define TCR_EXTLPBK 0x04
#define TCR_EXTLPBK2	0x06

#define TXSTART 0x40	// Start of TX buffers
#define RXSTART 0x46	// Start of RX buffers
#define RXSTOP	0x60	// End of RX buffers

//////////////////////
// NE2000 data
//////////////////////

extern uint8_t PROGMEM net_mac[6];

//////////////////////
// code
//////////////////////

void ne2000_Init(void) {
	ISA_Init();

	ISA_HardwareReset();
	
	ISA_Write(CR, (CR_PAGE0|CR_NODMA|CR_STOP)); // set page 0
	_delay_ms(2);					 // wait for traffic to complete
	ISA_Write(DCR, DCR_BYTEDMA|DCR_NOLPBK|DCR_ARM);
	ISA_Write(RBCR0,0x00);
	ISA_Write(RBCR1,0x00);
	ISA_Write(RCR, RCR_AB);
	ISA_Write(TCR, TCR_INTLPBK); 	// set internal lookback

	ISA_Write(TPSR, TXSTART);			// set the start page address of the packet to the transmitted.
	ISA_Write(PSTART, RXSTART);		// set the start page address of the receive buffer ring
	ISA_Write(BNRY, RXSTART);			// set Boundary Register	
	ISA_Write(PSTOP, RXSTOP);			// set the stop page address of the receive buffer ring 
		
	ISA_Write(CR, (CR_PAGE0|CR_NODMA|CR_STOP));
	ISA_Write(DCR, DCR_FIFO8|DCR_NOLPBK|DCR_ARM);
	ISA_Write(CR, (CR_NODMA|CR_START));
	ISA_Write(ISR,0xFF); 			// clear all interrupts
	ISA_Write(IMR, 0x00);			// no interupts
	ISA_Write(TCR, 0x00);			// normal operation
	
	/*
	// read mac from eeprom
	ISA_Write(RBCR0, 32); // Intend to read 32 Bytes
	ISA_Write(RBCR1, 0);
	ISA_Write(RSAR0, 0); // low byte of start address (0x0000)
	ISA_Write(RSAR1, 0); // high byte of start address
	ISA_Write(CR, CR_PAGE0 | CR_START | CR_DMAREAD);
	// for some reason, 2 reads are required, otherwise you get duplicate
	// values. the comments in the linux driver talk about values being
	// "doubled up", but i don't know why. whatever - it works this way
	// and i don't have time to investigate :)
	for (i = 0; i < 6; i++) {
		ISA_Read(RDMA);
		net_mac[i] = ISA_Read(RDMA);
	}
	// end (abort) the DMA transfer
	ISA_Write(CR, CR_PAGE0 | CR_START | CR_NODMA);
	*/
	
	
	ISA_Write(CR, (CR_PAGE1|CR_NODMA|CR_STOP)); // switch to page 1
	
	// write mac
	ISA_Write(PAR0, pgm_read_byte(net_mac));
	ISA_Write(PAR1, pgm_read_byte(net_mac+1));
	ISA_Write(PAR2, pgm_read_byte(net_mac+2));
	ISA_Write(PAR3, pgm_read_byte(net_mac+3));
	ISA_Write(PAR4, pgm_read_byte(net_mac+4));
	ISA_Write(PAR5, pgm_read_byte(net_mac+5));
	
	/*
	SendString_P(PSTR("MAC: "));
	for (i = 0; i < 6; i++ ) {
		if (i>0)
			SendByte(':');
		SendHexByte(net_mac[i]);
	}
	SendByte('\n');
	*/
	
	ISA_Write(CURR, RXSTART); // init curr pointer

	ISA_Write(CR, (CR_NODMA|CR_START));	// start the NIC
}

void ne2000_SendPacket(uint8_t *packet, uint16_t length) {
	uint16_t i;
	
	if (length < 0x40) { length = 0x40; }
	i = 0;
	// If there is still a packet in transmission, wait until it's done!
	while ((i < 50000) && (ISA_Read(CR) & CR_TRANSMIT)) {
		#ifdef _USART_DEBUG
		SendString_P(PSTR("NE2000: still a packet in transmission\n"));
		#endif
		i++;
	}
	/* Abort any currently running "DMA" operations */
	ISA_Write(CR, CR_PAGE0 | CR_START | CR_NODMA);
	ISA_Write(RBCR0, length & 0xff);
	ISA_Write(RBCR1, length >> 8);
	ISA_Write(RSAR0, 0x00);
	ISA_Write(RSAR1, TXSTART);
	ISA_Write(CR, CR_PAGE0 | CR_START | CR_DMAWRITE);
	for (i = 0; i < length; i++) {
		ISA_Write(RDMA, packet[i]);
	}
	// Wait for something here?
	ISA_Write(CR, CR_PAGE0 | CR_START | CR_NODMA);
	ISA_Write(TBCR0, length & 0xff);
	ISA_Write(TBCR1, length >> 8);
	ISA_Write(TPSR, TXSTART);
	ISA_Write(CR, CR_PAGE0 | CR_START | CR_TRANSMIT);
}

uint16_t ne2000_ReceivePacket(uint8_t *packet) {
	uint8_t curr, bnry;
	uint16_t i = 0;
	
	uint16_t net_recvdbytes;
	
	ISA_Write(CR, CR_PAGE1 | CR_START | CR_NODMA);	// goto register page 1
	curr = ISA_Read(CURR);	// read the CURRent pointer
	
	ISA_Write(CR, CR_PAGE0 | CR_START | CR_NODMA);	// goto register page 0
	
	// read the boundary register pointing to the beginning of the packet
	bnry = ISA_Read(BNRY);
	
	if( bnry == curr )
		return 0;
		
	// if boundary pointer is invalid
	if( (bnry >= RXSTOP) || (bnry < RXSTART) )
	{
		// reset the contents of the buffer and exit
		ISA_Write(BNRY, RXSTART);
		ISA_Write(CR, (CR_PAGE1|CR_NODMA|CR_START));
		ISA_Write(CURR, RXSTART);
		ISA_Write(CR, (CR_NODMA|CR_START));
		return 0;
	}
		
	if (bnry > RXSTOP - 1) { 
		bnry = RXSTART;
	}	

	ISA_Write(RBCR0, 0xff);
	ISA_Write(RBCR1, 0xff);
	ISA_Write(RSAR0, 0); // low byte of start address (0)
	ISA_Write(RSAR1, bnry); // high byte of start address (BNRY)
	
	// begin the dma read
	ISA_Write(CR, CR_PAGE0 | CR_START | CR_DMAREAD);
	
	ISA_Read(RDMA);
	
	bnry = ISA_Read(RDMA); // next-pointer
	if (bnry < RXSTART) {
		bnry = RXSTOP;
	};
	
	net_recvdbytes = ISA_Read(RDMA);
	net_recvdbytes += (ISA_Read(RDMA)<<8);
	
	if (net_recvdbytes > MTU_SIZE) {
		net_recvdbytes = MTU_SIZE;
	}
	
	for (i = 0; i < net_recvdbytes; i++) {
		packet[i] = ISA_Read(RDMA);
	}
	
	ISA_Write(CR, CR_PAGE0 | CR_START | CR_NODMA); // set page 0
	ISA_Write(BNRY, bnry); // write updated bnry pointer
	
	return net_recvdbytes;
}