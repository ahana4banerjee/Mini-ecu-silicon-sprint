#ifndef GPIO_INDICATORS_H
#define GPIO_INDICATORS_H

#include "board_config.h"

/**
 * @brief  Initializes GPIO input pins (Touch START sensor).
 *         LED initialization is performed by ECU_FSM_Init().
 */
void GPIO_Init(void);

/**
 * @brief  Reads the current digital state of the Touch START input sensor.
 * @retval 1 if active (pressed/touched), 0 if inactive.
 */
uint8_t Touch_START_Read(void);

#endif /* GPIO_INDICATORS_H */
