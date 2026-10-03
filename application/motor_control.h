#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <stdint.h>
#include "board_config.h"

/**
 * @brief Initializes ADC1 (PA1) for potentiometer sampling 
 *        and TIM1 CH1 (PA8) for motor PWM speed control.
 */
void Motor_Control_Init(void);

/**
 * @brief Performs a software-triggered ADC conversion on PA1 (ADC1_IN1).
 * @retval 12-bit ADC raw value (0 to 4095).
 */
uint16_t Potentiometer_Read_ADC(void);

/**
 * @brief Converts raw 12-bit ADC reading into motor PWM percentage (0% to 100%).
 *        Includes deadband filtering for 0% and 100% bounds.
 * @param adc_val Raw 12-bit ADC value (0-4095).
 * @retval Calculated PWM percentage (0-100%).
 */
uint8_t ADC_To_PWM_Percent(uint16_t adc_val);

/**
 * @brief Sets the motor PWM duty cycle output on PA8 (TIM1_CH1).
 * @param pwm_percent Speed percentage (0 to 100%).
 */
void Motor_Set_Speed(uint8_t pwm_percent);

#endif /* MOTOR_CONTROL_H */
