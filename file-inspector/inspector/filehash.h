#pragma once

#include "hashfunc.h"
#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"
#include <Windows.h>

/*
Буфер для чтения из файла
*/
#define BUFFSIZE 0xFFFF
typedef unsigned char byte_t;
byte_t buffer[BUFFSIZE];

uint_least32_t fgetcrc32(char *filename);
int fhash_save(struct ht_item_t **tbl, const char *outfilename);
int fhash_load(struct ht_item_t **tbl, const char *infilename);