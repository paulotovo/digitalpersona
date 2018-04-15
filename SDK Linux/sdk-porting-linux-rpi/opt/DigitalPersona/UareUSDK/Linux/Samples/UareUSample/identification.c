/* 
 * Copyright (C) 2011, Digital Persona, Inc.
 *
 * This file is a part of sample code for the UareU SDK 2.x.
 */

#include "identification.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "helpers.h"

#include <dpfj.h>

void Identification(DPFPDD_DEV hReader){
	const int nFingerCnt = 5;
	unsigned char* vFmd[nFingerCnt];
	unsigned int vFmdSize[nFingerCnt];
	char* vFingerName[nFingerCnt];
	
	//initialization
	int i = 0;
	for(i = 0; i < nFingerCnt; i++){
		vFmd[i] = NULL;
		vFmdSize[i] = 0;
	}
	vFingerName[0] = "your thumb";
	vFingerName[1] = "your index finger";
	vFingerName[2] = "your middle finger";
	vFingerName[3] = "your ring finger";
	vFingerName[4] = "any finger";

	//set green and red LEDs to client-controlled mode
	int result = dpfpdd_led_config(hReader, DPFPDD_LED_ACCEPT | DPFPDD_LED_REJECT, DPFPDD_LED_CLIENT, NULL);
	if(DPFPDD_SUCCESS != result && DPFPDD_E_NOT_IMPLEMENTED != result){
		print_error("dpfpdd_led_config()", result);
	}

	int bStop = 0;
	while(!bStop){
		printf("Identification started\n\n");

		//capture fingers
		for(i = 0; i < nFingerCnt; i++){
			if(0 == CaptureFinger(vFingerName[i], hReader, DPFJ_FMD_ANSI_378_2004, &vFmd[i], &vFmdSize[i])) continue;
			
			bStop = 1;
			break;
		}

		if(!bStop){
			//run identification

			//target false positive identification rate: 0.00001
			//for a discussion of  how to evaluate dissimilarity scores, as well as the statistical validity of the dissimilarity score and error rates, consult the Developer Guide
			unsigned int falsepositive_rate = DPFJ_PROBABILITY_ONE / 100000; 
			unsigned int nCandidateCnt = nFingerCnt;
			DPFJ_CANDIDATE vCandidates[nFingerCnt];
			int result = dpfj_identify(DPFJ_FMD_ANSI_378_2004, vFmd[nFingerCnt - 1], vFmdSize[nFingerCnt - 1], 0,
				DPFJ_FMD_ANSI_378_2004, nFingerCnt - 1, vFmd, vFmdSize, falsepositive_rate, &nCandidateCnt, vCandidates);

			if(DPFJ_SUCCESS == result){
				if(0 != nCandidateCnt){
					//optional: to get false match rate run compare for the top candidate
					unsigned int falsematch_rate = 0;
					result = dpfj_compare(DPFJ_FMD_ANSI_378_2004, vFmd[nFingerCnt - 1], vFmdSize[nFingerCnt - 1], 0, 
						DPFJ_FMD_ANSI_378_2004, vFmd[vCandidates[0].fmd_idx], vFmdSize[vCandidates[0].view_idx], 0, &falsematch_rate);

					//turn green LED on for 1 sec
					dpfpdd_led_ctrl(hReader, DPFPDD_LED_ACCEPT, DPFPDD_LED_CMD_ON);
					sleep(1);
					dpfpdd_led_ctrl(hReader, DPFPDD_LED_ACCEPT, DPFPDD_LED_CMD_OFF);

					//print out the results
					printf("Fingerprint identified, %s\n", vFingerName[vCandidates[0].fmd_idx]);
					printf("dissimilarity score: 0x%x.\n", falsematch_rate);
					printf("false match rate: %e.\n\n\n", (double)(falsematch_rate / DPFJ_PROBABILITY_ONE));
				}
				else{
					//turn red LED on for 1 sec
					dpfpdd_led_ctrl(hReader, DPFPDD_LED_REJECT, DPFPDD_LED_CMD_ON);
					sleep(1);
					dpfpdd_led_ctrl(hReader, DPFPDD_LED_REJECT, DPFPDD_LED_CMD_OFF);

					//print out the results
					printf("Fingerprint was not identified.\n\n\n");
				}
			}
			else print_error("dpfj_identify()", result);
		}

		//release memory
		for(i = 0; i < nFingerCnt; i++){
			if(NULL != vFmd[i]) free(vFmd[i]);
			vFmd[i] = NULL;
			vFmdSize[i] = 0;
		}
	}
}

