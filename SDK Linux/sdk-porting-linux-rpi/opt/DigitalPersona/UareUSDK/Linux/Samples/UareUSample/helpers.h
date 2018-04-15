/* 
 * Copyright (C) 2011, Digital Persona, Inc.
 *
 * This file is a part of sample code for the UareU SDK 2.x.
 */

#pragma once

#include <dpfpdd.h>
#include <dpfj.h>
#include <dpfj_compression.h>
#include <dpfj_quality.h>

//error handling
void print_error(const char* szFunctionName, int nError);

//returns 0 if captured, otherwise an error code
int CaptureFinger(const char* szFingerName, DPFPDD_DEV hReader, DPFJ_FMD_FORMAT nFtType, unsigned char** ppFt, unsigned int* pFtSize);

