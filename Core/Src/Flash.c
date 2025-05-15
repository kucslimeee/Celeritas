/*
 * Flash.c
 *
 *  Created on: Dec 4, 2024
 *      Author: hpraszpi
 */

#include "Flash.h"
#include "main.h"


/**
 * @brief	Loads data from flash
 * @param	address	The address we start the loading from
 * @param	length	Number of read operation that need to be performed.
 * 					ATTENTION! 1 read operation = 4 bytes of data
 * @param	data	save location
 */
void flash_load(uint32_t* address, uint16_t length, uint32_t* data) {
	for (uint16_t i = 0; i < length; i++) {
	    data[i] = *(__IO uint32_t *)address; // __IO is for reading from the flash
	    address++;
	}
}

/*
 * @brief	Saves data to flash
 * @param	address		The address we start saving to. MUST BE a valid flash page addr!
 * @param	nf_pages	Number of flash pages to be erased before use.
 * @param	length		Number of write operations that need to be performed.
 * 						ATTENTION! 1 write operation = 2 bytes of data
 * @param	data		The data we save to flash
 */
void flash_save(uint32_t address, uint8_t nf_pages, uint16_t length, uint16_t* data) {
	HAL_FLASH_Unlock();

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PageError;
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = address;
	EraseInitStruct.NbPages     = nf_pages;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
	  Error_Handler();

	/* Program the user Flash area word by word **/
	for (uint16_t i = 0; i < length; i++) {
	    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data[i]) != HAL_OK)
	    	Error_Handler();

	    address += 2;
	}

	    /* Lock the Flash to disable the flash control register access (recommended
	       to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();
}

/**
 * @brief	RESETS the FLASH user area!
 */
void flash_reset() {
	HAL_FLASH_Unlock();

	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PageError;
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = FLASH_USER_START_ADDR;
	EraseInitStruct.NbPages     = (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE + 1;

	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
	  Error_Handler();
	HAL_FLASH_Lock();
}
