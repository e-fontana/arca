#include "temp.h"
#include "dht11.h"

DHT11_Dev sensor;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);

int getTemp(void)
{
    int resultado = DHT11_read(&sensor);

    if (resultado == DHT11_SUCCESS) {
      uint8_t temp    = sensor.temperature;
      uint8_t umidade = sensor.humidity;
      (void)temp;
      (void)umidade;
    }
    else if (resultado == DHT11_ERROR_CHECKSUM) {
      // dado corrompido, tenta no próximo ciclo
    }
    else if (resultado == DHT11_ERROR_TIMEOUT) {
      // sensor não respondeu, verifique a ligação
    }

    HAL_Delay(2000);
}