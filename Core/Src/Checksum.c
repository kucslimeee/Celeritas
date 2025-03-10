/*
  * checksum.c
  *
  * Created on: Sep 11, 2024
  *     Author: badam
  */

 #include "Checksum.h"

 uint8_t calculate_checksum(uint8_t item[], int length) {
     uint8_t checksum = 0;
     uint16_t sum = 0;
     for (int i = 0; i < length; i++) {
         sum += item[i];
     }
     checksum = (uint8_t)sum;
     return checksum;
 }
