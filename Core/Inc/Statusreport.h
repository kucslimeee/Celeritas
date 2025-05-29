/*
 * Statusreport.h
 *
 *  Created on: May 29, 2025
 *      Author: Roland Gerendas
 */

#ifndef INC_STATUSREPORT_H_
#define INC_STATUSREPORT_H_

#include "Scheduler.h"
#include "QueueManager.h"

static uint8_t status_report_data[16];

uint8_t * generate_status_report();


#endif /* INC_STATUSREPORT_H_ */
