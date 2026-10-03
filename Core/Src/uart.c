#include "stm32f4xx.h"
#include "uart.h"
#include "ecu_fsm.h"
#include "board_config.h"
#include <stdint.h>
#include <string.h>

/* =========================================================
   UART COMMAND FLAGS
   ========================================================= */

static volatile uint8_t cmd_forward = 0;
static volatile uint8_t cmd_stop    = 0;
static volatile uint8_t cmd_reset   = 0;
static volatile uint8_t cmd_status  = 0;


/* =========================================================
   RX BUFFER
   ========================================================= */

static char rx_buffer[20];
static uint8_t rx_index = 0;


/* =========================================================
   TX BUFFER
   ========================================================= */

#define TX_BUFFER_SIZE 256

static volatile char tx_buffer[TX_BUFFER_SIZE];
static volatile uint16_t tx_head = 0;
static volatile uint16_t tx_tail = 0;


/* =========================================================
   UART2 INITIALIZATION
   PA2 -> TX
   PA3 -> RX
   AF7 -> USART2
   ========================================================= */

void UART2_Init(void)
{
    /* Enable GPIOA clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    /* Enable USART2 clock */
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;


    /* -----------------------------------------------------
       PA2 and PA3 -> Alternate Function mode
       MODER = 10
       ----------------------------------------------------- */

    GPIOA->MODER &= ~(
          (3U << (UART_TX_PIN * 2))
        | (3U << (UART_RX_PIN * 2))
    );

    GPIOA->MODER |= (
          (2U << (UART_TX_PIN * 2))
        | (2U << (UART_RX_PIN * 2))
    );


    /* -----------------------------------------------------
       PA2 and PA3 -> AF7 = USART2
       ----------------------------------------------------- */

    GPIOA->AFR[0] &= ~(
          (0xFU << (UART_TX_PIN * 4))
        | (0xFU << (UART_RX_PIN * 4))
    );

    GPIOA->AFR[0] |= (
          (7U << (UART_TX_PIN * 4))
        | (7U << (UART_RX_PIN * 4))
    );


    /* -----------------------------------------------------
       USART configuration

       Assuming:
       PCLK1 = 42 MHz
       Baud rate = 115200
       Oversampling = 16

       BRR = 0x16D
       ----------------------------------------------------- */

    USART2->BRR = 0x16D;


    /* Enable transmitter */
    USART2->CR1 |= USART_CR1_TE;

    /* Enable receiver */
    USART2->CR1 |= USART_CR1_RE;

    /* Enable USART */
    USART2->CR1 |= USART_CR1_UE;
}


/* =========================================================
   INTERNAL TX BUFFER FUNCTION
   ========================================================= */

static void UART2_QueueChar(char c)
{
    uint16_t next_head;

    next_head = (tx_head + 1) % TX_BUFFER_SIZE;

    /* Only add if buffer is not full */
    if (next_head != tx_tail)
    {
        tx_buffer[tx_head] = c;
        tx_head = next_head;
    }
}


/* =========================================================
   SEND CHARACTER
   Non-blocking
   ========================================================= */

void UART2_SendChar(char c)
{
    UART2_QueueChar(c);
}


/* =========================================================
   SEND STRING
   ========================================================= */

void UART2_SendString(const char *str)
{
    while (*str)
    {
        UART2_QueueChar(*str);
        str++;
    }
}


/* =========================================================
   TX TASK
   Sends one character whenever TXE is available
   ========================================================= */

void UART2_TxTask(void)
{
    if (tx_tail == tx_head)
    {
        return;
    }

    if (USART2->SR & USART_SR_TXE)
    {
        USART2->DR = tx_buffer[tx_tail];

        tx_tail = (tx_tail + 1) % TX_BUFFER_SIZE;
    }
}


/* =========================================================
   RECEIVE / COMMAND PARSER
   NON-BLOCKING
   ========================================================= */

void UART2_ProcessCommand(void)
{
    char c;

    /*
     * Check whether data has arrived.
     * If not, immediately return.
     */
    if (!(USART2->SR & USART_SR_RXNE))
    {
        return;
    }

    /* Read received character */
    c = (char)(USART2->DR & 0xFF);


    /* -----------------------------------------------------
       ENTER key -> command is complete
       ----------------------------------------------------- */

    if ((c == '\r') || (c == '\n'))
    {
        rx_buffer[rx_index] = '\0';


        /* F command */
        if (strcmp(rx_buffer, "F") == 0)
        {
            cmd_forward = 1;
        }


        /* S command */
        else if (strcmp(rx_buffer, "S") == 0)
        {
            cmd_stop = 1;
        }


        /* RESET command */
        else if (strcmp(rx_buffer, "RESET") == 0)
        {
            cmd_reset = 1;
        }


        /* STATUS command */
        else if (strcmp(rx_buffer, "STATUS") == 0)
        {
            cmd_status = 1;
        }


        /* Clear buffer for next command */
        rx_index = 0;
    }


    /* -----------------------------------------------------
       Normal character -> store in buffer
       ----------------------------------------------------- */

    else
    {
        if (rx_index < sizeof(rx_buffer) - 1)
        {
            rx_buffer[rx_index] = c;
            rx_index++;
        }
    }
}


/* =========================================================
   COMMAND GETTER: F
   Returns 1 ONCE when command was received
   Then clears the event
   ========================================================= */

uint8_t UART_GetForwardCommand(void)
{
    uint8_t event;

    event = cmd_forward;
    cmd_forward = 0;

    return event;
}


/* =========================================================
   COMMAND GETTER: S
   ========================================================= */

uint8_t UART_GetStopCommand(void)
{
    uint8_t event;

    event = cmd_stop;
    cmd_stop = 0;

    return event;
}


/* =========================================================
   COMMAND GETTER: RESET
   ========================================================= */

uint8_t UART_GetResetCommand(void)
{
    uint8_t event;

    event = cmd_reset;
    cmd_reset = 0;

    return event;
}


/* =========================================================
   INTEGER -> STRING HELPER
   Avoids modifying the original ADC value
   ========================================================= */

static void UART_SendUInt16(uint16_t value)
{
    char digits[6];
    uint8_t i = 0;

    if (value == 0)
    {
        UART2_SendChar('0');
        return;
    }

    while (value > 0)
    {
        digits[i++] = '0' + (value % 10);
        value = value / 10;
    }

    while (i > 0)
    {
        UART2_SendChar(digits[--i]);
    }
}


/* =========================================================
   SEND STATE NAME
   Uses ECU_FSM_GetState()
   ========================================================= */

static void UART_SendStateName(ECU_State state)
{
    switch (state)
    {
        case STATE_IDLE:
            UART2_SendString("IDLE");
            break;

        case STATE_READY:
            UART2_SendString("READY");
            break;

        case STATE_RUNNING:
            UART2_SendString("RUNNING");
            break;

        case STATE_EMERGENCY:
            UART2_SendString("EMERGENCY");
            break;

        default:
            UART2_SendString("UNKNOWN");
            break;
    }
}


/* =========================================================
   STATUS
   ========================================================= */

void UART_SendStatus(uint16_t adc_value, uint8_t pwm_percent)
{
    ECU_State state;

    /* Get state through FSM interface */
    state = ECU_FSM_GetState();


    /* STATE */
    UART2_SendString("\r\nSTATE: ");
    UART_SendStateName(state);
    UART2_SendString("\r\n");


    /* SYSTEM */
    UART2_SendString("SYSTEM: ");

    if (state == STATE_EMERGENCY)
    {
        UART2_SendString("EMERGENCY");
    }
    else if (state == STATE_RUNNING)
    {
        UART2_SendString("ACTIVE");
    }
    else
    {
        UART2_SendString("IDLE");
    }

    UART2_SendString("\r\n");


    /* MOTOR */
    UART2_SendString("MOTOR: ");

    if (state == STATE_RUNNING)
    {
        UART2_SendString("ON");
    }
    else
    {
        UART2_SendString("OFF");
    }

    UART2_SendString("\r\n");


    /* PWM */
    UART2_SendString("PWM: ");
    UART_SendUInt16(pwm_percent);
    UART2_SendString("%\r\n");


    /* ADC */
    UART2_SendString("ADC: ");

    /*
     * IMPORTANT:
     * We do not modify adc_value.
     *
     * UART_SendUInt16() receives a copy of adc_value
     * as its parameter, so the original ADC value remains unchanged.
     */
    UART_SendUInt16(adc_value);

    UART2_SendString("\r\n");


    /* EMERGENCY */
    UART2_SendString("EMERGENCY: ");

    if (state == STATE_EMERGENCY)
    {
        UART2_SendString("YES");
    }
    else
    {
        UART2_SendString("NO");
    }

    UART2_SendString("\r\n\r\n");
}


/* =========================================================
   STATUS COMMAND TASK
   ========================================================= */

void UART2_StatusTask(uint16_t adc_value, uint8_t pwm_percent)
{
    if (cmd_status)
    {
        cmd_status = 0;

        UART_SendStatus(adc_value, pwm_percent);
    }
}