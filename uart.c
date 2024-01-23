#include "uart.h"
#include "LPC17xx.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

void uart_init(LPC_UART_TypeDef *uart)
{
	if(uart == LPC_UART0)
		LPC_PINCON->PINSEL0 |= (1<<4) | (1<<6); // P0.2, P0.3
	
	uart->LCR = 3|(1<<7); // Word Length Select: 8-bit // Divisor Latch Access Bit
	uart->DLL = 12; // 115200
	uart->DLM = 0;
	uart->FCR |= (1<<0) | (1<<1) | (1<<2); // FIFO Enable // RX FIFO Reset // TX FIFO Reset
	uart->FDR = (15<<4) | 2; // MULVAL // DIVADDVAL
	uart->LCR = 3;
}

void uart_send_data(LPC_UART_TypeDef *uart, uint8_t *data, int n)
{
	for(int i = 0; i < n; i++)
	{
		while(!(uart->LSR & 1<<5))
			;
		uart->THR = *data;
		++data;
	}
}

void uart_send_weak(LPC_UART_TypeDef *uart, uint8_t *data, int n)
{
	for(int i = 0; i < n; i++)
	{
		if((uart->LSR & 1<<5))
			return;
		uart->THR = *data;
		++data;
	}
}

void uart_send_str(LPC_UART_TypeDef *uart, const char *format, ...)
{
	char str_buffer[64];
	va_list args;
  va_start(args, format);
	vsprintf(str_buffer, format, args);
	uart_send_data(uart, (uint8_t *)str_buffer, strlen(str_buffer));
	va_end(args);
}

void uart_send_strln(LPC_UART_TypeDef *uart, const char *format, ...)
{
	char str_buffer[64];
	va_list args;
  va_start(args, format);
	vsprintf(str_buffer, format, args);
	strcat(str_buffer, "\r\n");
	uart_send_data(uart, (uint8_t *)str_buffer, strlen(str_buffer));
	va_end(args);
}