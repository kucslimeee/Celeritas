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

#define TIMEOUT 0xFD
#define INTERRUPT 0xFB
#define CORRUPTED 0xF7

 void queue_push(uint8_t* item, bool prioriy, bool checksum);
 uint8_t* queue_get(bool* result);
 void queue_clear(void);

 #endif /* INC_I2C_QUEUE_H_ */
