/* 
 * Copyright (C) 2011, Digital Persona, Inc.
 *
 * This file is a part of sample code for the UareU SDK 2.x.
 */

#include "selection.h" 
#include "menu.h"
#include "helpers.h"

#include <dpfpdd.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>


DPFPDD_DEV SelectAndOpenReader(char* szReader, size_t nReaderLen){
	DPFPDD_DEV hReader = NULL;
	strncpy(szReader, "", nReaderLen);
	int bStop = 0;
	
	while(!bStop){
		//enumerate the readers
		unsigned int nReaderCnt = 1;
		DPFPDD_DEV_INFO* pReaderInfo = (DPFPDD_DEV_INFO*)malloc(sizeof(DPFPDD_DEV_INFO) * nReaderCnt);
		while(NULL != pReaderInfo){
			unsigned int i = 0;
			for(i = 0; i < nReaderCnt; i++){
				pReaderInfo[i].size = sizeof(DPFPDD_DEV_INFO);
			}
			
			unsigned int nNewReaderCnt = nReaderCnt;
			int result = dpfpdd_query_devices(&nNewReaderCnt, pReaderInfo);
			
			//quit if error
			if(DPFPDD_SUCCESS != result && DPFPDD_E_MORE_DATA != result){
				print_error("dpfpdd_query_devices()", result);
				free(pReaderInfo);
				nReaderCnt = 0;
				break;
			}
			
			//allocate memory if needed and do over
			if(DPFPDD_E_MORE_DATA == result){
				DPFPDD_DEV_INFO* pri = (DPFPDD_DEV_INFO*)realloc(pReaderInfo, sizeof(DPFPDD_DEV_INFO) * nNewReaderCnt);
				if(NULL == pri){
					print_error("realloc()", ENOMEM);
					break;
				}
				pReaderInfo = pri;
				nReaderCnt = nNewReaderCnt;
				continue;
			}
			
			//success
			nReaderCnt = nNewReaderCnt;
			break;
		}
		
		//list readers
		if(0 != nReaderCnt){
			printf("\n\nAvailable readers:\n");
			unsigned int i = 0;
			for(i = 0; i < nReaderCnt; i++){
				printf("  %s\n", pReaderInfo[i].name);
			}
		}
		else printf("\n\nNo readers available\n");
		
		//put menu on screen
		menu_t* pMenu = NULL;
		int result = Menu_Create("Reader selection", MENU_TYPE_BACK, &pMenu);
		unsigned int i = 0;
		for(i = 0; i < nReaderCnt; i++){
			char szBuffer[1024];
			snprintf(szBuffer, sizeof(szBuffer), "Select %s", pReaderInfo[i].name);
			if(0 == result) result = Menu_AddItem(pMenu, i, szBuffer);
		}
		if(0 == result) result = Menu_AddItem(pMenu, nReaderCnt, "Refresh reader list");
		if(0 == result){
			int nChoice = 0;
			Menu_DoModal(pMenu, &nChoice);
			
			if(-1 == nChoice){
				//back
				bStop = 1;
			}
			if(nReaderCnt > nChoice){
				//reader selected, print out the info
				printf("\n");
				printf("Selected reader:  %s\n", pReaderInfo[nChoice].name);
				printf("Vendor name:      %s\n", pReaderInfo[nChoice].descr.vendor_name);
				printf("Product name:     %s\n", pReaderInfo[nChoice].descr.product_name);
				printf("Serial number:    %s\n", pReaderInfo[nChoice].descr.serial_num);
				printf("USB VID:          %04x\n", pReaderInfo[nChoice].id.vendor_id);
				printf("USB PID:          %04x\n", pReaderInfo[nChoice].id.product_id);
				printf("USB BCD revision: %04x\n", pReaderInfo[nChoice].ver.bcd_rev);
				printf("HW version:       %d.%d.%d\n", pReaderInfo[nChoice].ver.hw_ver.major, pReaderInfo[nChoice].ver.hw_ver.minor, pReaderInfo[nChoice].ver.hw_ver.maintenance);
				printf("FW version:       %d.%d.%d\n", pReaderInfo[nChoice].ver.fw_ver.major, pReaderInfo[nChoice].ver.fw_ver.minor, pReaderInfo[nChoice].ver.fw_ver.maintenance);
				
				//open reader
				result = dpfpdd_open(pReaderInfo[nChoice].name, &hReader);
				if(DPFPDD_SUCCESS == result){
					strncpy(szReader, pReaderInfo[nChoice].name, nReaderLen);
					
					//read capabilities
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
						//capabilities acquired, print them out
						printf("\n");
						printf("can capture image:       %d\n", pCaps->can_capture_image);
						printf("can stream image:        %d\n", pCaps->can_stream_image);
						printf("can extract features:    %d\n", pCaps->can_extract_features);
						printf("can match:               %d\n", pCaps->can_match);
						printf("can identify:             %d\n", pCaps->can_identify);
						printf("has fingerprint storage: %d\n", pCaps->has_fp_storage);
						printf("indicator type:          %d\n", pCaps->indicator_type);
						printf("has power management:    %d\n", pCaps->has_pwr_mgmt);
						printf("has calibration:         %d\n", pCaps->has_calibration);
						printf("PIV compliant:           %d\n", pCaps->piv_compliant);
						unsigned int i = 0;
						for(i = 0; i < pCaps->resolution_cnt; i++){
							printf("resolution:              %d dpi\n", pCaps->resolutions[i]);
						}
						free(pCaps);
						break;
					}
				}
				else print_error("dpfpdd_open()", result);
				
				printf("\n");
				bStop = 1;
			}
			Menu_Destroy(pMenu);
		}
		else print_error("Menu_Create() or Menu_AddItem()", result);
		
		//release memory
		if(NULL != pReaderInfo) free(pReaderInfo);
		pReaderInfo = NULL;
		nReaderCnt = 0;
	}
	
	return hReader;
}

