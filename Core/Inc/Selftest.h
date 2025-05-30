/*
 * Selftest.h
 *
 *  Created on: Nov 14, 2024
 *      Author: hpraszpi
 */

#ifndef INC_SELFTEST_H_
#define INC_SELFTEST_H_
#include "Request.h"

void selftest(Request request);

extern bool backup_save;
extern bool ABORTED;

#endif /* INC_SELFTEST_H_ */
