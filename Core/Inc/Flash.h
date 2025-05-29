/*
 * Flash.h
 *
 *  Created on: Dec 4, 2024
 *      Author: hpraszpi
 */


#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include <stdint.h>
#include <stdbool.h>

#define ADDR_FLASH_PAGE_16    ((uint32_t)0x08008000) /* Base address of Page 16, 2 Kbytes */
#define ADDR_FLASH_PAGE_17    ((uint32_t)0x08008800) /* Base address of Page 17, 2 Kbytes */
#define ADDR_FLASH_PAGE_18    ((uint32_t)0x08009000) /* Base address of Page 18, 2 Kbytes */
#define ADDR_FLASH_PAGE_19    ((uint32_t)0x08009800) /* Base address of Page 19, 2 Kbytes */
#define ADDR_FLASH_PAGE_20    ((uint32_t)0x0800A000) /* Base address of Page 20, 2 Kbytes */
#define ADDR_FLASH_PAGE_21    ((uint32_t)0x0800A800) /* Base address of Page 21, 2 Kbytes */
#define ADDR_FLASH_PAGE_22    ((uint32_t)0x0800B000) /* Base address of Page 22, 2 Kbytes */
#define ADDR_FLASH_PAGE_23    ((uint32_t)0x0800B800) /* Base address of Page 23, 2 Kbytes */
#define ADDR_FLASH_PAGE_24    ((uint32_t)0x0800C000) /* Base address of Page 24, 2 Kbytes */
#define ADDR_FLASH_PAGE_25    ((uint32_t)0x0800C800) /* Base address of Page 25, 2 Kbytes */
#define ADDR_FLASH_PAGE_26    ((uint32_t)0x0800D000) /* Base address of Page 26, 2 Kbytes */
#define ADDR_FLASH_PAGE_27    ((uint32_t)0x0800D800) /* Base address of Page 27, 2 Kbytes */
#define ADDR_FLASH_PAGE_28    ((uint32_t)0x0800E000) /* Base address of Page 28, 2 Kbytes */
#define ADDR_FLASH_PAGE_29    ((uint32_t)0x0800E800) /* Base address of Page 29, 2 Kbytes */
#define ADDR_FLASH_PAGE_30    ((uint32_t)0x0800F000) /* Base address of Page 30, 2 Kbytes */
#define ADDR_FLASH_PAGE_31    ((uint32_t)0x0800F800) /* Base address of Page 31, 2 Kbytes */

#define FLASH_USER_START_ADDR   ADDR_FLASH_PAGE_20   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ADDR_FLASH_PAGE_25   /* End @ of user Flash area */

#define SETTINGS_ADDR 			ADDR_FLASH_PAGE_20
#define I2C_QUEUE_PAGE_1_ADDR	ADDR_FLASH_PAGE_21
#define I2C_QUEUE_PAGE_2_ADDR	ADDR_FLASH_PAGE_22 	// separated an marked for clearance, won't be mentioned in code
#define REQ_QUEUE_ADDR			ADDR_FLASH_PAGE_23
#define SCHEDULER_ADDR			ADDR_FLASH_PAGE_24
#define QUEUE_MANAGER_ADDR		ADDR_FLASH_PAGE_25



void flash_load(uint32_t* address, uint16_t length, uint32_t* data);
void flash_save(uint32_t address, uint8_t nfpages, uint16_t length, uint16_t* data);
void flash_reset();

extern bool flash_busy;

#endif /* INC_FLASH_H_ */
