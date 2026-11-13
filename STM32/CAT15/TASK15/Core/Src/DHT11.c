#include "DHT11.h"

#define DHT11_TIMEOUT_US  500U
#define DHT11_BIT_ONE_US  50U /* high pulse: about 26 us = 0, about 70 us = 1 */

/* Waits while the pin holds `level`. Returns the time spent in us, or 0xFFFF on timeout. */
static uint16_t DHT11_WaitWhile(DHT11_HandleTypeDef *dht, GPIO_PinState level)
{
    __HAL_TIM_SET_COUNTER(dht->htim, 0);
    while (HAL_GPIO_ReadPin(dht->port, dht->pin) == level) {
        if ((uint16_t)__HAL_TIM_GET_COUNTER(dht->htim) > DHT11_TIMEOUT_US) {
            return 0xFFFF;
        }
    }
    return (uint16_t)__HAL_TIM_GET_COUNTER(dht->htim);
}

/* Restores interrupts and stops the timer on every exit path of the timed section. */
static uint8_t DHT11_EndTransfer(DHT11_HandleTypeDef *dht, uint8_t result)
{
    HAL_TIM_Base_Stop(dht->htim);
    __enable_irq();
    return result;
}

void DHT11_Init(DHT11_HandleTypeDef *dht, TIM_HandleTypeDef *htim, GPIO_TypeDef *port, uint16_t pin)
{
    dht->htim = htim;
    dht->port = port;
    dht->pin = pin;
    dht->temperature = 0;
    dht->humidity = 0;
}

void DHT11_SetGPIO_Mode(DHT11_HandleTypeDef *dht, uint8_t mode)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = dht->pin;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Mode = (mode == DHT11_OUTPUT) ? GPIO_MODE_OUTPUT_PP : GPIO_MODE_INPUT;
    HAL_GPIO_Init(dht->port, &GPIO_InitStruct);
}

/* Returns 1 if the read succeeded and the checksum matched, 0 otherwise */
uint8_t DHT11_Read(DHT11_HandleTypeDef *dht)
{
    uint8_t data[5] = {0};
    uint16_t lowTime, highTime;

    /* Start signal: hold the line low for at least 18 ms */
    DHT11_SetGPIO_Mode(dht, DHT11_OUTPUT);
    HAL_GPIO_WritePin(dht->port, dht->pin, GPIO_PIN_RESET);
    HAL_Delay(18);

    __disable_irq(); /* the bit timing below must not be interrupted */
    HAL_TIM_Base_Start(dht->htim);
    DHT11_SetGPIO_Mode(dht, DHT11_INPUT); /* release the line */

    /* Sensor response: pulls low, then about 80 us low, then about 80 us high */
    if (DHT11_WaitWhile(dht, GPIO_PIN_SET) == 0xFFFF) {
        return DHT11_EndTransfer(dht, 0);
    }
    lowTime = DHT11_WaitWhile(dht, GPIO_PIN_RESET);
    highTime = DHT11_WaitWhile(dht, GPIO_PIN_SET);
    if (lowTime < 60 || lowTime > 100 || highTime < 60 || highTime > 100) {
        return DHT11_EndTransfer(dht, 0);
    }

    /* 40 data bits, MSB first: each is about 50 us low, then a high pulse whose length sets the value */
    for (uint8_t i = 0; i < 40; i++) {
        if (DHT11_WaitWhile(dht, GPIO_PIN_RESET) == 0xFFFF) {
            return DHT11_EndTransfer(dht, 0);
        }
        highTime = DHT11_WaitWhile(dht, GPIO_PIN_SET);
        if (highTime == 0xFFFF) {
            return DHT11_EndTransfer(dht, 0);
        }
        data[i / 8] <<= 1;
        if (highTime > DHT11_BIT_ONE_US) {
            data[i / 8] |= 1U;
        }
    }

    DHT11_EndTransfer(dht, 1);

    /* Frame: humidity int, humidity dec, temperature int, temperature dec, checksum */
    if ((uint8_t)(data[0] + data[1] + data[2] + data[3]) != data[4]) {
        return 0;
    }

    dht->humidity = data[0];
    dht->temperature = data[2];

    dht->humidity = data[0] + data[1] * 0.1f;
    dht->temperature = data[2] + (data[3] & 0x0F) * 0.1f;

    return 1;
}
