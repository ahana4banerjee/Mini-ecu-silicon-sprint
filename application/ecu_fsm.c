#include ecu_fsm.h
#include "ecu_fsm.h"

static ECU_State current_state = STATE_IDLE;

void ECU_FSM_Update(ECU_Input *input)
{
    switch (current_state)
    {
        case STATE_IDLE:

            if (input->start_pressed)
            {
                current_state = STATE_READY;
            }

            break;


        case STATE_READY:

            /*
             * Phase 1:
             * Stay READY until later phases add
             * movement commands.
             */

            break;


        case STATE_RUNNING:

            /* Phase 2/3 */

            break;


        case STATE_EMERGENCY:

            /* Phase 4 */

            break;


        default:

            current_state = STATE_IDLE;

            break;
    }
}

ECU_State ECU_FSM_GetState(void)
{
    return current_state;
}