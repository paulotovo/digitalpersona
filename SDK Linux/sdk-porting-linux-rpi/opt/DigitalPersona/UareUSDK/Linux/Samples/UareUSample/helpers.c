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
		case DPFJ_E_INVALID_FID: szError = "FID is invalid."; break;
		case DPFJ_E_TOO_SMALL_AREA: szError = "Image is too small."; break;
		case DPFJ_E_INVALID_FMD: szError = "FMD is invalid."; break;
		case DPFJ_E_ENROLLMENT_IN_PROGRESS: szError = "Enrollment operation is in progress."; break;
		case DPFJ_E_ENROLLMENT_NOT_STARTED: szError = "Enrollment operation has not begun."; break;
		case DPFJ_E_ENROLLMENT_NOT_READY: szError = "Not enough in the pool of FMDs to create enrollment FMD."; break;
		case DPFJ_E_ENROLLMENT_INVALID_SET: szError = "Unable to create enrollment FMD with the collected set of FMDs."; break;
		case DPFJ_E_COMPRESSION_IN_PROGRESS: szError = "Compression or decompression operation is in progress"; break;
		case DPFJ_E_COMPRESSION_NOT_STARTED: szError = "Compression or decompression operation was not started."; break;
		case DPFJ_E_COMPRESSION_INVALID_WSQ_PARAMETER: szError = "One or more parameters passed for WSQ compression are invalid."; break;
		case DPFJ_E_COMPRESSION_WSQ_FAILURE: szError = "Unspecified error during WSQ compression or decompression."; break;
		case DPFJ_E_COMPRESSION_WSQ_LIB_NOT_FOUND: szError = "Library for WSQ compression is not found or not built-in."; break;
		case DPFJ_E_QUALITY_NO_IMAGE: szError = "Image is invalid or absent."; break;
		case DPFJ_E_QUALITY_TOO_FEW_MINUTIA: szError = "Too few minutia detected in the fingerprint image."; break;
		case DPFJ_E_QUALITY_FAILURE: szError = "Unspecified error during execution."; break;
		case DPFJ_E_QUALITY_LIB_NOT_FOUND: szError = "Library for image quality is not found or not built-in."; break;
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

void signal_handler(int nSignal) {
	if(SIGINT == nSignal){
		//cancel capture
		if(NULL != g_hReader) dpfpdd_cancel(g_hReader);
	}
}

int CaptureFinger(const char* szFingerName, DPFPDD_DEV hReader, DPFJ_FMD_FORMAT nFtType, unsigned char** ppFt, unsigned int* pFtSize){
	int result = 0;
	*ppFt = NULL;
	*pFtSize = 0;

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
	
	while(1){
		//wait until ready
		int is_ready = 0;
		while(1){
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

		//capture fingerprint
		printf("Put %s on the reader, or press Ctrl-C to cancel...\r\n", szFingerName);
		result = dpfpdd_capture(hReader, &cparam, -1, &cresult, &nImageSize, pImage);
		if(DPFPDD_SUCCESS != result){
			print_error("dpfpdd_capture()", result);
		}
		else{
			if(cresult.success){
				//captured
				printf("    fingerprint captured,\n");

				//get max size for the feature template
				unsigned int nFeaturesSize = MAX_FMD_SIZE;
				unsigned char* pFeatures = (unsigned char*)malloc(nFeaturesSize);
				if(NULL == pFeatures){
					print_error("malloc()", ENOMEM);
					result = ENOMEM;
				}
				else{
					//create template
					result = dpfj_create_fmd_from_fid(DPFJ_FID_ISO_19794_4_2005, pImage, nImageSize, nFtType, pFeatures, &nFeaturesSize);

					if(DPFJ_SUCCESS == result){
						*ppFt = pFeatures;
						*pFtSize = nFeaturesSize;
						printf("    features extracted.\n\n");
					}
					else{
						print_error("dpfj_create_fmd_from_fid()", result);
						free(pFeatures);
					}
				}
			}
			else if(DPFPDD_QUALITY_CANCELED == cresult.quality){
				//capture canceled
				result = EINTR;
			}
			else{
				//bad capture
				printf("    bad capture, quality feedback: 0x%x\n", cresult.quality);
				unsigned int i = 0;
				for(i = 1; i < 0x80000000; i <<= 1){
					switch(cresult.quality & i){
					case 0: break;
					case DPFPDD_QUALITY_TIMED_OUT:            printf("    timeout expired \n"); break;
					case DPFPDD_QUALITY_CANCELED:             printf("    capture was canceled \n"); break;
					case DPFPDD_QUALITY_NO_FINGER:            printf("    not a finger detected \n"); break;
					case DPFPDD_QUALITY_FAKE_FINGER:          printf("    fake finger detected \n"); break;
					case DPFPDD_QUALITY_FINGER_TOO_LEFT:      printf("    finger is too far left on the reader \n"); break;
					case DPFPDD_QUALITY_FINGER_TOO_RIGHT:     printf("    finger is too far right on the reader \n"); break;
					case DPFPDD_QUALITY_FINGER_TOO_HIGH:      printf("    finger is too high on the reader \n"); break;
					case DPFPDD_QUALITY_FINGER_TOO_LOW:       printf("    finger is too low in the reader \n"); break;
					case DPFPDD_QUALITY_FINGER_OFF_CENTER:    printf("    finger is not centered on the reader \n"); break;
					case DPFPDD_QUALITY_SCAN_SKEWED:          printf("    scan is skewed too much \n"); break;
					case DPFPDD_QUALITY_SCAN_TOO_SHORT:       printf("    scan is too short \n"); break;
					case DPFPDD_QUALITY_SCAN_TOO_LONG:        printf("    scan is too long \n"); break;
					case DPFPDD_QUALITY_SCAN_TOO_SLOW:        printf("    speed of the swipe is too slow \n"); break;
					case DPFPDD_QUALITY_SCAN_TOO_FAST:        printf("    speed of the swipe is too fast \n"); break;
					case DPFPDD_QUALITY_SCAN_WRONG_DIRECTION: printf("    direction of the swipe is wrong \n"); break;
					case DPFPDD_QUALITY_READER_DIRTY:         printf("    reader needs cleaning \n"); break;
					default: printf("    unknown quality bitmask: 0x%x \n", i); break;
					}
				}
				continue;
			}
		}
		break;
	}
	
	//restore signal mask
	sigprocmask(SIG_SETMASK, &old_sigmask, NULL);
	
	//restore signal handler
	sigaction (SIGINT, &old_action, NULL);
	g_hReader = NULL;
	
	if(NULL != pImage) free(pImage);
	return result;
}


