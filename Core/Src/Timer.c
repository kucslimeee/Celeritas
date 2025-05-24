/*
 * timer.c
 *
 * Created on: Oct 4, 2024
 *     Author: adamg
 */
#include "Timer.h"
#include "main.h"
#include "Scheduler.h"

volatile uint32_t current_unix_time = 0;
volatile uint16_t counter = 0;
uint16_t devide;
/*
 * An interrupt handler to be called at the end of systick interrupt.
 */
void Systick_Interrupt(){
	counter++;
	// This must run in every tent of a second, otherwise we would check only the
	// cases when our LED is turned on.

	if (counter % 100 == 0) {
		if(status == IDLE){
			devide = 200;
		}
		else {
			if(status == STARTING) {devide = 1000;}
		};		//idle status flashes rapidly, starting status flashes slowly
		if(status != RUNNING)  {
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, counter % devide == 0);
		}
		else {HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, 0); //in running or any other status the LED does not flash
		}
	}
	if (counter == 1000) {
		current_unix_time++;
		counter = 0;
		scheduler_on_even_second();
	}
}


/**
 * Sets the current system time to the given value. Disables interrupts during
 * the operation to ensure atomicity.
 *
 * @param timestamp The new system time, in seconds since the Unix epoch.
 */
void Set_SystemTime(uint32_t timestamp)
{
	current_unix_time = timestamp;
}

uint32_t Get_SystemTime() {
	return current_unix_time;
}

