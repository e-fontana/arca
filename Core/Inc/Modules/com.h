#ifndef MODULES_COM_H
#define MODULES_COM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

/**
 * @brief Inicializa o CC1101, executa testes de diagnóstico via UART e entra em modo RX.
 */
void COM_CC1101_Init(SPI_HandleTypeDef *hspi, UART_HandleTypeDef *huart);

/**
 * @brief Envia um pacote e aguarda a conclusão do TX via polling do GDO0.
 * @param data   Buffer com o payload
 * @param length Tamanho do payload em bytes
 */
void COM_CC1101_Transmit(const uint8_t *data, uint8_t length);

/**
 * @brief Processa um pacote recebido após gdo0_flag ser setado pela ISR.
 *        Lê o FIFO, valida CRC e preenche buf/len. Reinicia o modo RX ao final.
 * @param buf    Buffer de saída para o payload
 * @param len    Entrada: tamanho máximo do buffer. Saída: bytes recebidos.
 * @return 1 se pacote válido (CRC OK), 0 caso contrário
 */
uint8_t COM_CC1101_Receive(uint8_t *buf, uint8_t *len);

#endif /* MODULES_COM_H */
