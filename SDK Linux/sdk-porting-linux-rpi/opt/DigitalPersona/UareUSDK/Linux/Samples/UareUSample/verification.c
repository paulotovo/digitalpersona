/* 
 * Copyright (C) 2011, Digital Persona, Inc.
 *
 * This file is a part of sample code for the UareU SDK 2.x.
 */

#include "verification.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "helpers.h"

#include <dpfj.h>

void Verification(DPFPDD_DEV hReader){
	unsigned char* pFeatures1 = NULL;
	unsigned int nFeatures1Size = 0;
	unsigned char* pFeatures2 = NULL;
	unsigned int nFeatures2Size = 0;

	int bStop = 0;
	while(!bStop){
		//set green and red LEDs to client-controlled mode
		int result = dpfpdd_led_config(hReader, DPFPDD_LED_ACCEPT | DPFPDD_LED_REJECT, DPFPDD_LED_CLIENT, NULL);
		if(DPFPDD_SUCCESS != result && DPFPDD_E_NOT_IMPLEMENTED != result){
			print_error("dpfpdd_led_config()", result);
		}

		printf("Verification started\n\n");

		//capture first fingerprint
		result = CaptureFinger("any finger", hReader, DPFJ_FMD_ISO_19794_2_2005, &pFeatures1, &nFeatures1Size);
		//capture second fingerprint
		if(0 == result) result = CaptureFinger("the same or any other finger", hReader, DPFJ_FMD_ISO_19794_2_2005, &pFeatures2, &nFeatures2Size);
		if(0 == result){
			//run comparison
			unsigned int falsematch_rate = 0;
			int result = dpfj_compare(DPFJ_FMD_ISO_19794_2_2005, pFeatures1, nFeatures1Size, 0, 
				DPFJ_FMD_ISO_19794_2_2005, pFeatures2, nFeatures2Size, 0, &falsematch_rate);

			if(DPFJ_SUCCESS == result){
				const unsigned int target_falsematch_rate = DPFJ_PROBABILITY_ONE / 100000; //target rate is 0.00001
				if(falsematch_rate < target_falsematch_rate){
					//turn green LED on for 1 sec
					dpfpdd_led_ctrl(hReader, DPFPDD_LED_ACCEPT, DPFPDD_LED_CMD_ON);
					sleep(1);
					dpfpdd_led_ctrl(hReader, DPFPDD_LED_ACCEPT, DPFPDD_LED_CMD_OFF);

					//print out the results
					printf("Fingerprints matched.\n\n\n");
					printf("dissimilarity score: 0x%x.\n", falsematch_rate);
					printf("false match rate: %e.\n\n\n", (double)(falsematch_rate / DPFJ_PROBABILITY_ONE));
				}
				else{
					//turn red LED on for 1 sec
					dpfpdd_led_ctrl(hReader, DPFPDD_LED_REJECT, DPFPDD_LED_CMD_ON);
					sleep(1);
					dpfpdd_led_ctrl(hReader, DPFPDD_LED_REJECT, DPFPDD_LED_CMD_OFF);

					//print out the results
					printf("Fingerprints did not match.\n\n\n");
				}
			}
			else print_error("dpfjmx_compare()", result);
		}
		else bStop = 1;
		
		//release memory
		if(NULL != pFeatures1) free(pFeatures1);
		pFeatures1 = NULL;
		nFeatures1Size = 0;
		if(NULL != pFeatures2) free(pFeatures2);
		pFeatures2 = NULL;
		nFeatures2Size = 0;
	}
}


