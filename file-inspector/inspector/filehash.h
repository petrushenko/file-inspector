#pragma once

#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"
#include "hashfunc.h"
#include <stdint.h>
#include <Windows.h>

/*
Буфер для чтения из файла
*/
#define BUFFSIZE 0xFFFF
typedef WCHAR byte_t;
byte_t buffer[BUFFSIZE];

uint_least32_t fgetcrc32(WCHAR *filename);
int fhash_save(struct ht_item_t **tbl, const WCHAR *outfilename);
int fhash_load(struct ht_item_t **tbl, const WCHAR *infilename);