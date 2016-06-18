/**
  ******************************************************************************
  * @file    Eeprom.h
  * @author  Wang Ren of Magicfirm Soft Team
  * @version V1.0.0
  * @date    27-April-2015
  * @brief
  *
  ******************************************************************************
  */
#ifndef EEPROM_H
#define EEPROM_H

#include "Jaws_main.h"


void eeprom_init(void);
void eeprom_write_byte(uint16_t location, uint8_t value);
void eeprom_write_word(uint16_t location, uint16_t value);
void eeprom_write_dword(uint16_t location, uint32_t value);
void eeprom_write_block(uint8_t* data_ptr, uint16_t location, uint16_t data_size);

uint8_t eeprom_read_byte(uint16_t location);
uint16_t eeprom_read_word(uint16_t location);
uint32_t eeprom_read_dword(uint16_t location);
void eeprom_read_block(uint8_t* data_ptr, uint16_t location, uint16_t data_size);

void eeprom_erase();

#endif /* EEPROM_H */
