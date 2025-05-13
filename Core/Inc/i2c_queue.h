/*
 * i2c_queue.h
 *
 * Created on: Sep 11, 2024
 * 	   Author: badam
 */

#ifndef INC_I2C_QUEUE_H_
#define INC_I2C_QUEUE_H_

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "Checksum.h"
#include "Request.h"

typedef enum {
  TIMEOUT = 0xFD,
  CORRUPTED = 0xF7,
  MEASUREMENT = 0xFB,
  TERMINATED = 0xF0,
} ErrorType;

void i2c_queue_init();
void i2c_queue_push(uint8_t* item, bool checksum);
uint8_t* i2c_queue_get(bool* result);
uint8_t i2c_queue_count(bool (*filter)(uint8_t* item));
uint8_t* i2c_queue_fetch(uint8_t idx, bool *result);
void i2c_queue_save();

void add_header(Request request, uint16_t duration);
void add_spectrum(uint16_t* spectrum);
void add_error(uint8_t request_id, uint8_t error_type);

#endif /* INC_I2C_QUEUE_H_ */
