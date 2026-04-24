#ifndef PN532_H
#define PN532_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <string.h>

/* ── Pinagem (conforme seu CubeMX) ─────────────────────────────────────── */
#define PN532_NSS_PORT      GPIOA
#define PN532_NSS_PIN       GPIO_PIN_3

#define PN532_RST_PORT      GPIOB
#define PN532_RST_PIN       GPIO_PIN_2

#define PN532_IRQ_PORT      GPIOA
#define PN532_IRQ_PIN       GPIO_PIN_1   /* INT_NFC → EXTI1 */

/* ── Bytes de controle SPI ──────────────────────────────────────────────── */
#define PN532_SPI_DATAWRITE     0x01
#define PN532_SPI_DATAREAD      0x03
#define PN532_SPI_STATREAD      0x02
#define PN532_SPI_READY         0x01

/* ── Bytes do protocolo TFI ─────────────────────────────────────────────── */
#define PN532_PREAMBLE          0x00
#define PN532_STARTCODE1        0x00
#define PN532_STARTCODE2        0xFF
#define PN532_POSTAMBLE         0x00
#define PN532_HOSTTOPN532       0xD4
#define PN532_PN532TOHOST       0xD5

/* ── Comandos ───────────────────────────────────────────────────────────── */
#define PN532_CMD_GETFIRMWAREVERSION    0x02
#define PN532_CMD_INLISTPASSIVETARGET   0x4A

/* ── Tipos de cartão ────────────────────────────────────────────────────── */
#define PN532_MIFARE_ISO14443A          0x00

/* ── Tamanhos ───────────────────────────────────────────────────────────── */
#define PN532_MAX_UID_LEN       7
#define PN532_ACK_LEN           6

/* ── Estrutura de retorno da leitura ────────────────────────────────────── */
typedef struct {
    uint8_t uid[PN532_MAX_UID_LEN];
    uint8_t uid_len;
    uint16_t atqa;
    uint8_t  sak;
    uint8_t  valid;
} PN532_Card_t;

/* ── API pública ────────────────────────────────────────────────────────── */
void    PN532_Init(SPI_HandleTypeDef *hspi);
uint8_t PN532_GetFirmwareVersion(uint8_t *ver);
uint8_t PN532_ReadPassiveTarget(PN532_Card_t *card);
void    PN532_IRQ_Handler(void);          /* chamar do EXTI callback */

/* flag consultada no loop principal */
extern volatile uint8_t pn532_card_ready;

#endif /* PN532_H */