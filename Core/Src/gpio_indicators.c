#include "gpio_indicators.h"

void GPIO_Init(void)
{
    /* Enable GPIOA Clock for Touch START Sensor */
    TOUCH_START_CLK_ENABLE();

    /* Configure PA0 as Digital Input */
    TOUCH_START_PORT->MODER &= ~(3U << (TOUCH_START_PIN * 2));

    /* Configure Pull-down resistor on PA0 */
    TOUCH_START_PORT->PUPDR &= ~(3U << (TOUCH_START_PIN * 2));
    TOUCH_START_PORT->PUPDR |=  (2U << (TOUCH_START_PIN * 2)); /* 10: Pull-down */
}

uint8_t Touch_START_Read(void)
{
    /* Return 1 if PA0 is HIGH, 0 if LOW */
    return (TOUCH_START_PORT->IDR & (1U << TOUCH_START_PIN)) ? 1U : 0U;
}
