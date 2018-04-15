/* 
 * Copyright (C) 2011, Digital Persona, Inc.
 *
 * This file is a part of sample code for the UareU SDK 2.x.
 */

#include "enrollment.h"

#include <stdio.h>
#include <stdlib.h>

#include "helpers.h"

#include <dpfj.h>

void Enrollment(DPFPDD_DEV hReader){

	int bStop = 0;
	while(!bStop){
		int capture_cnt = 0;
		unsigned char* pFmd = NULL;
		unsigned int nFmdSize = 0;
		
		printf("Enrollment started\n\n");

		//start the enrollment
		int result = dpfj_start_enrollment(DPFJ_FMD_ANSI_378_2004);
		if(DPFJ_SUCCESS != result){
			print_error("dpfj_start_enrollment()", result);
			return;
		}

		//capture fingers, create templates
		int bDone = 0;
		int bFirst = 1;
		while(!bDone){
			capture_cnt++;
			printf("%d\r\n", capture_cnt);

			//capture finger, create template
			if(bFirst){
				bFirst = 0;
				if(0 != CaptureFinger("any finger", hReader, DPFJ_FMD_ANSI_378_2004, &pFmd, &nFmdSize)){
					bStop = 1;
					break;
				}
			}
			else{
				if(0 != CaptureFinger("the same finger", hReader, DPFJ_FMD_ANSI_378_2004, &pFmd, &nFmdSize)){
					bStop = 1;
					break;
				}
			}

			//add template to enrollment
			result = dpfj_add_to_enrollment(DPFJ_FMD_ANSI_378_2004, pFmd, nFmdSize, 0);

			//template is not needed anymore
			free(pFmd);
			pFmd = NULL;

			if(DPFJ_E_MORE_DATA == result){
				//need to add another template
				continue;
			}
			else if(DPFJ_SUCCESS == result){
				//enrollment is ready
				bDone = 1;
				break;
			}
			else{
				print_error("dpfj_add_to_enrollment()", result);
				break;
			}
		}
		
		//determine size (optional, MAX_FMR_SIZE can be used)
		unsigned char* pEnrollmentFmd = NULL;
		unsigned int nEnrollmentFmdSize = 0;

		if(bDone){
			result = dpfj_create_enrollment_fmd(NULL, &nEnrollmentFmdSize);

			if(DPFJ_E_MORE_DATA == result){
				pEnrollmentFmd = (unsigned char*)malloc(nEnrollmentFmdSize);
				if(NULL == pEnrollmentFmd){
					print_error("malloc()", result);
				}
				else{
					result = dpfj_create_enrollment_fmd(pEnrollmentFmd, &nEnrollmentFmdSize);
				}
			}
			if(DPFJ_SUCCESS != result){
				print_error("dpfj_create_enrollment_fmd()", result);
				nEnrollmentFmdSize = 0;
			}

			if(NULL != pEnrollmentFmd && 0 != nEnrollmentFmdSize){
				printf("Enrollment template created, size: %d\n\n\n", nEnrollmentFmdSize);

				//now enrollment template can be stored in the database

				//release memory
				free(pEnrollmentFmd);
			}
		}

		//finish the enrollment
		result = dpfj_finish_enrollment();
		if(DPFJ_SUCCESS != result){
			print_error("dpfj_finish_enrollment()", result);
		}
	}
}

