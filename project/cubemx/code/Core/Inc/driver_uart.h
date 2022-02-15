
#ifndef CORE_INC_DRIVER_UART_H_
#define CORE_INC_DRIVER_UART_H_

#include <stdint.h>

extern void UART_Init();

extern void UART_AsyncTransmitCharacter(char character);
extern void UART_AsyncTransmitString(char const *string);
extern void UART_AsyncTransmitDecimal(uint32_t decimal);

extern char UART_BlockReceiveCharacter();
extern char* UART_BlockReceiveString();
extern uint32_t UART_BlockReceiveDecimal();

#endif /* CORE_INC_DRIVER_UART_H_ */
