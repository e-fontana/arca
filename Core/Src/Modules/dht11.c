#include "dht11.h"
#include "dw_stm32_delay.h"

static void delay_us(uint32_t us) {
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (HAL_RCC_GetHCLKFreq() / 16);
    while ((DWT->CYCCNT - start) < ticks);
}

static void set_output(DHT11_Dev* dev) {
    GPIO_InitTypeDef g = {0};
    g.Pin   = dev->pin;
    g.Mode  = GPIO_MODE_OUTPUT_OD;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(dev->port, &g);
}

static void set_input(DHT11_Dev* dev) {
    GPIO_InitTypeDef g = {0};
    g.Pin  = dev->pin;
    g.Mode = GPIO_MODE_INPUT;
    g.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(dev->port, &g);
}

void DHT11_init(DHT11_Dev* dev, GPIO_TypeDef* port, uint16_t pin) {
    dev->port = port;
    dev->pin  = pin;

    // Inicializa o DWT para delay_us
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

    set_output(dev);
    HAL_GPIO_WritePin(dev->port, dev->pin, GPIO_PIN_SET);
}

int DHT11_read(DHT11_Dev* dev) {
    uint8_t data[5] = {0};
    uint32_t timeout;
    DWT_Delay_Init();

    // Sinal de START
    
    set_output(dev);
    HAL_GPIO_WritePin(dev->port, dev->pin, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(dev->port, dev->pin, GPIO_PIN_SET);
    DWT_Delay_us(40);

    // Muda para input
    set_input(dev);

    // Espera DHT11 puxar LOW (resposta)
    timeout = 10000;
    while (HAL_GPIO_ReadPin(dev->port, dev->pin) == GPIO_PIN_SET)
        if (!timeout--) return DHT11_ERROR_TIMEOUT;

    // Espera DHT11 soltar HIGH
    timeout = 10000;
    while (HAL_GPIO_ReadPin(dev->port, dev->pin) == GPIO_PIN_RESET)
        if (!timeout--) return DHT11_ERROR_TIMEOUT;

    timeout = 10000;
    while (HAL_GPIO_ReadPin(dev->port, dev->pin) == GPIO_PIN_SET)
        if (!timeout--) return DHT11_ERROR_TIMEOUT;

    // Lê 40 bits
    for (int j = 0; j < 5; j++) {
        for (int i = 0; i < 8; i++) {
            timeout = 10000;
            while (HAL_GPIO_ReadPin(dev->port, dev->pin) == GPIO_PIN_RESET)
                if (!timeout--) return DHT11_ERROR_TIMEOUT;

            DWT_Delay_us(40);
            data[j] = (data[j] << 1);
            if (HAL_GPIO_ReadPin(dev->port, dev->pin) == GPIO_PIN_SET)
                data[j] |= 1;

            timeout = 10000;
            while (HAL_GPIO_ReadPin(dev->port, dev->pin) == GPIO_PIN_SET)
                if (!timeout--) break;
        }
    }

    // Verifica checksum
    if (data[4] != (uint8_t)(data[0] + data[1] + data[2] + data[3]))
        return DHT11_ERROR_CHECKSUM;

    dev->humidity    = data[0];
    dev->temperature = data[2];

    return DHT11_SUCCESS;
}