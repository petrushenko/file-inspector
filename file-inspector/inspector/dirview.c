//////////////////////////////////////////////////////////////////////
//
//  dirview.c - функции поиска файлов по папкам
//
/////////////////////////////////////////////////////////////////////

#include "dirview.h"

int fname_to_str(WCHAR *fname)
{
	int res = 0;
	for (int i = 0; i < wcslen(fname); i = i + 1) {
		if (*(fname + i) == L'\\' || *(fname + i) == L':') {
			*(fname + i) = L'\0';			
			res++;
			break;
		}
	}
	return res;
}

// Поиск файлов по папкам
bool ListDirectoryContents(const WCHAR *sDir, ht_item_t **tbl, HANDLE hProgressBar)
{
	WIN32_FIND_DATAW fdFile;
	HANDLE hFind = NULL;

	WCHAR sPath[PATHMAXSIZE];

	//Specify a file mask. *.* = We want everything!
	swprintf_s(sPath, PATHMAXSIZE, L"%ls\\*.*", sDir);

	if ((hFind = FindFirstFileW(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	do
	{
		//Find first file will always return "."
		//    and ".." as the first two directories.
		if (wcscmp(fdFile.cFileName, L".") != 0
			&& wcscmp(fdFile.cFileName, L"..") != 0)
		{
			//Build up our file path using the passed in
			//  [sDir] and the file/foldername we just found:
			swprintf_s(sPath, PATHMAXSIZE, L"%ls\\%ls", sDir, fdFile.cFileName);

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
	} while (FindNextFileW(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return true;
}

//Количество файлов на диске
void recursiveFilesCount(const WCHAR Path[], int* nCount)
{
	WIN32_FIND_DATAW fdFile;
	HANDLE hFind = NULL;

	WCHAR sPath[PATHMAXSIZE];
	swprintf_s(sPath, PATHMAXSIZE, L"%ls\\*.*", Path);

	if ((hFind = FindFirstFileW(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	do
	{
		if (wcscmp(fdFile.cFileName, L".") != 0
			&& wcscmp(fdFile.cFileName, L"..") != 0)
		{
			swprintf_s(sPath, PATHMAXSIZE, L"%ls\\%ls", Path, fdFile.cFileName);

			if (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				recursiveFilesCount(sPath, nCount);
			}
			else {
				(*nCount)++;
			}
		}
	} while (FindNextFileW(hFind, &fdFile)); //Find the next file.

	FindClose(hFind); //Always, Always, clean things up!

	return true;
}
