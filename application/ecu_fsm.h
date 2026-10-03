#ifndef ECU_FSM_H
#define ECU_FSM_H

#include <stdint.h>

typedef enum
{
    STATE_IDLE = 0,
    STATE_READY,
    STATE_RUNNING,
    STATE_EMERGENCY

} ECU_State;


typedef struct
{
    uint8_t start_pressed;

    uint8_t f_received;
    uint8_t s_received;
    uint8_t reset_received;

    uint8_t emergency_active;

    uint8_t pwm_percent;

} ECU_Input;


void ECU_FSM_Init(void);

void ECU_FSM_Update(const ECU_Input *input);

ECU_State ECU_FSM_GetState(void);

#endif