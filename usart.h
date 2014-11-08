void USARTInit(uint16_t ubrr_value);
void SendString (char *data);
void SendString_P (const char *data);
void SendByte (uint8_t data);
void SendHexByte (uint8_t data);
void SendInt (int data);
void SendLong (uint32_t data);
void SendBits (uint8_t data);
uint8_t ReceiveByte (uint8_t *byte);