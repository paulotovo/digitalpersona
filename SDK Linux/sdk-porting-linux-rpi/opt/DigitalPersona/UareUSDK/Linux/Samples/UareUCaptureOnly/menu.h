/* 
 * Copyright (C) 2011, Digital Persona, Inc.
 *
 * This file is a part of sample code for the UareU SDK 2.x.
 */

#pragma once

#define MAX_MENU_STRING_LEN 120

#define MENU_TYPE_BACK 0x1
#define MENU_TYPE_EXIT 0x2

typedef struct {
	char szItem[MAX_MENU_STRING_LEN];
	int  nValue;
} menu_item_t;

typedef struct {
	char szTitle[MAX_MENU_STRING_LEN];
	int  nType;
	int  nItemsCnt;
	menu_item_t* pItems;
} menu_t;

int Menu_Create(const char* szTitle, int nType, menu_t** ppMenu);
void Menu_Destroy(menu_t* pMenu);
int  Menu_AddItem(menu_t* pMenu, int nItemValue, const char* szItem);
int  Menu_DoModal(menu_t* pMenu, int* pItemValue);
