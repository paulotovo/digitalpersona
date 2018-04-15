/* 
 * Copyright (C) 2011, Digital Persona, Inc.
 *
 * This file is a part of sample code for the UareU SDK 2.x.
 */

#include "helpers.h" 

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// error handling

void print_error(const char* szFunctionName, int nError){
	char sz[256];
	if(_DP_FACILITY == (nError >> 16)){
		char* szError = NULL;
		switch(nError){
		case DPFPDD_E_NOT_IMPLEMENTED: szError = "API call is not implemented."; break;
		case DPFPDD_E_FAILURE: szError = "Unspecified failure."; break;
		case DPFPDD_E_NO_DATA: szError = "No data is available."; break;
		case DPFPDD_E_MORE_DATA: szError = "The memory allocated by the application is not big enough for the data which is expected."; break;
		case DPFPDD_E_INVALID_PARAMETER: szError = "One or more parameters passed to the API call are invalid."; break;
		case DPFPDD_E_INVALID_DEVICE: szError = "Reader handle is not valid."; break;
		case DPFPDD_E_DEVICE_BUSY: szError = "The API call cannot be completed because another call is in progress."; break;
		case DPFPDD_E_DEVICE_FAILURE: szError = "The reader is not working properly."; break;
		}
		sprintf(sz, "%s returned DP error: 0x%x \n%s", szFunctionName, (0xffff & nError), szError);
	}
	else{
		sprintf(sz, "%s returned system error: 0x%x", szFunctionName, (0xffff & nError));
	}
	printf("%s \n\n", sz);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// capture

DPFPDD_DEV g_hReader = NULL;
int        g_bCancel = 0;

void signal_handler(int nSignal) {
	if(SIGINT == nSignal){
		g_bCancel = 1;
		//cancel capture
		if(NULL != g_hReader) dpfpdd_cancel(g_hReader);
	}
}

int CaptureFinger(DPFPDD_DEV hReader, int bStream){
	int result = 0;

	if(bStream){
		//check if streaming supported
		unsigned int nCapsSize = sizeof(DPFPDD_DEV_CAPS);
		while(1){
			DPFPDD_DEV_CAPS* pCaps = (DPFPDD_DEV_CAPS*)malloc(nCapsSize);
			if(NULL == pCaps){
				print_error("malloc()", ENOMEM);
				break;
			}
			pCaps->size = nCapsSize;
			result = dpfpdd_get_device_capabilities(hReader, pCaps);

			if(DPFPDD_SUCCESS != result && DPFPDD_E_MORE_DATA != result){
				print_error("dpfpdd_get_device_capabilities()", result);
				free(pCaps);
				break;
			}
			if(DPFPDD_E_MORE_DATA == result){
				nCapsSize = pCaps->size;
				free(pCaps);
				continue;
			}

			if(0 == pCaps->can_stream_image){
				printf("this reader cannot work in streaming mode \n\n");
				free(pCaps);
				return 0;
			}

			free(pCaps);
			break;
		}
	}

	//prepare capture parameters and result
	DPFPDD_CAPTURE_PARAM cparam = {0};
	cparam.size = sizeof(cparam);
	cparam.image_fmt = DPFPDD_IMG_FMT_ISOIEC19794;
	cparam.image_proc = DPFPDD_IMG_PROC_NONE;
	cparam.image_res = 500;
	DPFPDD_CAPTURE_RESULT cresult = {0};
	cresult.size = sizeof(cresult);
	cresult.info.size = sizeof(cresult.info);
	//get size of the image
	unsigned int nImageSize = 0;
	result = dpfpdd_capture(hReader, &cparam, 0, &cresult, &nImageSize, NULL);
	if(DPFPDD_E_MORE_DATA != result){
		print_error("dpfpdd_capture()", result);
		return result;
	}

	unsigned char* pImage = (unsigned char*)malloc(nImageSize);
	if(NULL == pImage){
		print_error("malloc()", ENOMEM);
		return ENOMEM;
	}
		
	//set signal handler
	g_hReader = hReader;
	struct sigaction new_action, old_action;
	new_action.sa_handler = &signal_handler;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;
	sigaction(SIGINT, &new_action, &old_action);

	//unblock SIGINT (Ctrl-C)
	sigset_t new_sigmask, old_sigmask;
	sigemptyset(&new_sigmask);
	sigaddset(&new_sigmask, SIGINT);
	sigprocmask(SIG_UNBLOCK, &new_sigmask, &old_sigmask);
	
	g_bCancel = 0;
	while(!g_bCancel){
		//wait until ready
		int is_ready = 0;
		while(!g_bCancel){
			DPFPDD_DEV_STATUS ds;
			ds.size = sizeof(DPFPDD_DEV_STATUS);
			result = dpfpdd_get_device_status(hReader, &ds);
			if(DPFPDD_SUCCESS != result){
				print_error("dpfpdd_get_device_status()", result);
				break;
			}
			
			if(DPFPDD_STATUS_FAILURE == ds.status){
				print_error("Reader failure", DPFPDD_STATUS_FAILURE);
				break;
			}
			if(DPFPDD_STATUS_READY == ds.status || DPFPDD_STATUS_NEED_CALIBRATION == ds.status){
				is_ready = 1;
				break;
			}
		}
		if(!is_ready) break;

		if(0 == bStream){
			//capture fingerprint
			printf("Put your finger on the reader, or press Ctrl-C to cancel...\r\n");
			result = dpfpdd_capture(hReader, &cparam, -1, &cresult, &nImageSize, pImage);
			if(DPFPDD_SUCCESS != result){
				print_error("dpfpdd_capture()", result);
			}
			else{
				if(cresult.success){
					//captured
					printf("    fingerprint captured\n");
					printf("    width: %d, height: %d \n\n", cresult.info.width, cresult.info.height);
				}
				else if(DPFPDD_QUALITY_CANCELED == cresult.quality){
					//capture canceled
					printf("    fingerprint capture canceled\n");
					result = EINTR;
				}
				else{
					//bad capture
					printf("    bad capture, quality feedback: 0x%x.\n", cresult.quality);
					continue;
				}
			}
			break;
		}
		else{
			//stream fingerprints
			int nCnt = 0;
			int bFailure = 0;

			result = dpfpdd_start_stream(hReader);
			if(DPFPDD_SUCCESS != result){
				print_error("dpfpdd_start_stream()", result);
			}

			while(DPFPDD_SUCCESS == result && !g_bCancel){
				result = dpfpdd_get_stream_image(hReader, &cparam, &cresult, &nImageSize, pImage);
				if(DPFPDD_SUCCESS == result){
					nCnt++;
					printf("    %d image captured, score: %d \n", nCnt, cresult.score);
				}
				else{
					print_error("dpfpdd_get_stream_image()", result);
					bFailure = 1;
					break;
				}
			}

			result = dpfpdd_stop_stream(hReader);
			if(DPFPDD_SUCCESS != result){
				print_error("dpfpdd_stop_stream()", result);
			}

			if(bFailure) break;
		}
	}
	
	//restore signal mask
	sigprocmask(SIG_SETMASK, &old_sigmask, NULL);
	
	//restore signal handler
	sigaction (SIGINT, &old_action, NULL);
	g_hReader = NULL;
	
	if(NULL != pImage) free(pImage);
	return result;
}


