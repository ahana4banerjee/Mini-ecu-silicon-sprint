#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "stm32f4xx.h"

/* ===================================================================
 * MINI-ECU HARDWARE BOARD CONFIGURATION & PIN MAPPING
 * Target MCU: STM32F401RET6
 * =================================================================== */

/* -------------------------------------------------------------------
 * Status Indicator LEDs (GPIOB)
 * Matches teammate FSM PR implementation
 * ------------------------------------------------------------------- */
#define LED_PORT                GPIOB
#define LED_PORT_CLK_ENABLE()   (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN)

#define ORANGE_LED_PIN          0U    /* PB0  - IDLE State LED (Orange) */
#define GREEN_LED_PIN           1U    /* PB1  - READY State LED (Green) */
#define BLUE_LED_PIN            2U    /* PB2  - RUNNING State LED (Blue) */
#define RED_LED_PIN             10U   /* PB10 - EMERGENCY State LED (Red) */

/* -------------------------------------------------------------------
 * Touch START Digital Input (GPIOA - Phase 1)
 * ------------------------------------------------------------------- */
#define TOUCH_START_PORT        GPIOA
#define TOUCH_START_PIN         0U    /* PA0  - Touch START Sensor Input */
#define TOUCH_START_CLK_ENABLE()(RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN)

/* -------------------------------------------------------------------
 * Emergency Stop EXTI Interrupt Input (GPIOC - Phase 4)
 * ------------------------------------------------------------------- */
#define ESTOP_PORT              GPIOC
#define ESTOP_PIN               13U   /* PC13 - Emergency Stop Button (EXTI15_10) */
#define ESTOP_CLK_ENABLE()      (RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN)
#define ESTOP_EXTI_IRQn         EXTI15_10_IRQn

/* -------------------------------------------------------------------
 * Potentiometer Analog Input (GPIOA - Phase 2)
 * ------------------------------------------------------------------- */
#define POT_ADC_PORT            GPIOA
#define POT_ADC_PIN             1U    /* PA1  - ADC1 Channel 1 (Potentiometer) */
#define POT_ADC_CHANNEL         1U

/* -------------------------------------------------------------------
 * Motor Drive PWM Output (GPIOA - Phase 2)
 * ------------------------------------------------------------------- */
#define MOTOR_PWM_PORT          GPIOA
#define MOTOR_PWM_PIN           8U    /* PA8  - TIM1 Channel 1 PWM Output */

/* -------------------------------------------------------------------
 * Serial Communication UART (GPIOA - Phase 3)
 * ------------------------------------------------------------------- */
#define ECU_UART_PORT           GPIOA
#define ECU_UART_TX_PIN         2U    /* PA2  - USART2_TX (ST-LINK Virtual COM) */
#define ECU_UART_RX_PIN         3U    /* PA3  - USART2_RX */

#endif /* BOARD_CONFIG_H */
