//////////////////////////////////////////////////////////////////////
//
//  dirview.c - функции поиска файлов по папкам
//
/////////////////////////////////////////////////////////////////////

#include "dirview.h"


int fname_to_str(char *fname)
{
	int res = 0;
	for (int i = 0; i < strlen(fname); i++) {
		if (*(fname + i) == '\\' || *(fname + i) == ':') {
			*(fname + i) = '_';
			res++;
		}
	}
	return res;
}

// Поиск файлов по папкам
bool ListDirectoryContents(const char *sDir, ht_item_t **tbl, HANDLE hProgressBar)
{
	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char sPath[PATHMAXSIZE];

	//Specify a file mask. *.* = We want everything!
	sprintf_s(sPath, PATHMAXSIZE, "%s\\*.*", sDir);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories.
		if (strcmp(fdFile.cFileName, ".") != 0
			&& strcmp(fdFile.cFileName, "..") != 0)
		{
			//Build up our file path using the passed in
			//  [sDir] and the file/foldername we just found:
			sprintf_s(sPath, PATHMAXSIZE, "%s\\%s", sDir, fdFile.cFileName);

			//Is the entity a File or Folder?
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				ListDirectoryContents(sPath, tbl, hProgressBar); 
			}
			else {
				ht_add(tbl, sPath, fgetcrc32(sPath));
				SendMessageW(hProgressBar, PBM_STEPIT, 0, 0);
			}
		}
	} while (FindNextFile(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return true;
}

//Количество файлов на диске
void recursiveFilesCount(const TCHAR Path[], int* nCount)
{
   	WIN32_FIND_DATA fdFile;
	HANDLE hFind = NULL;

	char sPath[PATHMAXSIZE];

	//Specify a file mask. *.* = We want everything!
	sprintf_s(sPath, PATHMAXSIZE, "%s\\*.*", Path);

	if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories.
		if (strcmp(fdFile.cFileName, ".") != 0
			&& strcmp(fdFile.cFileName, "..") != 0)
		{
			//Build up our file path using the passed in
			//  [sDir] and the file/foldername we just found:
			sprintf_s(sPath, PATHMAXSIZE, "%s\\%s", Path, fdFile.cFileName);

			//Is the entity a File or Folder?
			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				//ListDirectoryContents(sPath, tbl, hProgressBar); 
				recursiveFilesCount(sPath, nCount);
			}
			else {
				(*nCount)++;
			}
		}
	} while (FindNextFile(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return true;
}
