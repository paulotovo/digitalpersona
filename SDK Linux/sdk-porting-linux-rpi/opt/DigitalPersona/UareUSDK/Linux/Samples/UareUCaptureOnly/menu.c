/* 
 * Copyright (C) 2011, Digital Persona, Inc.
 *
 * This file is a part of sample code for the UareU SDK 2.x.
 */

#include "menu.h" 

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int Menu_Create(const char* szTitle, int nType, menu_t** ppMenu){
	*ppMenu = (menu_t*)malloc(sizeof(menu_t));
	if(NULL != *ppMenu){
		menu_t* pMenu = *ppMenu;
		strncpy(pMenu->szTitle, szTitle, sizeof(pMenu->szTitle));
		pMenu->szTitle[sizeof(pMenu->szTitle) - 1] = '\0';
		pMenu->nType = nType;
		pMenu->nItemsCnt = 0;
		pMenu->pItems = NULL;
		return 0;
	}
	else return ENOMEM;
}

void Menu_Destroy(menu_t* pMenu){
	if(NULL == pMenu) return;
	if(NULL != pMenu->pItems) free(pMenu->pItems);
	free(pMenu);
}

int Menu_AddItem(menu_t* pMenu, int nItemValue, const char* szItem){
	if(NULL == pMenu || NULL == szItem) return EINVAL;
	if(9 <= pMenu->nItemsCnt) return ERANGE;
	
	size_t nItem = 0;
	for(; nItem < pMenu->nItemsCnt; nItem++){
		if(pMenu->pItems[nItem].nValue != nItemValue) continue;
		break;
	}
	
	if(nItem < pMenu->nItemsCnt){
		strncpy(pMenu->pItems[nItem].szItem, szItem, sizeof(pMenu->pItems[nItem].szItem));
	}
	else{
		menu_item_t* pNewItems = (menu_item_t*)realloc(pMenu->pItems, sizeof(menu_item_t) * (pMenu->nItemsCnt + 1));
		if(NULL == pNewItems) return ENOMEM;

		pMenu->pItems = pNewItems;
		strncpy(pMenu->pItems[pMenu->nItemsCnt].szItem, szItem, sizeof(pMenu->pItems[pMenu->nItemsCnt].szItem));
		pMenu->pItems[pMenu->nItemsCnt].nValue = nItemValue;
		pMenu->nItemsCnt++;
	}
	return 0;
}

int Menu_DoModal(menu_t* pMenu, int* pItemValue){
	if(NULL == pMenu || NULL == pItemValue) return EINVAL;
	
	while(1){
		printf("\n\n\n%s\n\n", pMenu->szTitle);
		int i = 0;
		for(i = 0; i < pMenu->nItemsCnt; i++){
			printf("  %d: %s\n", i + 1, pMenu->pItems[i].szItem);
		}
		
		if(2 < pMenu->nItemsCnt) printf("\nEnter %d - %d", 1, pMenu->nItemsCnt);
		else if(2 == pMenu->nItemsCnt) printf("\nEnter 1, 2");
		else  printf("\nEnter 1");
		if(pMenu->nType & MENU_TYPE_BACK) printf(", or \'B\' to go back");
		if(pMenu->nType & MENU_TYPE_EXIT) printf(", or \'E\' to exit");
		printf(": ");
		
		char szBuffer[100];
		fgets(szBuffer, sizeof(szBuffer), stdin);
		char ch = szBuffer[0];

		if('1' <= ch && pMenu->nItemsCnt + '0' >= ch){
			*pItemValue = pMenu->pItems[ch - '1'].nValue;
			return 0;
		}
		if((pMenu->nType & MENU_TYPE_BACK) && ('B' == ch || 'b' == ch)){
			*pItemValue = -1;
			return 0;
		}
		if((pMenu->nType & MENU_TYPE_EXIT) && ('E' == ch || 'e' == ch)){
			*pItemValue = -2;
			return 0;
		}
		
		printf("Invalid choice: %c", ch);
	}
}
