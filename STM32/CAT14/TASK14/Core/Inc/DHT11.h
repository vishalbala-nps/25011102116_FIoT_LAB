#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include "stm32f1xx_hal.h"

#define DHT11_OUTPUT 1U
#define DHT11_INPUT  0U

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
    TIM_HandleTypeDef *htim; /* must tick at 1 MHz (1 count = 1 us) */
    float temperature;     /* degrees C */
    float humidity;        /* %RH */
} DHT11_HandleTypeDef;

void DHT11_Init(DHT11_HandleTypeDef *dht, TIM_HandleTypeDef *htim, GPIO_TypeDef *port, uint16_t pin);
void DHT11_SetGPIO_Mode(DHT11_HandleTypeDef *dht, uint8_t mode);
uint8_t DHT11_Read(DHT11_HandleTypeDef *dht);

#endif /* INC_DHT11_H_ */
