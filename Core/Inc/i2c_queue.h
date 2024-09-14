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

 void queue_push(uint8_t item[], bool prioriy, bool checksum);
 uint8_t* queue_get(void);
 void queue_clear(void);

 #endif /* INC_I2C_QUEUE_H_ */
