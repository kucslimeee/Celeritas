/*
 * Flash.c
 *
 *  Created on: Dec 4, 2024
 *      Author: hpraszpi
 */

#include "Flash.h"
#include "main.h"

void flash_load(uint32_t address, uint16_t length, uint16_t* data) {
	for (uint16_t i = 0; i < length; i++) {
	    data[i] = *(__IO uint16_t *)address;
	    address += 2;
	}
}

void flash_save(uint32_t address, uint16_t length, uint16_t* data) {
	HAL_FLASH_Unlock();

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PageError;
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
	EraseInitStruct.NbPages     = 1;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
	  Error_Handler();

	/* Program the user Flash area word by word
	  (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
	for (uint16_t i = 0; i < length; i++) {
	   if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data[i]) != HAL_OK)
		   Error_Handler();
	    address += 2;
	}

	    /* Lock the Flash to disable the flash control register access (recommended
	       to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();
}
