#pragma once

#include <Windows.h>
#include <stdio.h>
#include "hashtable.h"
#include <Commctrl.h>
//#include "filehash.h"
#include <stdbool.h>
#include <string.h>


bool ListDirectoryContents(const WCHAR *sDir, ht_item_t **tbl, HANDLE hProgressBar);
int fname_to_str(WCHAR *fname);
void recursiveFilesCount(const WCHAR Path[], int* nCount);



