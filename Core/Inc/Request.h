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
	UNKNOWN = 0x00,
	SELFTEST = 0x01,
	MAX_TIME = 0x02,
	MAX_HITS = 0x03
} RequestType;

typedef struct {
	uint8_t ID;				// (new) measurement ID
	RequestType type;
	bool is_okay;
	bool is_header;
	uint16_t limit;
	uint32_t start_time;
	uint16_t min_voltage;
	uint16_t max_voltage;
	uint8_t samples;
	uint8_t resolution;
} Request;

static Request empty_request = {
	    .ID = 0,
	    .type = 0,          // Assuming RequestType is an enum or similar, set to its default value
	    .is_okay = false,
	    .is_header = false,
	    .limit = 0,
	    .start_time = 0,
	    .min_voltage = 0,
	    .max_voltage = 0,
	    .samples = 0,
	    .resolution = 0
	};

#endif /* INC_REQUEST_HANDLER_H_ */
