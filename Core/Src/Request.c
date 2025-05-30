/*
 * Request.c
 *
 *  Created on: Mar 17, 2025
 *      Author: hpraszpi
 */

#include "Request.h"

Request empty_request = {
	    .ID = 0,
	    .type = UNKNOWN,     // Assuming RequestType is an enum or similar, set to its default value
	    .is_okay = false,
		.continue_with_full_channel = false,
	    .is_header = false,
	    .limit = 0,
	    .start_time = 0,
	    .min_voltage = 0,
	    .max_voltage = 0,
	    .samples = 0,
	    .resolution = 0
};

bool check_request(Request request, uint32_t time) {
	if(request.type != SELFTEST && request.type != MAX_TIME && request.type != MAX_HITS)
		return false;		//return false if the measurement is neither of the specified ones
	if(request.start_time < time) return false;		//if the measurement was required to be carried out in the past, return false
	return true;	//other ways return true
}
