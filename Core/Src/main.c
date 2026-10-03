#include "main.h"

#include "ecu_fsm.h"
#include "gpio.h"
#include "uart.h"

/* Private function prototypes */
void SystemClock_Config(void);


/* =========================================================
   MAIN
   ========================================================= */

int main(void)
{
    ECU_Input input;

    /*
     * These are the values that Member 2's ADC/PWM code
     * will provide.
     *
     * Keep them here temporarily until Member 2 gives
     * the actual interface.
     */
    uint16_t adc_value = 0U;
    uint8_t pwm_percent = 0U;


    /* -----------------------------------------------------
       Basic MCU initialization
       ----------------------------------------------------- */

    HAL_Init();

    SystemClock_Config();


    /* -----------------------------------------------------
       Peripheral initialization
       ----------------------------------------------------- */

    GPIO_Init();
    EXTI_Init();

    UART2_Init();

    ECU_FSM_Init();


    /* -----------------------------------------------------
       Initial LED state
       IDLE -> Orange ON
       ----------------------------------------------------- */

    GPIO_UpdateLEDs(ECU_FSM_GetState());


    /* =====================================================
       MAIN APPLICATION LOOP
       ===================================================== */

    while (1)
    {
        /*
         * Start every loop with a clean event structure.
         *
         * Events are momentary:
         * START, F, S and RESET are consumed once.
         */
        input.start_pressed = 0U;
        input.f_received = 0U;
        input.s_received = 0U;
        input.reset_received = 0U;
        input.emergency_active = 0U;
        input.pwm_percent = pwm_percent;


        /* =================================================
           1. ADC / PWM
           =================================================

           MEMBER 2:
           Replace/update this section with the actual
           ADC/PWM interface.

           For now:
               adc_value    = ...
               pwm_percent  = ...
        */


        /* =================================================
           2. UART RECEIVE
           ================================================= */

        UART2_ProcessCommand();


        /* =================================================
           3. PHYSICAL INPUTS
           ================================================= */

        input.start_pressed =
            GPIO_ReadSTARTPressedEvent();


        /*
         * E-STOP is checked as a LEVEL as well as an
         * interrupt event.
         *
         * This is intentional:
         *
         *   Level -> remains emergency while button active
         *   Event -> captures the EXTI emergency event
         */
        input.emergency_active =
            (uint8_t)(
                GPIO_ReadESTOPActive() ||
                GPIO_TakeESTOPEvent()
            );


        /* =================================================
           4. UART COMMAND EVENTS
           ================================================= */

        input.f_received =
            UART_GetForwardCommand();

        input.s_received =
            UART_GetStopCommand();

        input.reset_received =
            UART_GetResetCommand();


        /* =================================================
           5. UPDATE FSM
           ================================================= */

        ECU_FSM_Update(&input);


        /* =================================================
           6. UPDATE LEDs FROM FSM STATE
           ================================================= */

        GPIO_UpdateLEDs(
            ECU_FSM_GetState()
        );


        /* =================================================
           7. STATUS COMMAND
           ================================================= */

        UART2_StatusTask(
            adc_value,
            pwm_percent
        );


        /* =================================================
           8. UART TRANSMIT
           ================================================= */

        UART2_TxTask();
    }
}


/* =========================================================
   SYSTEM CLOCK
   ========================================================= */

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();

    __HAL_PWR_VOLTAGESCALING_CONFIG(
        PWR_REGULATOR_VOLTAGE_SCALE2
    );


    RCC_OscInitStruct.OscillatorType =
        RCC_OSCILLATORTYPE_HSI;

    RCC_OscInitStruct.HSIState =
        RCC_HSI_ON;

    RCC_OscInitStruct.HSICalibrationValue =
        RCC_HSICALIBRATION_DEFAULT;

    RCC_OscInitStruct.PLL.PLLState =
        RCC_PLL_NONE;


    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }


    RCC_ClkInitStruct.ClockType =
          RCC_CLOCKTYPE_HCLK
        | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1
        | RCC_CLOCKTYPE_PCLK2;

    RCC_ClkInitStruct.SYSCLKSource =
        RCC_SYSCLKSOURCE_HSI;

    RCC_ClkInitStruct.AHBCLKDivider =
        RCC_SYSCLK_DIV1;

    RCC_ClkInitStruct.APB1CLKDivider =
        RCC_HCLK_DIV1;

    RCC_ClkInitStruct.APB2CLKDivider =
        RCC_HCLK_DIV1;


    if (HAL_RCC_ClockConfig(
            &RCC_ClkInitStruct,
            FLASH_LATENCY_0) != HAL_OK)
    {
        Error_Handler();
    }
}


/* =========================================================
   ERROR HANDLER
   ========================================================= */

void Error_Handler(void)
{
    __disable_irq();

    while (1)
    {
    }
}