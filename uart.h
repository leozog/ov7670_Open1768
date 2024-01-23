#pragma once
#include "LPC17xx.h"

void uart_init(LPC_UART_TypeDef *uart);
void uart_send_data(LPC_UART_TypeDef *uart, uint8_t *data, int n);
void uart_send_data_weak(LPC_UART_TypeDef *uart, uint8_t *data, int n);
void uart_send_str(LPC_UART_TypeDef *uart, const char *format, ...);
void uart_send_strln(LPC_UART_TypeDef *uart, const char *format, ...);