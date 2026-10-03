#include "motor_control.h"

/* ===================================================================
 * PERIPHERAL INITIALIZATION (ADC1 & TIM1 PWM)
 * =================================================================== */

void Motor_Control_Init(void)
{
    /* 1. Enable Clocks for GPIOA, ADC1, and TIM1 */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    /* ---------------------------------------------------------------
     * PA1 (Potentiometer ADC1_IN1) Configuration - Analog Mode
     * --------------------------------------------------------------- */
    POT_ADC_PORT->MODER |= (3U << (POT_ADC_PIN * 2));     /* 11: Analog mode */
    POT_ADC_PORT->PUPDR &= ~(3U << (POT_ADC_PIN * 2));    /* 00: No pull-up/pull-down */

    /* ---------------------------------------------------------------
     * PA8 (Motor PWM TIM1_CH1) Configuration - Alternate Function (AF1)
     * --------------------------------------------------------------- */
    MOTOR_PWM_PORT->MODER &= ~(3U << (MOTOR_PWM_PIN * 2));
    MOTOR_PWM_PORT->MODER |=  (2U << (MOTOR_PWM_PIN * 2));   /* 10: Alternate Function */

    /* Set AF1 for PA8 (TIM1_CH1) in AFR[1] (High Register) */
    MOTOR_PWM_PORT->AFR[1] &= ~(0xFU << ((MOTOR_PWM_PIN - 8U) * 4));
    MOTOR_PWM_PORT->AFR[1] |=  (1U   << ((MOTOR_PWM_PIN - 8U) * 4));  /* 0001: AF1 */

    /* ---------------------------------------------------------------
     * ADC1 Setup (12-bit, Single Conversion, Software Trigger)
     * --------------------------------------------------------------- */
    ADC1->CR1 = 0U;                                        /* 12-bit resolution */
    ADC1->CR2 = 0U;                                        /* Right alignment, single mode */

    /* Set sample time for Channel 1 to 84 cycles for stable sampling */
    ADC1->SMPR2 &= ~(7U << (POT_ADC_CHANNEL * 3));
    ADC1->SMPR2 |=  (4U << (POT_ADC_CHANNEL * 3));         /* 100: 84 cycles */

    /* Select Channel 1 as 1st sequence conversion */
    ADC1->SQR3 = POT_ADC_CHANNEL;

    /* Turn ADC1 ON */
    ADC1->CR2 |= ADC_CR2_ADON;

    /* ---------------------------------------------------------------
     * TIM1 PWM Setup (1 kHz Frequency, PWM Mode 1 on CH1)
     * System Clock = 16 MHz (default HSI)
     * --------------------------------------------------------------- */
    TIM1->PSC = 15U;                                       /* 16 MHz / 16 = 1 MHz clock (1 us tick) */
    TIM1->ARR = 999U;                                      /* Period = 1000 us = 1 ms (1 kHz PWM) */
    TIM1->CCR1 = 0U;                                       /* Initial 0% Duty Cycle (Motor OFF) */

    /* Configure PWM Mode 1 on Channel 1 (OC1M = 110) */
    TIM1->CCMR1 &= ~TIM_CCMR1_OC1M;
    TIM1->CCMR1 |=  (6U << TIM_CCMR1_OC1M_Pos);
    TIM1->CCMR1 |=  TIM_CCMR1_OC1PE;                       /* Output compare 1 preload enable */

    /* Enable Channel 1 Output & Preload */
    TIM1->CCER |= TIM_CCER_CC1E;
    TIM1->CR1  |= TIM_CR1_ARPE;

    /* Main Output Enable (MOE) required for TIM1 Advanced Timer */
    TIM1->BDTR |= TIM_BDTR_MOE;

    /* Start Timer Counter */
    TIM1->CR1 |= TIM_CR1_CEN;
}

/* ===================================================================
 * ADC SAMPLING
 * =================================================================== */

uint16_t Potentiometer_Read_ADC(void)
{
    /* Select Channel 1 */
    ADC1->SQR3 = POT_ADC_CHANNEL;

    /* Start software conversion */
    ADC1->CR2 |= ADC_CR2_SWSTART;

    /* Poll EOC (End Of Conversion) flag */
    uint32_t timeout = 10000U;
    while (!(ADC1->SR & ADC_SR_EOC) && --timeout);

    /* Return 12-bit conversion value */
    return (uint16_t)ADC1->DR;
}

/* ===================================================================
 * ADC TO PWM PERCENT CONVERSION
 * =================================================================== */

uint8_t ADC_To_PWM_Percent(uint16_t adc_val)
{
    /* Deadband noise filtering for lower/upper bounds */
    if (adc_val < 40U)
    {
        return 0U;
    }
    if (adc_val > 4050U)
    {
        return 100U;
    }

    /* Map 0-4095 linearly to 0%-100% */
    uint32_t percent = ((uint32_t)adc_val * 100U) / 4095U;
    return (uint8_t)percent;
}

/* ===================================================================
 * MOTOR PWM SPEED CONTROL
 * =================================================================== */

void Motor_Set_Speed(uint8_t pwm_percent)
{
    if (pwm_percent > 100U)
    {
        pwm_percent = 100U;
    }

    /* Convert percentage (0-100%) to CCR1 compare value (0-999) */
    uint32_t compare_val = ((uint32_t)pwm_percent * 999U) / 100U;
    TIM1->CCR1 = compare_val;
}
