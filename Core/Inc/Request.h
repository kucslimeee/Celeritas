/*
 * Request.h
 *
 * Created on: Sep 2, 2024
 * 	   Author: badam
 */

#ifndef INC_REQUEST_HANDLER_H_
#define INC_REQUEST_HANDLER_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum class {
	SELFTEST = 0x01,
	MAX_TIME = 0x02,
	MAX_HITS = 0x03
} RequestType;

typedef struct {
	uint8_t ID;				// (new) measurement ID
	RequestType type;		//
	bool is_priority;		//
	bool is_header;
	uint8_t limit;
	uint32_t start_time;
	uint8_t min_voltage;
	uint8_t max_voltage;
} Request;

#endif /* INC_REQUEST_HANDLER_H_ */
