// to work with files (specifically .jar's)

#include <windows.h>
#include <string.h>

#define MAX_PATH_LENGTH 256

// bring everything to camelCase
#define copyFile CopyFile

// get number of .<ext> files in a directory
int getNFiles(char* directory, char ext[]) {
	WIN32_FIND_DATA findFileData;

	char path[MAX_PATH_LENGTH];
	snprintf(path, MAX_PATH_LENGTH, "%s*.%s", directory, ext);

	HANDLE handleFind = FindFirstFile(path, &findFileData);
	if (handleFind == INVALID_HANDLE_VALUE) return 0;

	// if we've got to this point we already have a file, found by FindFirstFile
	int n_files = 1;

	while (FindNextFile(handleFind, &findFileData) != 0) n_files++;

	return n_files;
}

// puts the pointer to the array of pointers to malloc()'ed filenames of .<ext> files into out_files
int findFiles(char* directory, char ext[], int n_files, char** out_files) {
	WIN32_FIND_DATA findFileData;

	char path[MAX_PATH_LENGTH];
	snprintf(path, MAX_PATH_LENGTH, "%s*.%s", directory, ext);

	HANDLE handleFind = FindFirstFile(path, &findFileData);
	if (handleFind == INVALID_HANDLE_VALUE) return -1;

	for (int i = 0; i < n_files; i++) {
		char* filename = (char*)malloc(strlen(findFileData.cFileName) + 1);
		strcpy(filename, findFileData.cFileName);
		out_files[i] = filename;
		FindNextFile(handleFind, &findFileData);
	}
	
	return 0;
}
