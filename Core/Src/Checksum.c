/*
  * checksum.c
  *
  * Created on: Sep 11, 2024
  *     Author: badam
  *
  * Last checksum uses Brian Kernighan’s Algorithm
  */

 #include "Checksum.h"

 /*uint8_t calculate_checksum(uint8_t item[], int length) { //old checksum
     uint8_t checksum = 0;
     uint16_t sum = 0;
     for (int i = 0; i < length; i++) {
         sum += item[i];
     }
     checksum = (uint8_t)sum;
     return checksum;
 	 }
}*/

uint8_t calculate_checksum(uint8_t item[], int length){     // this checksum calculates the number of set bits in an array of bytes
    uint8_t count = 0;                                	// return value
    uint8_t temp;                               // for storing seperately

    for (int i = 0; i < length; i++) {                  // go through each byte

        temp = item[i];

            while (temp > 0) {
                temp &= (temp - 1); // Clears the least significant set bit
                count++;
            }
    }
    return count;
}

