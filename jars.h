// to work with files (specifically .jar's)

#include <windows.h>
#include <string.h>

#define MAX_PATH_LENGTH 256

// bring everything to camelCase
#define copyFile CopyFile

// get number of .jar's in a directory
int getNJars(const char* directory) {
	WIN32_FIND_DATA findFileData;

	char path[MAX_PATH_LENGTH];
	snprintf(path, MAX_PATH_LENGTH, "%s\\*.jar", directory);

	HANDLE handleFind = FindFirstFile(path, &findFileData);
	if (handleFind == INVALID_HANDLE_VALUE) return 0;

	// if we've got to this point we already have a file, found by FindFirstFile
	int n_jars = 1;

	while (FindNextFile(handleFind, &findFileData) != 0) n_jars++;

	return n_jars;
}

// puts the pointer to the array of pointers to malloc()'ed into out_jars
// must be NECESSARILY free()'d
int findJars(const char* directory, char** out_jars) {
	WIN32_FIND_DATA findFileData;

	int n_jars = getNJars(directory);
	if (n_jars == 0) return -1;

	char path[MAX_PATH_LENGTH];
	snprintf(path, MAX_PATH_LENGTH, "%s\\*.jar", directory);

	HANDLE handleFind = FindFirstFile(path, &findFileData);
	if (handleFind == INVALID_HANDLE_VALUE) return -1;

	for (int i = 0; i < n_jars; i++) {
		char* filename = (char*)malloc(strlen(findFileData.cFileName) + 1);
		strcpy(filename, findFileData.cFileName);
		out_jars[i] = filename;
		FindNextFile(handleFind, &findFileData);
	}
	
	return 0;
}
