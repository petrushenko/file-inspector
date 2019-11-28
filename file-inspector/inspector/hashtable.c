//////////////////////////////////////////////////////////////////////
//
// hashtable.c - функции работы с хеш-таблицей
//
//////////////////////////////////////////////////////////////////////

#include "hashtable.h"

/* http://en.wikipedia.org/wiki/Jenkins_hash_function */
uint32_t _htable_hash(const WCHAR *key, const size_t key_len) {
	uint32_t hash, i;
	for (hash = i = 0; i < key_len; ++i) {
		hash += key[i];
		hash += (hash << 10);
		hash ^= (hash >> 6);
	}
	hash += (hash << 3);
	hash ^= (hash >> 11);
	hash += (hash << 15);
	return hash % (HASHTABLE_MIN_SIZE);
}
/*
Инициализация хеш-таблицы
*/
void ht_init(struct ht_item_t **tbl)
{
	//tbl = malloc(sizeof(ht_item_t)*HASHTABLE_MIN_SIZE);
	for (int i = 0; i < HASHTABLE_MIN_SIZE; i++) {
		tbl[i] = NULL;
	}
}
/*
Добавление в хеш таблицу
*/
void ht_add(struct ht_item_t **tbl, WCHAR *fname, uint32_t crc32) {

	ht_item_t *item;
	uint32_t index = _htable_hash(fname, wcslen(fname));

	item = malloc(sizeof(ht_item_t));
	if (item) {
		wcscpy_s((item->fname), PATHMAXSIZE, fname);
		item->crc32 = crc32;
		item->pnext = tbl[index];
		tbl[index] = item;
	}
}
/*
Получение СПИСКА найденных элементов из хеш таблицы
*/
ht_item_t *ht_get(struct ht_item_t **tbl, WCHAR *fname)
{
	uint32_t index = _htable_hash(fname, wcslen(fname));

	ht_item_t *ht_item = tbl[index];

	ht_item_t *res = NULL;

	ht_item_t *tmp_res;

	while (ht_item) {
		if (wcscmp(fname, ht_item->fname) == 0) {
			tmp_res = malloc(sizeof(ht_item_t));
			if (tmp_res) {
				tmp_res->crc32 = ht_item->crc32;
				//tmp_res->fname = ht_item->fname;
				wcscpy_s(tmp_res->fname, PATHMAXSIZE, ht_item->fname);
				tmp_res->pnext = res;
				res = tmp_res;
			}
			else
				return NULL;
		}
		ht_item = ht_item->pnext;
	}
	return res;
}

/*
Удаление из хеш таблицы
*/
void ht_delete(struct ht_item_t **tbl, WCHAR *fname, uint32_t crc32)
{
	uint32_t index = _htable_hash(fname, wcslen(fname));
	
	ht_item_t *tmp_ptr, *to_free;

	tmp_ptr = tbl[index];

	if (tmp_ptr && wcscmp(tmp_ptr->fname, fname) == 0 && tmp_ptr->crc32 == crc32) {
		tbl[index] = tmp_ptr->pnext;
		free(tmp_ptr);
		return;
	}
	
	while (tmp_ptr && tmp_ptr->pnext) {
		if (wcscmp(tmp_ptr->pnext->fname, fname) == 0 && tmp_ptr->pnext->crc32 == crc32) {
			to_free = tmp_ptr->pnext;
			tmp_ptr->pnext = tmp_ptr->pnext->pnext;
			free(to_free);
		}
		tmp_ptr = tmp_ptr->pnext;
	}
}

/*
Очистка хеш таблицы
*/
void ht_clear(struct ht_item_t **tbl) 
{
	ht_init(tbl);
}

void ht_destroy(struct ht_item_t **tbl) 
{
	ht_item_t *ht_item, *prev_ht_item;

	for (int i = 0; i < HASHTABLE_MIN_SIZE; i++) {
		ht_item = tbl[i];
		while (ht_item) {
			prev_ht_item = ht_item;
			ht_item = ht_item->pnext;
			ht_delete(tbl, prev_ht_item->fname, prev_ht_item->crc32);
		}
	}
	free(tbl);
}

//Поиск изменений на диске
uint32_t _ht_chngs(ht_item_t **newf, ht_item_t **del, ht_item_t **chn, const WCHAR *fname, const WCHAR *dir, HANDLE hProgressBar)
{
	htable_t *hash_table = _ht_init();

	ht_item_t *ht_item = NULL, *changed = NULL, *deleted = NULL, *pnew = NULL, *tmpPtr = NULL;

	FILE *f;
	WCHAR new_fname[PATHMAXSIZE] = L"tmp_";
	wcscat(new_fname, fname);

	errno_t err = _wfopen_s(&f, fname, L"rb");

	uint32_t crc = 0; //crc32 прочитанного файла
	WCHAR filename[PATHMAXSIZE]; //имя прочитанного файла
	
	if (!err && f) {		
		ListDirectoryContents(dir, hash_table->ht_items, hProgressBar);
		fhash_save(hash_table->ht_items, new_fname);
		while (!feof(f)) {
			fgetws(filename, PATHMAXSIZE, f);
			if (filename)
				filename[wcslen(filename) - 1] = L'\0';
			else
				return -1;
			fwscanf_s(f, L"%u\n", &crc);
			ht_item = ht_get(hash_table->ht_items, filename);
			
			if (!ht_item) {
				tmpPtr = malloc(sizeof(ht_item_t));
				tmpPtr->crc32 = crc;
				wcscpy_s(tmpPtr->fname, PATHMAXSIZE, filename);
				tmpPtr->pnext = deleted;
				deleted = tmpPtr;
			}
			else {
				while (ht_item) {
					if (wcscmp(ht_item->fname, filename) == 0) {
						if (ht_item->crc32 != crc) {
							tmpPtr = malloc(sizeof(ht_item_t));
							tmpPtr->crc32 = ht_item->crc32;
							wcscpy_s(tmpPtr->fname, PATHMAXSIZE, ht_item->fname);
							tmpPtr->pnext = changed;
							changed = tmpPtr;
							ht_delete(hash_table->ht_items, tmpPtr->fname, tmpPtr->crc32);
						}
						else//удаление из хеш-таблицы
							ht_delete(hash_table->ht_items, filename, crc);
					}
					ht_item = ht_item->pnext;
				}
			}
		}
		fclose(f);
		for (uint32_t i = 0; i < HASHTABLE_MIN_SIZE; i++) {
			ht_item = hash_table->ht_items[i];
			while (ht_item) {
				tmpPtr = malloc(sizeof(ht_item_t));
				tmpPtr->crc32 = ht_item->crc32;
				wcscpy_s(tmpPtr->fname, PATHMAXSIZE, ht_item->fname);
				tmpPtr->pnext = pnew;
				pnew = tmpPtr;
				ht_item = ht_item->pnext;
				ht_delete(hash_table->ht_items, tmpPtr->fname, tmpPtr->crc32);
			}
		}
		fhash_load(hash_table->ht_items, new_fname);
		fhash_save(hash_table->ht_items, fname);
		DeleteFileW(new_fname);

		ht_destroy(hash_table->ht_items);
		free(hash_table);
	}
	else
		return -1;

	*del = deleted;
	*chn = changed;
	*newf = pnew;

	return 0;
}
/*
Инициализация хеш-таблицы
*/
htable_t *_ht_init(void)
{
	ht_item_t **_ht_items;
	htable_t *tbl = malloc(sizeof(htable_t));

	if (!tbl) {
		return NULL;
	}

	tbl->dir = L"\0";
	tbl->nitems = 0;
	tbl->size = HASHTABLE_MIN_SIZE;

	_ht_items = malloc(sizeof(ht_item_t*) * tbl->size);

	ht_init(_ht_items);

	if (!_ht_items) {
		free(tbl);
		return NULL;
	}
	memset(_ht_items, 0, sizeof(ht_item_t*) * tbl->size);
	tbl->ht_items = _ht_items;

	return tbl;
}