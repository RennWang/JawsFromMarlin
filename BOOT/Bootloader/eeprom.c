#include "string.h"
#include "eeprom.h"

void eeprom_write(uint16_t addr, uint16_t data)
{
	uint8_t read_buffer[EE_PAGE_SIZE];
	
	uint32_t PageError = 0;
	uint32_t page_addr;
	uint32_t data_index;
	uint32_t i;
	uint32_t destination;
  FLASH_EraseInitTypeDef pEraseInit;
  HAL_StatusTypeDef status = HAL_OK;
	
	if(eeprom_read(addr) != data){
		page_addr = EEPROM_ADDR + (addr / EE_PAGE_SIZE)*EE_PAGE_SIZE;
		data_index = addr % EE_PAGE_SIZE;
		
		memcpy(read_buffer,(const void *)page_addr, EE_PAGE_SIZE);
		memcpy(read_buffer + data_index, &data, sizeof(data));
		
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

uint16_t eeprom_read(uint16_t addr)
{
		return *(uint16_t*)(EEPROM_ADDR + addr);
}
