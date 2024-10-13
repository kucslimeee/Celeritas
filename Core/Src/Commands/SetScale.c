/*
 * SetScale.c
 *
 *  Created on: Oct 13, 2024
 *      Author: hadha
 */

void setScale(unsigned char id, unsigned char* dec) {

    setSetting(MIN_VOLTAGE, (dec[1] << 4) + (dec[2] >> 4));
    setSetting(MAX_VOLTAGE, ((dec[2]%16)<<8)+dec[3]);
    setSetting(RESOLUTION, dec[4]);
    setSetting(SAMPLES, dec[5]);



}
