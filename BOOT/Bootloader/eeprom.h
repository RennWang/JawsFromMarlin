/*
 * Flash emulated eeprom, size 2K
 */
#ifndef __EEPROM_H
#define __EEPROM_H 
 
#include "stm32f1xx_hal.h"

#define  EEPROM_ADDR    (0x08006000)
#define  EE_PAGE_SIZE			FLASH_PAGE_SIZE

void eeprom_write(uint16_t addr, uint16_t data);
//void eeprom_write_dword(uint16_t addr, uint32_t);
uint16_t eeprom_read(uint16_t addr);
//uint32_t eeprom_read_dword(uint16_t addr);



#endif
