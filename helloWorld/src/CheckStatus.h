#ifndef __CHECKSTATUS_H__
#define __CHECKSTATUS_H__

#include "dds_dcps.h"
#include <stdio.h>
#include <stdlib.h>

/* Array to hold the names for all ReturnCodes. */
char *RetCodeName[13];

/**
 * Returns the name of an error code.
 **/
char *getErrorName(DDS_ReturnCode_t status);

/**
 * Check the return status for errors. If there is an error, then terminate.
 **/
void checkStatus(DDS_ReturnCode_t status, const char *info);

/**
 * Check whether a valid handle has been returned. If not, then terminate.
 **/
void checkHandle(void *handle, const char *info);

#endif
