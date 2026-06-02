#include "main.h"
#include <stdbool.h>

/* * Importamos a variável do conversor analógico (ADC) que 
 * foi configurada automaticamente pelo CubeMX no main.c
 */
extern ADC_HandleTypeDef hadc1; 

/* * Limiar (threshold) de pressão para considerar a porta aberta.
 * Como a leitura de 12 bits vai de 0 a 4095, escolhemos 2000 como ponto de partida.
 * ATENÇÃO: Terá de ajustar este valor depois, quando testar fisicamente na porta!
 */
#define THRESHOLD_DOOR 2000 

/**
  * @brief  Lê o valor bruto do sensor de pressão.
  * @retval Valor de 0 a 4095 representando a tensão lida no pino.
  */
uint32_t Press_ReadRaw(void) {
    uint32_t adcValue = 0;

    // Inicia a conversão analógica no pino configurado (PA4)
    HAL_ADC_Start(&hadc1);

    // Espera até a leitura terminar (com timeout de 10 milissegundos)
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
        // Guarda o valor lido
        adcValue = HAL_ADC_GetValue(&hadc1);
    }

    // Pára o ADC para poupar energia e preparar a próxima leitura
    HAL_ADC_Stop(&hadc1);

    return adcValue;
}

/**
  * @brief  Verifica o estado da porta com base na leitura de pressão.
  * @retval true se a porta estiver aberta, false se estiver fechada.
  */
bool Press_IsDoorOpen(void) {
    // Chama a função acima para saber a pressão atual
    uint32_t current_pressure = Press_ReadRaw();
    
    /* * Lógica: assumindo que quando a porta está fechada ela aperta o sensor (valor alto).
     * Quando a porta abre, o botão/sensor é solto e a leitura desce.
     */
    if (current_pressure < THRESHOLD_DOOR) {
        return true;  // Porta Aberta
    } else {
        return false; // Porta Fechada
    }
}