/* Statusreport.c
 * Created on 2025.05.29 by
 * Roland Gerendas
 *
 * Status reports take over I2C packets, when the I2C queue is empty.
 * This way valuable information about the status of Celeritas
 * can be collected, when otherwise no information would come
 */

#include "Statusreport.h"
#include "Scheduler.h"
#include "QueueManager.h"
#include "Checksum.h"
#include "Measurements.h"
#include "Timer.h"

uint8_t * generate_status_report(){

	uint32_t time = Get_SystemTime();

	uint16_t temperature = 0; //default values
	uint16_t v_int = 0;

	if (status == IDLE){ //while the ADC is occupied for measurement, the temperature can not be read
		v_int = get_refint_voltage();			//Call temperature only in IDLE mode
		temperature = get_temperature(v_int);
	}
	uint8_t current_ID = scheduler_get_request_id(0);
	QueueCursor* i2c_cursor_temp = queue_manager_get_cursor(I2C_QUEUE);
	QueueCursor* request_cursor_temp = queue_manager_get_cursor(REQUEST_QUEUE);


	status_report_data[0] = status;

	status_report_data[1] = (uint8_t)((time >> 24) & 0xFF);
	status_report_data[2] = (uint8_t)((time >> 16) & 0xFF);
	status_report_data[3] = (uint8_t)((time >> 8) & 0xFF);
	status_report_data[4] = (uint8_t)(time & 0xFF);

	status_report_data[5] = i2c_cursor_temp->head;
	status_report_data[6] = i2c_cursor_temp->tail;
	status_report_data[7] = i2c_cursor_temp->size;

	status_report_data[8] = request_cursor_temp->head;
	status_report_data[9] = request_cursor_temp->tail;
	status_report_data[10] = request_cursor_temp->size;

	status_report_data[11] = (uint8_t)(temperature >> 8);
	status_report_data[12] = (uint8_t)(temperature & 0xFF);

	status_report_data[13] = current_ID;

	status_report_data[14] = 0x55;

	uint8_t checksum = calculate_checksum(status_report_data, 15);
	status_report_data[15] = checksum;

	return status_report_data;
}
