// to work with files

#include <windows.h>
#include <string.h>
#include <io.h>

#define MAX_PATH_LENGTH 256

// bring everything to camelCase
#define copyFile CopyFile

int isfile(char* path) {
	FILE* file;
	file = fopen(path, "r");

	// couldn't open file - probably doesn't exist
	if (file == NULL) return 0;

	// if we got to this point - file exists
	fclose(file);
	return 1;
}

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
int findFiles(char** out_files, char ext[], int n_files, char* directory) {
	WIN32_FIND_DATA findFileData;

	char path[MAX_PATH_LENGTH];
	snprintf(path, MAX_PATH_LENGTH, "%s*.%s", directory, ext);

	HANDLE handleFind = FindFirstFile(path, &findFileData);
	if (handleFind == INVALID_HANDLE_VALUE) return -1;

	for (int i = 0; i < n_files; i++) {
		char* filename = (char*) malloc(strlen(findFileData.cFileName) + 1);
		strcpy(filename, findFileData.cFileName);
		out_files[i] = filename;

		FindNextFile(handleFind, &findFileData);
	}
	
	return 0;
}

// opens file ignoring '\r's
FILE* fopenNoCR(char* filename, char* mode) {
	if (!mode || mode[0] != 'r') {
		return fopen(filename, mode);  // we care only about reading
	}

	FILE* originalFile = fopen(filename, "rb");  // open in binary to see '\r'
	if (!originalFile) return NULL;

	FILE* tempFile = tmpfile();
	if (!tempFile) {
		fclose(originalFile);
		return NULL;
	}

	// copy ignoring '\r'
	char c;
	while ((c = fgetc(originalFile)) != EOF) {
		if (c != '\r') fputc(c, tempFile);
	}

	fclose(originalFile);
	rewind(tempFile); // prepare for further usage

	return tempFile;
}


int getFileLength(FILE* file) {
	fseek(file, 0, SEEK_END);
	int length = ftell(file);
	rewind(file);
	return length;
}

int getNLines(FILE* file) {
	int fileLength = getFileLength(file);
	if (fileLength == 0) return 0;

	char buffer[fileLength];
	fread(buffer, 1, fileLength, file);
	rewind(file);

	int n_lines = 1;
	for (int i = 0; i < fileLength; i++) n_lines += (buffer[i] == '\n');

	return n_lines;
}

void freadLines(char** out, int n_lines, FILE* file) {
	int fileLength = getFileLength(file);

	char buffer[fileLength];
	fread(buffer, 1, fileLength, file);
	rewind(file);

	char* p = buffer;
	for (int i = 0; i < n_lines; i++) {
		int lineLength;
		for (lineLength = 0; !(p[lineLength] == '\n' || (int)(p - buffer) + lineLength == fileLength); lineLength++);
		lineLength++; // the for loop doesn't consider the '\n' (or EOF) at the end
					// manually add a byte for '\0'

		char* line = malloc(lineLength);
		memcpy(line, p, lineLength);
		line[lineLength - 1] = '\0'; // replace '\n' or EOF with '\0'
		out[i] = line;

		p += lineLength; // move on to the next line
	}
}

void fwriteLines(char** lines, int n_lines, FILE* file) {
	for (int i = 0; i < n_lines; i++) {
		char* line = lines[i];

		// considering the '\0'
		int lineLength = strlen(line) + 1;

		if (lineLength > 1) { // if it's not an empty line
			for (int j = 0; j < lineLength; j++) {
				char c = line[j];
				if (c != '\0') { fputc(c, file); } // if it's not '\0'
				else {
					// if it's not the last line
					if (i != n_lines - 1) fputc('\n', file);
					// leave the last line without '\n'
				}
			}
		} else if (i == n_lines - 1) { // if it's the last line, it's empty, we left a \n at the end
			fflush(file);
			int fd = _fileno(file);
			// After fputc the cursor is after the last byte, (considering indexing differences) it's equal
			// to current size of file. Subtract 2 to remove \n and \r which windows (hate it) adds
			int newFileSize = ftell(file) - 2;
			int status = _chsize(fd, newFileSize);
			printf("%d\n", status);
		}
	}
}
