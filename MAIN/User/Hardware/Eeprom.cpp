/**
  ******************************************************************************
  * @file    Eeprom.cpp
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    27-April-2015
  * @brief
  *
  ******************************************************************************
  */
#include "Eeprom.h"
#include <string.h>

///eeprom 8K
#define  EEPROM_SIZE    6144
#define  EEPROM_ADDR    0x08006000
#define  EE_PAGE_SIZE   0x800

void eeprom_init(void)
{
  /* Unlock the Program memory */
  HAL_FLASH_Unlock();

  /* Clear all FLASH flags */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);
  /* Unlock the Program memory */
  HAL_FLASH_Lock();
}

void eeprom_write_byte(uint16_t location, uint8_t value)
{
    uint8_t read_buffer[EE_PAGE_SIZE];

	uint32_t PageError = 0;
	uint32_t page_addr;
	uint32_t data_index;
	uint32_t i;
	uint32_t destination;
    FLASH_EraseInitTypeDef pEraseInit;
    HAL_StatusTypeDef status = HAL_OK;

	if(eeprom_read_byte(location) != value){
		page_addr = EEPROM_ADDR + (location/EE_PAGE_SIZE)*EE_PAGE_SIZE;
		data_index = location % EE_PAGE_SIZE;

		memcpy(read_buffer, (const void *)page_addr, EE_PAGE_SIZE);
		memcpy(read_buffer + data_index, &value, sizeof(value));

		HAL_FLASH_Unlock();

		pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
		pEraseInit.PageAddress = page_addr;
		pEraseInit.Banks = FLASH_BANK_1;
		pEraseInit.NbPages = 1;
		status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

		if (status == HAL_OK)
		{
			destination = page_addr;
			for (i = 0; (i < EE_PAGE_SIZE) && (destination <= page_addr + EE_PAGE_SIZE  -4); i+=4)
			{
				if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, *(uint32_t*)(read_buffer+i)) == HAL_OK)
				{
					if (*(uint32_t*)destination != *(uint32_t*)(read_buffer+i))
					{
						break;
					}
					destination += 4;
				}
				else
				{
					break;
				}
			}
		}
		HAL_FLASH_Lock();
	}
}

void eeprom_write_word(uint16_t location, uint16_t value)
{
    uint8_t read_buffer[EE_PAGE_SIZE];

	uint32_t PageError = 0;
	uint32_t page_addr;
	uint32_t data_index;
	uint32_t i;
	uint32_t destination;
    FLASH_EraseInitTypeDef pEraseInit;
    HAL_StatusTypeDef status = HAL_OK;

	if(eeprom_read_word(location) != value){
		page_addr = EEPROM_ADDR + (location/EE_PAGE_SIZE)*EE_PAGE_SIZE;
		data_index = location % EE_PAGE_SIZE;

		memcpy(read_buffer, (const void *)page_addr, EE_PAGE_SIZE);
		memcpy(read_buffer + data_index, &value, sizeof(value));

		HAL_FLASH_Unlock();

		pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
		pEraseInit.PageAddress = page_addr;
		pEraseInit.Banks = FLASH_BANK_1;
		pEraseInit.NbPages = 1;
		status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

		if (status == HAL_OK)
		{
			destination = page_addr;
			for (i = 0; (i < EE_PAGE_SIZE) && (destination <= page_addr + EE_PAGE_SIZE  -4); i+=4)
			{
				if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, *(uint32_t*)(read_buffer+i)) == HAL_OK)
				{
					if (*(uint32_t*)destination != *(uint32_t*)(read_buffer+i))
					{
						break;
					}
					destination += 4;
				}
				else
				{
					break;
				}
			}
		}
		HAL_FLASH_Lock();
	}
}

void eeprom_write_dword(uint16_t location, uint32_t value)
{
    uint8_t read_buffer[EE_PAGE_SIZE];
	uint32_t PageError = 0;
	uint32_t page_addr;
	uint32_t data_index;
	uint32_t i;
	uint32_t destination;
    FLASH_EraseInitTypeDef pEraseInit;
    HAL_StatusTypeDef status = HAL_OK;

	if(eeprom_read_dword(location) != value){
		page_addr = EEPROM_ADDR + (location/EE_PAGE_SIZE)*EE_PAGE_SIZE;
		data_index = location % EE_PAGE_SIZE;

		memcpy(read_buffer, (const void *)page_addr, EE_PAGE_SIZE);
		memcpy(read_buffer + data_index, &value, sizeof(value));

		HAL_FLASH_Unlock();

		pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
		pEraseInit.PageAddress = page_addr;
		pEraseInit.Banks = FLASH_BANK_1;
		pEraseInit.NbPages = 1;
		status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

		if (status == HAL_OK)
		{
			destination = page_addr;
			for (i = 0; (i < EE_PAGE_SIZE) && (destination <= page_addr + EE_PAGE_SIZE  -4); i+=4)
			{
				if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, *(uint32_t*)(read_buffer+i)) == HAL_OK)
				{
					if (*(uint32_t*)destination != *(uint32_t*)(read_buffer+i))
					{
						break;
					}
					destination += 4;
				}
				else
				{
					break;
				}
			}
		}
		HAL_FLASH_Lock();
	}
}

//不能跨2K调用
void eeprom_write_block(uint8_t* data_ptr, uint16_t location, uint16_t data_size)
{
    uint8_t read_buffer[EE_PAGE_SIZE];

	uint32_t PageError = 0;
	uint32_t page_addr;
	uint32_t data_index;
	uint32_t i;
	uint32_t destination;
    FLASH_EraseInitTypeDef pEraseInit;
    HAL_StatusTypeDef status = HAL_OK;

    if ((location/EE_PAGE_SIZE) != ((location+data_size)/EE_PAGE_SIZE))
        return;

    page_addr = EEPROM_ADDR + (location/EE_PAGE_SIZE)*EE_PAGE_SIZE;
    data_index = location % EE_PAGE_SIZE;

    memcpy(read_buffer, (const void *)page_addr, EE_PAGE_SIZE);
    memcpy(read_buffer + data_index, data_ptr, data_size);

    HAL_FLASH_Unlock();

    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    pEraseInit.PageAddress = page_addr;
    pEraseInit.Banks = FLASH_BANK_1;
    pEraseInit.NbPages = 1;
    status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);

    if (status == HAL_OK)
    {
        destination = page_addr;
        for (i = 0; (i < EE_PAGE_SIZE) && (destination <= page_addr + EE_PAGE_SIZE  -4); i+=4)
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, destination, *(uint32_t*)(read_buffer+i)) == HAL_OK)
            {
                if (*(uint32_t*)destination != *(uint32_t*)(read_buffer+i))
                {
                    break;
                }
                destination += 4;
            }
            else
            {
                break;
            }
        }
    }
    HAL_FLASH_Lock();
}

uint8_t eeprom_read_byte(uint16_t location)
{
    uint32_t addr = EEPROM_ADDR + (uint32_t)location;
    return (*(__IO uint8_t*)addr);
}

uint16_t eeprom_read_word(uint16_t location)
{
    uint32_t addr = EEPROM_ADDR + (uint32_t)location;
    return (*(__IO uint16_t*)addr);
}

uint32_t eeprom_read_dword(uint16_t location)
{
    uint32_t addr = EEPROM_ADDR + (uint32_t)location;
    return (*(__IO uint32_t*)addr);
}

void eeprom_read_block(uint8_t* data_ptr, uint16_t location, uint16_t data_size)
{
    uint32_t addr = EEPROM_ADDR + (uint32_t)location;

    for (uint16_t i = 0; i < data_size; i ++ ) {
        data_ptr[i] = (*(__IO uint8_t*)addr++);
	}
}

void eeprom_erase()
{
    uint32_t PageError = 0;
    FLASH_EraseInitTypeDef pEraseInit;

    HAL_FLASH_Unlock();

    pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES;
    pEraseInit.PageAddress = EEPROM_ADDR;
    pEraseInit.Banks = FLASH_BANK_1;
    pEraseInit.NbPages = 3;
    HAL_FLASHEx_Erase(&pEraseInit, &PageError);

    HAL_FLASH_Lock();
}

