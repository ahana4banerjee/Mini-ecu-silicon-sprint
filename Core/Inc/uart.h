#ifndef UART_H
#define UART_H

#include <stdint.h>

/* UART initialization */
void UART2_Init(void);

/* Non-blocking UART command processing */
void UART2_ProcessCommand(void);

/* Non-blocking TX task */
void UART2_TxTask(void);

/* Send functions */
void UART2_SendChar(char c);
void UART2_SendString(const char *str);

/* Command event getters */
uint8_t UART_GetForwardCommand(void);
uint8_t UART_GetStopCommand(void);
uint8_t UART_GetResetCommand(void);

/* STATUS */
void UART2_StatusTask(uint16_t adc_value, uint8_t pwm_percent);
void UART_SendStatus(uint16_t adc_value, uint8_t pwm_percent);

#endif