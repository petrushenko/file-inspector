//////////////////////////////////////////////////////////////////////
//
//  filehash.c - реализация функций для работы с файлами
//
/////////////////////////////////////////////////////////////////////

#include "filehash.h"
#include "dirview.h"

/*
Получение контрольной суммы из файла
Вычисляется CRC32 с полиномом 0x8005
См. описание Crc32()
*/
uint_least32_t fgetcrc32(char *filename)
{	
	FILE *f;
	uint_least32_t crc = 0xFFFFFFFFUL;
	errno_t err;
	err = fopen_s(&f, filename, "rb");
	if (err == 0 && f != NULL) {	
		while (!feof(f)) {
			int result = fread(buffer, 1, BUFFSIZE, f);
			if (result) {
				crc = Crc32(buffer, result, crc);
			}
		}
		crc = Crc32(filename, strlen(filename), crc);
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
int fhash_save(struct ht_item_t **tbl, const char *outfilename) {
	int res = 0;
	FILE *f;
	errno_t err = fopen_s(&f, outfilename, "w");
	ht_item_t *tmp_ptr;

	if (err == 0 && f != NULL) {
		for (uint32_t i = 0; i < HASHTABLE_MIN_SIZE; i++) {
			tmp_ptr = *(tbl + i);
			while (tmp_ptr) {
				//fputs(tmp_ptr->fname, f);
				//fprintf_s(f, "\n%u\n", tmp_ptr->crc32);
				fprintf(f, "%s\n%u\n", (tmp_ptr->fname), (tmp_ptr->crc32));
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
int fhash_load(struct ht_item_t **tbl, const char *infilename) {

	int res = 0;
	FILE *f;
	uint32_t crc = 0; //crc32 прочитанного файла
	char filename[PATHMAXSIZE]; //имя прочитанного файла
	errno_t err = fopen_s(&f, infilename, "r");
	char symb;

	if (!err && f) {
		while (!feof(f)) {
			fgets(filename, PATHMAXSIZE, f);
			if (filename)
				filename[strlen(filename) - 1] = '\0';
			fscanf_s(f, "%u\n", &crc);
			//fscanf_s(f, "%s\n%u\n", filename, PATHMAXSIZE, &crc);
			ht_add(tbl, filename, crc);
			res += 1;
		}
		fclose(f);
	}
	return res;
}
