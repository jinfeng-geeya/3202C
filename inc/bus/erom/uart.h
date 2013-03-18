#ifndef _UART_H_
#define _UART_H_

#define UART_SPEED_NORMAL   115200	
#define UART_HIGH_SPEED_2M  0x200000
#define UART_HIGH_SPEED_6M  0x600000

void uart_high_speed_init(unsigned long baudrate);
unsigned long uart_high_speed_get(void);
int uart_high_speed_config(unsigned long baudrate);

#endif /*_UART_H_*/
