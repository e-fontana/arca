#ifndef PN532_H
#define PN532_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

/* Defines de controle SPI */
#define PN532_SPI_DATAWRITE     0x01u
#define PN532_SPI_READY         0x01u
#define PN532_SPI_STATREAD      0x02u
#define PN532_SPI_DATAREAD      0x03u

/* TFI (Transport Frame Identifier) */
#define PN532_TFI_HOST_TO_PN    0xD4u
#define PN532_TFI_PN_TO_HOST    0xD5u

/* Comandos utilizados */
#define PN532_CMD_GETFIRMWAREVERSION    0x02u
#define PN532_CMD_SAMCONFIGURATION      0x14u
#define PN532_CMD_INLISTPASSIVETARGET   0x4Au

/* Timeouts */
#define PN532_SPI_TIMEOUT   50u
#define PN532_ACK_TIMEOUT   100u
#define PN532_RESP_TIMEOUT  500u

/* Tamanho máximo do buffer de resposta */
#define PN532_RESP_BUF_SIZE 32u

/* Comportamento do LED_STATUS */
#define PN532_LED_STATUS_ACTIVE_LOW 1u

/* Struct do cartão detectado */
typedef struct {
    uint8_t     uid[7];
    uint8_t     uid_len;
    uint16_t    atqa;
    uint8_t     sak;
    uint8_t     valid;
} PN532_Card_t;

/* API Pública */
void        NFC_Init(SPI_HandleTypeDef *hspi);
void        NFC_StartRead(void);
uint8_t     NFC_GetFirmwareVersion(void);
void        NFC_Begin(SPI_HandleTypeDef *hspi);
uint8_t     NFC_GetCard(PN532_Card_t *card);
void        NFC_IRQ_Handler(void);
void        NFC_CardDetected(PN532_Card_t *card);
void        NFC_Process(void);

#ifdef __cplusplus
}
#endif

#endif /* PN532_H */ 