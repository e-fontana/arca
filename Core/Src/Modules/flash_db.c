#include "flash_db.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_def.h"
#include <stdint.h>
#include <string.h>

_Static_assert(sizeof(FlashUser_t) == FLASH_RECORD_SIZE, "FlashUser_t must be 32 bytes");
_Static_assert(sizeof(FlashLog_t) == FLASH_RECORD_SIZE, "FlashLog_t must be 32 bytes");

static HAL_StatusTypeDef _flash_erase_sector(uint32_t sector)
{
    FLASH_EraseInitTypeDef erase = {
        .TypeErase = FLASH_TYPEERASE_SECTORS,
        .Sector = sector,
        .NbSectors = 1u,
        .VoltageRange = FLASH_VOLTAGE_RANGE_3
    };
    uint32_t sector_error = 0;

    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_FLASHEx_Erase(&erase, &sector_error);
    HAL_FLASH_Lock();

    return status;
}

static HAL_StatusTypeDef _flash_write_32(uint32_t dest_addr, const void *src)
{
    const uint32_t *words = (const uint32_t *)src;

    HAL_FLASH_Unlock();
    HAL_StatusTypeDef status = HAL_OK;
    for (int i = 0; i < 8 && status == HAL_OK; i++)
    {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest_addr + (i * 4), words[i]);
    }
    HAL_FLASH_Lock();

    return status;
}

/* Usuários */

uint32_t FlashDB_UserCount(void)
{
    uint32_t count = 0;
    for (uint32_t i = 0; i < MAX_FLASH_RECORDS; i++)
    {
        uint32_t addr = FLASH_USER_BASE_ADDR + (i * FLASH_RECORD_SIZE);
        const FlashUser_t *slot = (const FlashUser_t *)addr;
        if (slot->valid == FLASH_RECORD_ACTIVE)
        {
            count++;
        }
    }
    return count;
}

uint8_t FlashDB_UserRead(uint32_t index, FlashUser_t *user)
{
    if (index >= MAX_FLASH_RECORDS || user == NULL) return 0u;

    uint32_t addr = FLASH_USER_BASE_ADDR + (index * FLASH_RECORD_SIZE);
    memcpy(user, (const void *)addr, sizeof(FlashUser_t));

    return (user->valid == FLASH_RECORD_ACTIVE) ? 1u : 0u;
}

uint8_t FlashDB_UserAdd(const FlashUser_t *user)
{
    if (user == NULL) return 0u;

    for (uint32_t i = 0u; i < MAX_FLASH_RECORDS; i++)
    {
        uint32_t addr = FLASH_USER_BASE_ADDR + (i * FLASH_RECORD_SIZE);
        const FlashUser_t *slot = (const FlashUser_t *)addr;

        if (slot->valid == FLASH_RECORD_EMPTY)
        {
            FlashUser_t tmp = *user;
            tmp.valid = FLASH_RECORD_ACTIVE;
            return (_flash_write_32(addr, &tmp) == HAL_OK) ? 1u : 0u;
        }
    }

    return 0u;
}

uint8_t FlashDB_UserDelete(uint32_t index)
{
    if (index >= MAX_FLASH_RECORDS) return 0u;

    uint32_t addr = FLASH_USER_BASE_ADDR + (index * FLASH_RECORD_SIZE);
    const FlashUser_t *slot = (const FlashUser_t *)addr;

    if (slot->valid != FLASH_RECORD_ACTIVE) return 0u;

    FlashUser_t tmp;
    memcpy(&tmp, slot, sizeof(FlashUser_t));
    tmp.valid = FLASH_RECORD_DELETED;

    return (_flash_write_32(addr, &tmp) == HAL_OK) ? 1u : 0u;
}

uint8_t FlashDB_UserDeleteAll(void)
{
    return (_flash_erase_sector(FLASH_SECTOR_6) == HAL_OK) ? 1u : 0u;
}

uint32_t FlashDB_UserFindUID(const uint8_t *uid, uint8_t uid_len)
{
    if (uid == NULL) return MAX_FLASH_RECORDS;

    for (uint32_t i = 0u; i < MAX_FLASH_RECORDS; i++)
    {
        uint32_t addr = FLASH_USER_BASE_ADDR + (i * FLASH_RECORD_SIZE);
        const FlashUser_t *slot = (const FlashUser_t *)addr;

        if (slot->valid != FLASH_RECORD_ACTIVE)     continue;
        if (slot->uid_len != uid_len)               continue;
        if (memcmp(slot->uid, uid, uid_len) == 0)   return i;
    }

    return MAX_FLASH_RECORDS;
}