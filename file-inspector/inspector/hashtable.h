#pragma once

#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <stdio.h>

#include "filehash.h"

#define UNUCODE
#define _UNICODE

#define HASHTABLE_MIN_SIZE USHRT_MAX
#define PATHMAXSIZE MAX_PATH+1

//Элемент хэш таблицы
typedef struct _ht_item {
	uint32_t crc32;
	WCHAR fname[PATHMAXSIZE];
	struct _ht_item *pnext;
} ht_item_t, *ht_item_p;

//Хеш-таблица
//Решение коллизий - цепочки
//struct ht_item_t *hash_table[HASHTABLE_MIN_SIZE];
//struct ht_item_t **hash_table;

typedef struct _htable {
	ht_item_t **ht_items;
	WCHAR *dir;
	size_t size;
	size_t nitems;
} htable_t;

void ht_init(struct ht_item_t **tbl);
htable_t *_ht_init(void);
void ht_clear(struct ht_item_t **tbl);
void ht_add(struct ht_item_t **tbl, WCHAR *fname, uint32_t crc32);
ht_item_t *ht_get(struct ht_item_t **tbl, WCHAR *fname);
uint32_t _ht_chngs(ht_item_t **newf, ht_item_t **del, ht_item_t **chn, const WCHAR *fname, const WCHAR *dir, HANDLE hProgressBar);
void ht_delete(struct ht_item_t **tbl, WCHAR *fname, uint32_t crc32);
void ht_destroy(struct ht_item_t **tbl);