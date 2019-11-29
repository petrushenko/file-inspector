//////////////////////////////////////////////////////////////////////
//
//  filehash.c - реализация функций для работы с файлами
//
/////////////////////////////////////////////////////////////////////

#include "filehash.h"

/*
Получение контрольной суммы из файла
Вычисляется CRC32 с полиномом 0x8005
См. описание Crc32()
*/
uint_least32_t fgetcrc32(WCHAR *filename)
{	
	FILE *f;
	uint_least32_t crc = 0xFFFFFFFFUL;
	errno_t err;
	err = _wfopen_s(&f, filename, L"rb");
	if (err == 0 && f != NULL) {	
		while (!feof(f)) {
			int result = fread(buffer, 1, BUFFSIZE, f);
			if (result) {
				crc = Crc32(buffer, result, crc);
			}
		}
		//crc = Crc32(filename, wcslen(filename), crc);
		fclose(f);
		//int size = (BUFFSIZE * sizeof(byte_t));
		//ZeroMemory(buffer, size);
	}

	// Значение инвертируется в конце для возможности в цикле 
	// вычислять хеш-функцию частями
	return crc ^ 0xFFFFFFFFUL;
}
/*
Сохрание данных о файлах хеш таблицы в файл
Возвращает количество обработанных файлов
*/
int fhash_save(struct ht_item_t **tbl, const WCHAR *outfilename) {
	int res = 0;
	FILE *f;
	errno_t err = _wfopen_s(&f, outfilename, L"wb");
	ht_item_t *tmp_ptr;

	if (err == 0 && f != NULL) {
		for (uint32_t i = 0; i < HASHTABLE_MIN_SIZE; i++) {
			tmp_ptr = *(tbl + i);
			while (tmp_ptr) {
				fwprintf(f, L"%ls\n%u\n", (tmp_ptr->fname), (tmp_ptr->crc32));
				tmp_ptr = tmp_ptr->pnext;
				res += 1;
			}
		}	
		fclose(f);
	}
	return res;
}
/*
Загрузка данных о файлах в хеш таблицу
*/
int fhash_load(struct ht_item_t **tbl, const WCHAR *infilename) {

	int res = 0;
	FILE *f;
	uint32_t crc = 0; //crc32 прочитанного файла
	WCHAR filename[PATHMAXSIZE]; //имя прочитанного файла
	errno_t err = _wfopen_s(&f, infilename, L"rb");

	if (!err && f) {
		while (!feof(f)) {
			fgetws(filename, PATHMAXSIZE, f);
			if (filename)
				filename[wcslen(filename) - 1] = L'\0';
			fwscanf_s(f, L"%u\n", &crc);
			ht_add(tbl, filename, crc);
			res += 1;
		}
		fclose(f);
	}
	return res;
}
