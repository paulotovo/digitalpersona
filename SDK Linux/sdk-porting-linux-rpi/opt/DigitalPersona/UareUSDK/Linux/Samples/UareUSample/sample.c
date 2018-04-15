/* 
 * Copyright (C) 2011, Digital Persona, Inc.
 *
 * This file is a part of sample code for the UareU SDK 2.x.
 */

#include "helpers.h"
#include "menu.h"
#include "selection.h"
#include "verification.h"
#include "identification.h"
#include "enrollment.h"

#include <dpfpdd.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <locale.h>


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// main

int main(int argc, char** argv){
	//block all signals, SIGINT will be unblocked and handled in CaptureFinger()
	sigset_t sigmask;
	sigfillset(&sigmask);
	sigprocmask(SIG_BLOCK, &sigmask, NULL);
	
	setlocale(LC_ALL, "");
	
	//initialize capture library
	int result = dpfpdd_init();
	if(DPFPDD_SUCCESS != result) print_error("dpfpdd_init()", result);
	else{
		DPFPDD_DEV hReader = NULL; //handle of the selected reader
		char szReader[MAX_DEVICE_NAME_LENGTH]; //name of the selected reader
		
		menu_t* pMenu = NULL;
		int res = Menu_Create("UareU SDK 2.x sample application (verification, identification, enrollment)", MENU_TYPE_EXIT, &pMenu);
		if(0 == res) res = Menu_AddItem(pMenu, 101, "Select new reader (not selected)");
		if(0 == res) res = Menu_AddItem(pMenu, 102, "Run verification");
		if(0 == res) res = Menu_AddItem(pMenu, 103, "Run identification");
		if(0 == res) res = Menu_AddItem(pMenu, 104, "Run enrollment");
		if(0 == res){
			//main menu loop
			int bStop = 0;
			while(!bStop){
				int nChoice = 0;
				Menu_DoModal(pMenu, &nChoice);
				
				switch(nChoice){
					case 101: //select reader
						//close reader if opened
						if(NULL != hReader){
							result = dpfpdd_close(hReader);
							if(DPFPDD_SUCCESS != result) print_error("dpfpdd_close()", result);
							hReader = NULL;
						}
						//open new reader
						hReader = SelectAndOpenReader(szReader, sizeof(szReader));
						if(NULL != hReader){
							char szItem[MAX_DEVICE_NAME_LENGTH + 20];
							snprintf(szItem, sizeof(szItem), "Select new reader (selected: %s)", szReader);
							Menu_AddItem(pMenu, 101, szItem);
						}
						else{
							Menu_AddItem(pMenu, 101, "Select new reader (not selected)");
						}
						break;
					case 102: //run verification
						if(NULL == hReader){
							printf("\nReader is not selected!");
						}
						else{
							Verification(hReader);
						}
						break;
					case 103: //run identification
						if(NULL == hReader){
							printf("\nReader is not selected!");
						}
						else{
							Identification(hReader);
						}
						break;
					case 104: //run enrollment
						if(NULL == hReader){
							printf("\nReader is not selected!");
						}
						else{
							Enrollment(hReader);
						}
						break;
					case -2: //exit
						bStop = 1;
						break;
				}
			}
			
			Menu_Destroy(pMenu);
		}
		else print_error("Menu_Create() or Menu_AddItem()", res);
		
		//close reader
		if(NULL != hReader){
			result = dpfpdd_close(hReader);
			if(DPFPDD_SUCCESS != result) print_error("dpfpdd_close()", result);
			hReader = NULL;
		}
		
		//relese capture library
		dpfpdd_exit(); 
	}
	
	return 0;
}

