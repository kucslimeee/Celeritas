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

#define TIMEOUT 0xFD
#define INTERRUPT 0xFB
#define CORRUPTED 0xF7

void i2c_queue_init();
void i2c_queue_push(uint8_t* item, bool priority, bool checksum);
uint8_t* i2c_queue_get(bool* result);
uint8_t i2c_queue_count(bool (*filter)(uint8_t* item));
uint8_t* i2c_queue_fetch(uint8_t idx, bool *result);

void add_header(Request request, uint16_t duration);
void add_spectrum(Request request, uint8_t* spectrum, uint8_t resolution);
void add_error(Request request, uint8_t error_type);

#endif /* INC_I2C_QUEUE_H_ */
