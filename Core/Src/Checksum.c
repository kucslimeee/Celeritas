/*
  * checksum.c
  *
  * Created on: Sep 11, 2024
  *     Author: badam
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
 }*/

int power_calc(int base, int exponent){         //power function / hatvány függvény
    int value = 1;
    for (int i = 0; i < exponent; i++){
        value = value * base;
    }
    return value;
}

uint8_t calculate_checksum(uint8_t item[], int length){                      // new checksum
    uint8_t sum = 0;                                // return value
    uint8_t count[8];                               // for storing seperately

    for (int i = 0; i < length; i++) {                  // go through each byte

        count[i] = item[i];

        for (int k = 7; k >= 0; k--){               // count the number of decimal places with ones in binary
            if (count[i] >= power_calc(2, k)){
                count[i] = count[i] - power_calc(2, k);
                sum++;                              // add them up
            }
        }
    }
    return sum;                            // 7*8 = 56 is the maximum checksum value, minimum is 0
}
