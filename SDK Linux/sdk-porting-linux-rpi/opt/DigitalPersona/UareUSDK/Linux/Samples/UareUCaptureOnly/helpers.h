/* 
 * Copyright (C) 2011, Digital Persona, Inc.
 *
 * This file is a part of sample code for the UareU SDK 2.x.
 */

#pragma once

#include <dpfpdd.h>

//error handling
void print_error(const char* szFunctionName, int nError);

//returns 0 if captured, otherwise an error code
int CaptureFinger(DPFPDD_DEV hReader, int bStream);

