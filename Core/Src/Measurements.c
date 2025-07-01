/*
 * Measurements.c
 *
 *  Created on: Aug 9, 2024
 *      Author: hadha
 *
 * 	Last update: Roland Gerendas, May 29 2025
 */

#include "Measurements.h"
#include "Channels.h"
#include "main.h"
#include "exp_i2c_slave.h"
#include "i2c_queue.h"
#include "Scheduler.h"
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#include "stm32f3xx_ll_adc.h"

//globals

float intervalLength = 0;
uint64_t peak_counter = 0;
uint64_t peak_limit = 0;
int intervalIndex = 0;	//this is not unsigned to distiguish negative channel numbers
bool is_v_high = 0;		//bool to know if the voltage is above the threshold on the ADC
bool ABORTED = 0;		//bool indicating that the measurement is aborted due to safety reasons


extern ADC_HandleTypeDef hadc1;
extern volatile RunningState status;

float sample_adc(uint8_t samples, uint16_t min_voltage, uint16_t max_voltage, bool is_okay);

/**
 * The measurement process of Celeritas.
 *
 * First it takes a Request as its configuration and sets up all the necessary data structures,
 * then starts the analog hardware (signal) chain, the actual measuring unit and enters the "measurement loop".
 *
 * The measurement loop first checks its running condition:
 * - in case of MAX_HITS measurement we want to count a fixed number of peaks
 * - in case of MAX_TIME, the limit is the maximum value of uint64_t and the time management is done by the scheduler module
 * The spectrometer has an adjustable channel number, that we call the resolution of the measurement:
 *  - In "counting mode" (when the resolution_measurement is 1) we only count how many samples we've got and don't save any channels.
 *  - In "spectrum mode" (resolution_measurement >= 8) we fit the samples into channels (a given range of the whole measurement spectrum).
 *    With each sample we increment the appropriate channel and in the end, send how many samples we've had in that specific channels.
 * 	  Each channel is able to register up to 65535 hits and when one of them has reached it's limit, we say it is filled.
 * 	  At this point we are able to interrupt the measurement process by setting the `request.continue_with_full_channel` option
 * 	  to false. Of course we can proceed, when `request.continue_with_full_channel` option is set true
 * 	  though this way we lose peaks in the spectrum of the filled channels since we can't
 * 	  register any new records on those channels (they do not overflow).
 *
 * When we've finished with the measurement we shut down the analog measurement unit and store the results in i2c_queue.
 * At last we notify the Scheduler that we're finished.
 */
void measure(Request request){
	ABORTED = 0;
	uint16_t arr_length = 16;
	uint16_t resolution_measurement = 16;

	resolution_measurement = request.resolution; //input the resolution setting

	arr_length = resolution_measurement;

	uint16_t measurementData[arr_length];		//make a buffer for the channels
	memset(measurementData, 0, sizeof(measurementData));	//make every channel 0

	intervalLength = (float)((request.max_voltage - request.min_voltage)/(float)(resolution_measurement)); // the range of a single channel.
	peak_counter = 0;		//set peak count to zero

	uint16_t v_ref = get_refint_voltage();
	int8_t start_temperature = get_temperature(v_ref);

	// Start of the analog chain
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);	//turn on debug LED
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 1);	//turn on the signal chain

	HAL_Delay(250); //wait 250 ms for the signal chain to turn on

	// ADC setup
	select_measurement_channel();							// select the measurement channel
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); 	// ADC auto calibration for single-ended input (has to be called before start)
	HAL_ADC_Start(&hadc1);									// Start the ADC

	HAL_Delay(250); // wait 250 ms for absolute stabilization

	// Measurement
	peak_limit = (request.type == MAX_HITS) ? (uint64_t)request.limit : UINT64_MAX; //the desired number of peaks (when to stop)
	for(peak_counter = 0; peak_counter < peak_limit; peak_counter++){
		// Get a sample
		float sample = sample_adc(request.samples, request.min_voltage, request.max_voltage, request.is_okay); //wait for and sample a peak
		if(!sample || ABORTED == 1) {
			break;		//if something goes wrong, or the status is not RUNNING, then stop (see sample_adc function)
		};
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, peak_counter % 2); // toggle the debug LED

		// Store the sample
		// note: at "counting mode" we don't have to execute all this stuff as we only count the number of peaks (peak_counter stores this information)
		if (resolution_measurement >= 8) {
			// Calculating the channel and incrementing it

			intervalIndex = (int)((sample - request.min_voltage)/intervalLength);	//which channel took a hit
			if(intervalIndex > resolution_measurement - 1) {intervalIndex = resolution_measurement - 1;};	//if the the calculated channel number is higher than the maximum, then set it to the maximum (to avoid incrementing outside the buffer)
			if(intervalIndex <= 0) {intervalIndex = 0;};								//if somehow the channel number is lower than 0, then it is 0
			if((measurementData[intervalIndex] + 1) < UINT16_MAX) {				//do not overflow the channel values
				measurementData[intervalIndex]++;
			} else if(!request.continue_with_full_channel) {
				break; 				//if we want to stop when a channel is full
			}
		}
	}

	// Shutdown of the analog chain
	HAL_Delay(1);	//just to make time to separately see the shutdown on the oscilloscope
	HAL_ADC_Stop(&hadc1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, 0);	//shutdown the signal chain
  
	status = FINISHED;
	HAL_Delay(500);								//wait 500 ms
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);	//turn off debug LED

	//Adding the header packet (if requested)

	if(ABORTED == 1) {
		add_error(request.ID, MEASUREMENT);	//measurement aborted with error
	};

	if(request.is_header){
		add_header(request, request.limit, start_temperature);
	}

	if(resolution_measurement < 8) {
		// "counting" mode:
		//saving the number of peaks counted

		uint16_t geiger_mode_out[8];
		memset(geiger_mode_out, 0, sizeof(geiger_mode_out));

		geiger_mode_out[3] = (peak_counter & 0xFFFF000000000000) >> 48;
		geiger_mode_out[4] = (peak_counter & 0x0000FFFF00000000) >> 32;
		geiger_mode_out[5] = (peak_counter & 0x00000000FFFF0000) >> 16;
		geiger_mode_out[6] = (peak_counter & 0x000000000000FFFF);
		geiger_mode_out[7] = 0xAA00 + calculate_checksum(geiger_mode_out, 15); //to signal that this is a geiger counter measurement

		for (int i = 0; i < 8; i++)		//swapping bytes for Big Endian
			geiger_mode_out[i] = (geiger_mode_out[i] << 8) | (geiger_mode_out[i] >> 8);

		add_spectrum(geiger_mode_out, request.ID);

	} else {
		// "spectrum" mode

		// swapping the bytes inside of channels (to see it in the Serial monitor in Big Endian)
		for (int i = 0; i < arr_length; i++)
			measurementData[i] = (measurementData[i] << 8) | (measurementData[i] >> 8);

		uint8_t packets = arr_length / 8;			//save the packets in the i2c queue
		for (uint8_t i = 0; i < packets; i++) {
			add_spectrum(measurementData+(i*8), request.ID);
		}
	}

	// Tell the scheduler that we're done
	scheduler_finish_measurement();
}

/*
 * BEFORE YOU CALL make SURE ADC1 is INITIALIZED AND the selected channel is ADC_IN_0 !!!!
 */
float sample_adc(uint8_t samples, uint16_t min_voltage, uint16_t max_voltage, bool is_okay){
	uint32_t sum = 0; //ADC value
	uint8_t noise_bounds = 10; // in ADC values, this means 8 mV

	void wait_for_min_threshold(bool okaying) {								//wait for the analog voltage to drop below the specified minimum threshold
		if(okaying) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);		//if the okaying bit is set, open the transistor to drain current faster from the capacitor of the peak holder
		uint16_t voltage;
		for(uint16_t abort_counter = 0; abort_counter < 20000; abort_counter++){	//do this until voltage drops below the threshold - 10 LSB for noise compensation
			voltage = analogRead();													//or the abort_counter reaches it's max value (this happens after roughly a second)
			if (voltage < min_voltage || status != RUNNING){
				if(okaying) HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); //when done, lock the transistor
				return;
			}
		}
		ABORTED = 1;	//if the peak has been high for too long, then something is wrong with the detector, abort the process
		return;
	}


	while(ABORTED == 0){
		if(status != RUNNING) {					//stop if the status is not RUNNING
			return 0;
		}
		sum = analogRead(); 					// measure ADC
		if(sum > (max_voltage - noise_bounds)) {					// if the voltage is higher than the maximum threshold, it means the peak is too high amplitude
			is_v_high = 1;						//indicate that the voltage is above the minimum threshold
			wait_for_min_threshold(is_okay);		//wait for the too high peak to drop
		} else { is_v_high = 0;};				//otherwise the voltage is below minimum threshold
		if(!(sum > (min_voltage + noise_bounds) && sum < (max_voltage - noise_bounds))) {continue;}; //if the voltage value does not fall in the measurement range, then skip this iteration and start the while loop again
		is_v_high = 1;							//there is a peak, the voltage is high
		sum = analogRead();
		for(int i = 1; i < samples; i++){		//take samples
			sum += analogRead();
		}
		break;									//break the while loop
	}
	if(ABORTED == 1) {return 0;}					//if the abort is triggered, return with 0

	wait_for_min_threshold(is_okay);			//wait for the peak to drop
	is_v_high = 0;
	return (float)(sum/(float)(samples));				//return the sampled average ADC value of the peak
}

uint16_t analogRead()							//function for getting the ADC value
{
	HAL_ADC_PollForConversion(&hadc1, 10);		// poll the ADC for conversion, timeout after 10 ms
	uint32_t value = HAL_ADC_GetValue(&hadc1);	//HAL_ADC_GetValue returns in uint32_t !!!
	return (uint16_t)value; 						// get the ADC value cast in uint16_t
}


//function for measuring the inside temperature of the STM32 units: K (deg)
int8_t get_temperature(uint16_t input_refint_voltage) {	//the calculation also depends on the reference voltage
	select_temperature_channel();							//select the temperature channel
	uint32_t value = 0;										//for sampling
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); 	// ADC auto calibration for single-ended input (has to be called before start)
	HAL_ADC_Start(&hadc1);									//start the ADC
	HAL_ADC_PollForConversion(&hadc1, 100);					// poll the ADC for conversion
	value = HAL_ADC_GetValue(&hadc1);

	//uint32_t adc = 273 - 8 + __LL_ADC_CALC_TEMPERATURE_TYP_PARAMS(4300, 1430, 25, 3300, value, LL_ADC_RESOLUTION_12B); //inbuilt driver for temperature calculation typical parameters; (273 - offset + °C = K)

	uint32_t adc = __LL_ADC_CALC_TEMPERATURE(input_refint_voltage, value, LL_ADC_RESOLUTION_12B); //inbuilt driver for temperature calculation from stored factory presets; °C


	HAL_ADC_Stop(&hadc1);
	return (int8_t)adc;		//return the temperature in K, uint16_t
};

uint16_t get_refint_voltage() {				//function for measuring the internal reference voltage of the STM32, units: mV
	select_refint_channel();
	HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED); 	// ADC auto calibration for single-ended input (has to be called before start)
	HAL_ADC_Start(&hadc1);									//start the ADC
	HAL_ADC_PollForConversion(&hadc1, 100);
	uint16_t adc1 = HAL_ADC_GetValue(&hadc1);
	adc1 = __LL_ADC_CALC_VREFANALOG_VOLTAGE(adc1, LL_ADC_RESOLUTION_12B); //convert the digital value to mV
	HAL_ADC_Stop(&hadc1);
	return (uint16_t)adc1;					//return the voltage in mV, uint16_t
};

//vbat channel is not connected. The 32 pin package does not have it.
/*int get_vbat_voltage() {
	select_vbat_channel();
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, 100);
	int adc2 = HAL_ADC_GetValue(&hadc1);
	HAL_ADC_Stop(&hadc1);
	return adc2;
}*/
