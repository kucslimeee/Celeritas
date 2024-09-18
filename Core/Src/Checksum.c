/*
  * checksum.c
  *
  * Created on: Sep 11, 2024
  *     Author: badam
  */

 #include "Checksum.h"

 uint8_t calculate_checksum(uint8_t item[], int length) {
     uint8_t checksum = 0;
     for (int i = 0; i < length; i++) {
         checksum += item[i];
     }
     return checksum;
 }
