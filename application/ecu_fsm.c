#include "ecu_fsm.h"
#include "board_config.h"

static ECU_State current_state = STATE_IDLE;


/* =========================
 * LED CONTROL
 * ========================= */

static void LED_Init(void)
{
    /* Enable GPIOB clock */
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;

    /*
     * PB0  -> Orange
     * PB1  -> Green
     * PB2  -> Blue
     * PB10 -> Red
     */

    GPIOB->MODER &= ~(
          (3U << (ORANGE_LED_PIN * 2))
        | (3U << (GREEN_LED_PIN  * 2))
        | (3U << (BLUE_LED_PIN   * 2))
        | (3U << (RED_LED_PIN    * 2))
    );

    GPIOB->MODER |= (
          (1U << (ORANGE_LED_PIN * 2))
        | (1U << (GREEN_LED_PIN  * 2))
        | (1U << (BLUE_LED_PIN   * 2))
        | (1U << (RED_LED_PIN    * 2))
    );

    /* All LEDs OFF initially */
    GPIOB->ODR &= ~(
          (1U << ORANGE_LED_PIN)
        | (1U << GREEN_LED_PIN)
        | (1U << BLUE_LED_PIN)
        | (1U << RED_LED_PIN)
    );
}


/* =========================
 * LED STATE CONTROL
 * ========================= */

static void LED_Update(ECU_State state)
{
    /* Turn ALL LEDs OFF first */

    GPIOB->ODR &= ~(
          (1U << ORANGE_LED_PIN)
        | (1U << GREEN_LED_PIN)
        | (1U << BLUE_LED_PIN)
        | (1U << RED_LED_PIN)
    );


    /* Turn ON LED corresponding to FSM state */

    switch (state)
    {
        case STATE_IDLE:

            GPIOB->ODR |= (1U << ORANGE_LED_PIN);

            break;


        case STATE_READY:

            GPIOB->ODR |= (1U << GREEN_LED_PIN);

            break;


        case STATE_RUNNING:

            GPIOB->ODR |= (1U << BLUE_LED_PIN);

            break;


        case STATE_EMERGENCY:

            GPIOB->ODR |= (1U << RED_LED_PIN);

            break;


        default:

            GPIOB->ODR |= (1U << ORANGE_LED_PIN);

            break;
    }
}


/* =========================
 * FSM INITIALIZATION
 * ========================= */

void ECU_FSM_Init(void)
{
    current_state = STATE_IDLE;

    LED_Init();

    LED_Update(current_state);
}


/* =========================
 * FSM UPDATE
 * ========================= */

void ECU_FSM_Update(const ECU_Input *input)
{
    switch (current_state)
    {
        case STATE_IDLE:

            /* Emergency has priority */

            if (input->emergency_active)
            {
                current_state = STATE_EMERGENCY;
            }

            else if (input->start_pressed)
            {
                current_state = STATE_READY;
            }

            break;


        case STATE_READY:

            /* Emergency has priority */

            if (input->emergency_active)
            {
                current_state = STATE_EMERGENCY;
            }

            else if (input->f_received &&
                     (input->pwm_percent > 0))
            {
                current_state = STATE_RUNNING;
            }

            break;


        case STATE_RUNNING:

            /* Emergency has priority */

            if (input->emergency_active)
            {
                current_state = STATE_EMERGENCY;
            }

            else if (input->s_received ||
                     (input->pwm_percent == 0))
            {
                current_state = STATE_READY;
            }

            break;


        case STATE_EMERGENCY:


            if (input->reset_received)
            {
                current_state = STATE_IDLE;
            }

            break;


        default:

            current_state = STATE_IDLE;

            break;
    }


    /* Update LEDs according to new FSM state */

    LED_Update(current_state);
}


/* =========================
 * GET CURRENT STATE
 * ========================= */

ECU_State ECU_FSM_GetState(void)
{
    return current_state;
}