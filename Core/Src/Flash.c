/*
 * Flash.c
 *
 *	Source: https://controllerstech.com/flash-programming-in-stm32/
 *
 *  Created on: Dec 1, 2024
 *      Author: hpraszpi
 */
#include "Flash.h"
#include "main.h"

void flash_load(uint32_t address, uint16_t length, uint16_t* result)
{
	while (1)
	{
		*result = *(__IO uint16_t *)address;
		address += 2;
		result++;
		if (!(length--)) break;
	}
}


void flash_save(uint32_t page, uint16_t length, uint16_t* data) {
	static FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t PAGEError;
	/* Unlock the Flash to enable the flash control register access *************/
    HAL_FLASH_Unlock();

    /* Erase the user Flash area*/
    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = page;
    EraseInitStruct.NbPages     = 1;

    if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
    	Error_Handler();

     /* Program the user Flash area word by word*/
     for(uint16_t i = 0; i < length; i++)
     {
    	 if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page, data[i]) != HAL_OK)
    		 Error_Handler();
    	 page += 2;
     }

    /* Lock the Flash to disable the flash control register access (recommended
		to protect the FLASH memory against possible unwanted operation) *********/
    HAL_FLASH_Lock();

	return 0;
}
