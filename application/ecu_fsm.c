#include "ecu_fsm.h"

static ECU_State current_state = STATE_IDLE;


/* =========================
 * FSM INITIALIZATION
 * ========================= */

void ECU_FSM_Init(void)
{
    current_state = STATE_IDLE;
}


/* =========================
 * FSM UPDATE
 * ========================= */

void ECU_FSM_Update(const ECU_Input *input)
{
    if (input == 0)
    {
        return;
    }

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
                     (input->pwm_percent > 0U))
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
                     (input->pwm_percent == 0U))
            {
                current_state = STATE_READY;
            }

            break;


        case STATE_EMERGENCY:

            /*
             * RESET returns ECU to IDLE.
             * RESET does not start the motor.
             */
            if (input->reset_received)
            {
                current_state = STATE_IDLE;
            }

            break;


        default:

            current_state = STATE_IDLE;

            break;
    }
}


/* =========================
 * GET CURRENT STATE
 * ========================= */

ECU_State ECU_FSM_GetState(void)
{
    return current_state;
}