/**************************************************************************
 *  @file     pn532_stm32f4.c
 *  @brief    Camada SPI para a lib PN532 Waveshare — STM32F4xx HAL.
 *
 *  O PN532 em SPI comunica LSB-first. O SPI1 está configurado MSB-first
 *  (compartilhado com o CC1101), portanto CADA byte é invertido em
 *  software antes de TX e depois de RX (reverse8).
 *
 *  wait_ready usa STATREAD via SPI — não depende do pino IRQ.
 **************************************************************************/

#include <stdio.h>
#include <string.h>
#include "stm32f4xx_hal.h"
#include "main.h"
#include "pn532_stm32f4.h"

#define _SPI_STATREAD    0x02
#define _SPI_DATAWRITE   0x01
#define _SPI_DATAREAD    0x03
#define _SPI_READY       0x01
#define _SPI_TIMEOUT     100

/* Coloque 1 para ver os logs de WaitReady na UART. 0 em produção. */
#define PN532_DEBUG_WAITREADY  1

/* Buffer fixo: maior frame possível da lib + preâmbulo SPI */
#define PN532_BUF_MAX  270

extern SPI_HandleTypeDef hspi1;
extern volatile uint8_t nfc_card_ready;

#define NFC_COOLDOWN_MS  1500u

static NFC_CardCallback_t s_card_callback = NULL;

void NFC_SetCardCallback(NFC_CardCallback_t cb)
{
    s_card_callback = cb;
}

/* ---- Diagnóstico inspecionável via SWD/Watch ---- */
typedef struct {
    volatile uint32_t wr_calls;
    volatile uint32_t rd_calls;
    volatile uint32_t wait_ok;
    volatile uint32_t wait_timeout;
    volatile uint8_t  last_status;     /* último byte de STATREAD */
    volatile uint8_t  last_rx[16];
} pn532_dbg_t;
volatile pn532_dbg_t pn532_dbg;

/**************************************************************************
 * Inversão de bits — PN532 SPI é LSB-first, SPI1 é MSB-first
 **************************************************************************/
static uint8_t reverse8(uint8_t b) {
    b = (b & 0xF0u) >> 4 | (b & 0x0Fu) << 4;
    b = (b & 0xCCu) >> 2 | (b & 0x33u) << 2;
    b = (b & 0xAAu) >> 1 | (b & 0x55u) << 1;
    return b;
}

/**************************************************************************
 * Reset e Log
 **************************************************************************/
int PN532_Reset(void) {
    /* RESET ativo em LOW: pulso baixo reinicia o chip */
    HAL_GPIO_WritePin(NFC_RESET_GPIO_Port, NFC_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(NFC_RESET_GPIO_Port, NFC_RESET_Pin, GPIO_PIN_RESET);
    HAL_Delay(50);
    HAL_GPIO_WritePin(NFC_RESET_GPIO_Port, NFC_RESET_Pin, GPIO_PIN_SET);
    HAL_Delay(100);   /* tempo para o 80C51 interno bootar */
    return PN532_STATUS_OK;
}

void PN532_Log(const char* log) {
    printf("%s\r\n", log);
}

void PN532_Init(PN532* pn532) {
    PN532_SPI_Init(pn532);
}

/**************************************************************************
 * SPI — transferência com inversão de bits e controle de NSS
 **************************************************************************/
static void spi_rw(uint8_t* data, uint16_t count) {
    /* inverte tudo antes de enviar */
    for (uint16_t i = 0; i < count; i++) {
        data[i] = reverse8(data[i]);
    }

    HAL_GPIO_WritePin(NFC_NSS_GPIO_Port, NFC_NSS_Pin, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_SPI_TransmitReceive(&hspi1, data, data, count, _SPI_TIMEOUT);
    HAL_Delay(1);
    HAL_GPIO_WritePin(NFC_NSS_GPIO_Port, NFC_NSS_Pin, GPIO_PIN_SET);

    /* desinverte tudo que foi recebido */
    for (uint16_t i = 0; i < count; i++) {
        data[i] = reverse8(data[i]);
    }
}

int PN532_SPI_ReadData(uint8_t* data, uint16_t count) {
    uint8_t frame[PN532_BUF_MAX];
    if (count + 1 > PN532_BUF_MAX) return PN532_STATUS_ERROR;

    frame[0] = _SPI_DATAREAD;
    for (uint16_t i = 0; i < count; i++) frame[i + 1] = 0x00;

    HAL_Delay(2);
    spi_rw(frame, count + 1);

    for (uint16_t i = 0; i < count; i++) {
        data[i] = frame[i + 1];
        if (i < 16) pn532_dbg.last_rx[i] = data[i];
    }
    pn532_dbg.rd_calls++;
    return PN532_STATUS_OK;
}

int PN532_SPI_WriteData(uint8_t* data, uint16_t count) {
    uint8_t frame[PN532_BUF_MAX];
    if (count + 1 > PN532_BUF_MAX) return PN532_STATUS_ERROR;

    frame[0] = _SPI_DATAWRITE;
    for (uint16_t i = 0; i < count; i++) frame[i + 1] = data[i];

    spi_rw(frame, count + 1);
    pn532_dbg.wr_calls++;
    return PN532_STATUS_OK;
}

bool PN532_SPI_WaitReady(uint32_t timeout) {
    uint32_t tickstart = HAL_GetTick();

    while ((HAL_GetTick() - tickstart) < timeout) {
        uint8_t buf[2] = { _SPI_STATREAD, 0x00 };
        spi_rw(buf, 2);
        pn532_dbg.last_status = buf[1];

#if PN532_DEBUG_WAITREADY
        printf("WR [%02X %02X]\r\n", buf[0], buf[1]);
#endif
        if (buf[1] == _SPI_READY) {
            pn532_dbg.wait_ok++;
            return true;
        }
        HAL_Delay(5);
    }
    pn532_dbg.wait_timeout++;
#if PN532_DEBUG_WAITREADY
    printf("WR TIMEOUT\r\n");
#endif
    return false;
}

int PN532_SPI_Wakeup(void) {
    uint8_t data[1] = { 0x00 };
    HAL_GPIO_WritePin(NFC_NSS_GPIO_Port, NFC_NSS_Pin, GPIO_PIN_RESET);
    HAL_Delay(3);                       /* CSn baixo ≥2ms acorda o chip */
    spi_rw(data, 1);
    HAL_GPIO_WritePin(NFC_NSS_GPIO_Port, NFC_NSS_Pin, GPIO_PIN_SET);
    HAL_Delay(2);
    return PN532_STATUS_OK;
}

void PN532_SPI_Init(PN532* pn532) {
    pn532->reset      = PN532_Reset;
    pn532->read_data  = PN532_SPI_ReadData;
    pn532->write_data = PN532_SPI_WriteData;
    pn532->wait_ready = PN532_SPI_WaitReady;
    pn532->wakeup     = PN532_SPI_Wakeup;
    pn532->log        = PN532_Log;

    printf("=== PN532 INIT v4 ===\r\n");

    HAL_GPIO_WritePin(NFC_NSS_GPIO_Port, NFC_NSS_Pin, GPIO_PIN_SET);
    pn532->reset();
    pn532->wakeup();
}

/**************************************************************************
 * Detecção de cartão não-bloqueante (para uso com interrupção)
 *
 * NFC_StartDetection: envia InListPassiveTarget e consome só o ACK.
 *   Retorna imediatamente. A partir daqui o PN532 procura cartão sozinho
 *   e baixará o pino IRQ (PA1) quando achar.
 *
 * NFC_ReadDetection: lê a resposta JÁ pronta (chamar após o IRQ cair).
 *   Preenche uid[] e retorna o tamanho do UID, ou 0/negativo se erro.
 **************************************************************************/

int NFC_StartDetection(PN532* pn532) {
    uint8_t buff[8];
    buff[0] = PN532_HOSTTOPN532;
    buff[1] = PN532_COMMAND_INLISTPASSIVETARGET;
    buff[2] = 0x01;                      /* MaxTg: 1 cartão       */
    buff[3] = PN532_MIFARE_ISO14443A;    /* BrTy: ISO14443A       */

    if (PN532_WriteFrame(pn532, buff, 4) != PN532_STATUS_OK) {
        return PN532_STATUS_ERROR;
    }
    /* o ACK chega rápido; consumимos aqui mesmo */
    if (!pn532->wait_ready(100)) {
        return PN532_STATUS_ERROR;
    }
    pn532->read_data(buff, 6);
    /* não validamos byte-a-byte o ACK aqui; se vier errado,
       o ReadDetection adiante vai falhar de forma detectável */
    return PN532_STATUS_OK;
}

int NFC_ReadDetection(PN532* pn532, uint8_t* uid) {
    uint8_t buff[24];

    /* A resposta já deve estar pronta (IRQ caiu). wait_ready retorna rápido.
       Mas damos uma folga curta por segurança. */
    if (!pn532->wait_ready(50)) {
        return PN532_STATUS_ERROR;
    }

    int frame_len = PN532_ReadFrame(pn532, buff, sizeof(buff));
    if (frame_len < 0) {
        return PN532_STATUS_ERROR;
    }
    /* buff[0]=D5, buff[1]=0x4B (resposta do InListPassiveTarget) */
    if (buff[0] != PN532_PN532TOHOST ||
        buff[1] != PN532_RESPONSE_INLISTPASSIVETARGET) {
        return PN532_STATUS_ERROR;
    }
    /* layout a partir de buff[2]:
       [2] NbTg  [3] Tg  [4..5] ATQA  [6] SAK  [7] UIDlen  [8..] UID */
    uint8_t nbtg = buff[2];
    if (nbtg == 0) {
        return 0;                        /* nenhum cartão            */
    }
    uint8_t uid_len = buff[7];
    if (uid_len > 7) uid_len = 7;
    for (uint8_t i = 0; i < uid_len; i++) {
        uid[i] = buff[8 + i];
    }
    return uid_len;
}

int NFC_Begin(PN532* pn532) {
    PN532_SPI_Init(pn532);

    uint8_t fwbuf[4];
    if (PN532_GetFirmwareVersion(pn532, fwbuf) != PN532_STATUS_OK) {
        printf("PN532 not found!\r\n");
        return PN532_STATUS_ERROR;
    }
    printf("PN532 OK, fw %d.%d\r\n", fwbuf[1], fwbuf[2]);

    PN532_SamConfiguration(pn532);

    NFC_IRQ_Disarm();
    NFC_StartDetection(pn532);
    NFC_IRQ_Arm();
    printf("Aguardando cartao...\r\n");

    return PN532_STATUS_OK;
}

void NFC_IRQ_Arm(void) {
    __HAL_GPIO_EXTI_CLEAR_IT(NFC_INT_Pin);   /* zera borda pendente   */
    NVIC_ClearPendingIRQ(EXTI1_IRQn);
    nfc_card_ready = 0;
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
}

uint8_t NFC_HandleCardEvent(PN532* pn532) {
    static uint8_t  last_uid[7]  = {0};
    static uint8_t  last_uid_len = 0;
    static uint32_t last_read_ms = 0;
    uint8_t uid[7];

    NFC_IRQ_Disarm();
    nfc_card_ready = 0;

    int len = NFC_ReadDetection(pn532, uid);

    if (len > 0) {
        uint8_t same = (len == last_uid_len);
        if (same) {
            for (int i = 0; i < len; i++) {
                if (uid[i] != last_uid[i]) { same = 0; break; }
            }
        }

        uint32_t now        = HAL_GetTick();
        uint8_t  in_cooldown = (now - last_read_ms) < NFC_COOLDOWN_MS;

        if (!same || !in_cooldown) {
            printf("Cartao (%d): ", len);
            for (int i = 0; i < len; i++) printf("%02X ", uid[i]);
            printf("\r\n");

            if (s_card_callback) {
                s_card_callback(uid, (uint8_t)len);
            }

            last_uid_len = len;
            for (int i = 0; i < len; i++) last_uid[i] = uid[i];
            last_read_ms = now;
        }
    }

    NFC_StartDetection(pn532);
    NFC_IRQ_Arm();

    return (uint8_t)(len > 0);
}

void NFC_IRQ_Disarm(void) {
    HAL_NVIC_DisableIRQ(EXTI1_IRQn);
    __HAL_GPIO_EXTI_CLEAR_IT(NFC_INT_Pin);
    NVIC_ClearPendingIRQ(EXTI1_IRQn);
}