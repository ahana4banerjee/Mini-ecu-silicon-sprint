/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : MINI-ECU Main Bare-Metal Application Entry Point
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "board_config.h"
#include "ecu_fsm.h"
#include "gpio_indicators.h"
#include "motor_control.h"

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU System Reset & Flash Latency Initialization */
  HAL_Init();

  /* Configure System Clock to 16 MHz (HSI Oscillator) */
  SystemClock_Config();

  /* Initialize Mini-ECU Application Hardware & Drivers */
  ECU_FSM_Init();        /* Initialize FSM state & GPIOB status LEDs (PB0, PB1, PB2, PB10) */
  GPIO_Init();           /* Initialize PA0 Touch START digital input */
  Motor_Control_Init();  /* Initialize PA1 Potentiometer ADC1 & PA8 TIM1 PWM */

  /* Application Infinite Control Loop */
  while (1)
  {
    ECU_Input input = {0};

    /* 1. Read Touch START digital input (PA0) */
    input.start_pressed = Touch_START_Read();

    /* 2. Read Potentiometer ADC (PA1) and calculate speed percentage */
    uint16_t raw_adc = Potentiometer_Read_ADC();
    uint8_t pwm_perc = ADC_To_PWM_Percent(raw_adc);
    input.pwm_percent = pwm_perc;

    /* 3. Update Finite State Machine transition rules */
    ECU_FSM_Update(&input);

    /* 4. State-gated Motor Drive: Active only in STATE_RUNNING */
    if (ECU_FSM_GetState() == STATE_RUNNING)
    {
      Motor_Set_Speed(pwm_perc);
    }
    else
    {
      Motor_Set_Speed(0U);  /* Force 0% PWM output when not RUNNING */
    }

    /* Polling delay */
    HAL_Delay(20);
  }
}

/**
  * @brief System Clock Configuration (HSI = 16 MHz)
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Configure main regulator output voltage */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  /* Initialize HSI Oscillator */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /* Initialize CPU, AHB and APB bus clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
}
#endif /* USE_FULL_ASSERT */
