#ifndef _NET_COMMON_H_
#define _NET_COMMON_H_

#include <stdio.h>

#define htons(s)        ((s<<8) | (s>>8))
#define htonl(l)        ((l<<24) | ((l&0x00FF0000l)>>8) | ((l&0x0000FF00l)<<8) | (l>>24))

//uint16_t net_CalcChecksum2(uint16_t *packet, uint16_t length, uint8_t ippseudo);

void net_CalcChecksum(uint8_t *packet, uint16_t length, uint8_t ippseudo, uint8_t *sum);

#endif