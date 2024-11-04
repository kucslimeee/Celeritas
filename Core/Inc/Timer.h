/*
 * timer.h
 *
 * Created on: Oct 4
 *     Author: adamg
 */

#ifndef INC_TIMER_H
#define INC_TIMER_H

#include <stdbool.h>
#include <stdint.h>

void Systick_Interrupt();
void Set_SystemTime(uint32_t timestamp);
uint32_t Get_SystemTime(void);


#endif
