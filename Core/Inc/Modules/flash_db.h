#ifndef FLASH_DB_H
#define FLASH_DB_H

/* Onde na flash fica o banco de dados */
#include <stdint.h>
#define FLASH_DB_BASE_ADDR  0x08040000UL
#define FLASH_USER_MAX      4096u

/* Estado do slot */
#define FLASH_RECORD_EMPTY     0xFFu
#define FLASH_RECORD_ACTIVE    0x01u
#define FLASH_RECORD_DELETED   0x00u // (soft-delete)

/* Estrutura de um usuário */
typedef struct {
    uint8_t uid[7];
    uint8_t uid_len;
    char    name[20];
    uint8_t valid;
    uint8_t _pad[3];
} FlashUser_t;

/* Funções */
uint32_t    FlashDB_Count(void);
uint8_t     FlashDB_Read(uint32_t index, FlashUser_t *user);
uint8_t     FlashDB_Add(const FlashUser_t *user);
uint8_t     FlashDB_DeleteUID(uint32_t index);
uint8_t     FlashDB_DeleteAll(void);
uint32_t    FlashDB_FindUID(const uint8_t *uid, uint8_t uid_len);

#endif