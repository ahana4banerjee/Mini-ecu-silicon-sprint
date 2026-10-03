#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "stm32f4xx.h"

/* =========================================================
   LEDS
   ========================================================= */

#define ORANGE_LED_PORT   GPIOB
#define ORANGE_LED_PIN    0U

#define GREEN_LED_PORT    GPIOB
#define GREEN_LED_PIN     1U

#define BLUE_LED_PORT     GPIOB
#define BLUE_LED_PIN      2U

#define RED_LED_PORT      GPIOB
#define RED_LED_PIN       10U


/* =========================================================
   START
   ========================================================= */

#define START_PORT        GPIOC
#define START_PIN         13U

#define START_ACTIVE_LEVEL  0U


/* =========================================================
   E-STOP
   ========================================================= */

#define ESTOP_PORT        GPIOA
#define ESTOP_PIN         4U

#define ESTOP_ACTIVE_LEVEL  0U


/* =========================================================
   ADC
   ========================================================= */

#define ADC_PORT          GPIOA
#define ADC_PIN           0U


/* =========================================================
   PWM
   ========================================================= */

#define PWM_PORT          GPIOA
#define PWM_PIN           6U


/* =========================================================
   UART2
   ========================================================= */

#define UART_TX_PORT      GPIOA
#define UART_TX_PIN       2U

#define UART_RX_PORT      GPIOA
#define UART_RX_PIN       3U

#endif