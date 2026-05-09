#ifndef MODULES_COM_H
#define MODULES_COM_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

#define COM_CC1101_MAX_PAYLOAD_LEN 64u
#define COM_CC1101_MAX_MESSAGE_LEN 128u

typedef struct {
    uint8_t has_packet;
    uint8_t is_valid;
    uint8_t gdo0;
    uint8_t marcstate;
    uint8_t rxbytes;
    uint8_t payload_len;
    uint8_t payload[COM_CC1101_MAX_PAYLOAD_LEN];
    char message[COM_CC1101_MAX_MESSAGE_LEN];
} COM_CC1101_RxEvent_t;

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

/**
 * @brief Registra a interrupção do pino do CC1101 para processamento posterior.
 * @param gpio_pin Pino recebido no callback EXTI
 */
void COM_CC1101_HandleInterrupt(uint16_t gpio_pin);

/**
 * @brief Processa o evento pendente do CC1101 e retorna mensagem + payload.
 * @param event Estrutura de saída preenchida com status, mensagem e payload
 * @return 1 se recebeu payload válido, 0 caso contrário
 */
uint8_t COM_CC1101_ProcessInterrupt(COM_CC1101_RxEvent_t *event);

/**
 * @brief Atualiza o snapshot do CC1101 e, se houver pacote válido, preenche a struct.
 * @param event Estrutura de saída preenchida com status, mensagem e payload
 * @return 1 se recebeu payload válido, 0 caso contrário
 */
uint8_t COM_CC1101_Poll(COM_CC1101_RxEvent_t *event);

#endif /* MODULES_COM_H */
