#ifndef FLASH_DB_H
#define FLASH_DB_H

#include <stdint.h>

/* Onde na flash fica o banco de dados */
#define FLASH_USER_BASE_ADDR    0x08040000UL /* Setor 6 - 128kb */
#define FLASH_LOG_BASE_ADDR     0x08060000UL /* Setor 7 - 128kb */

/* Tamanho da memória */
#define FLASH_RECORD_SIZE   32u
#define FLASH_SECTOR_SIZE   (128u * 1024u)
#define MAX_FLASH_RECORDS   (FLASH_SECTOR_SIZE / FLASH_RECORD_SIZE)

/* Estado do slot */
#define FLASH_RECORD_EMPTY     0xFFu
#define FLASH_RECORD_ACTIVE    0x01u
#define FLASH_RECORD_DELETED   0x00u // (soft-delete)

/* Estrutura de um usuário */
typedef struct {
    uint8_t uid[7];
    uint8_t uid_len;
    char    name[21];
    uint8_t access_lvl;
    uint8_t valid;
    uint8_t _pad[1];
} FlashUser_t;

/* Estrutura de um log */
typedef struct {
    uint32_t    timestamp;
    uint8_t     uid[7];
    uint8_t     uid_len;
    uint8_t     room_id;
    uint8_t     direction;
    uint8_t     _pad[18];
} FlashLog_t;

/* API de usuários */
uint32_t    FlashDB_UserCount(void);
uint8_t     FlashDB_UserRead(uint32_t index, FlashUser_t *user);
uint8_t     FlashDB_UserAdd(const FlashUser_t *user);
uint8_t     FlashDB_UserDelete(uint32_t index);
uint8_t     FlashDB_UserDeleteAll(void);
uint32_t    FlashDB_UserFindUID(const uint8_t *uid, uint8_t uid_len);

/* API de logs */
uint32_t    FlashDB_LogCount(void);
uint8_t     FlashDB_LogRead(uint32_t index, FlashLog_t *log);
uint8_t     FlashDB_LogAdd(const FlashLog_t *log);
uint8_t     FlashDB_LogDeleteAll(void);

#endif