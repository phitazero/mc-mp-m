// to work with files

#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX_PATH_LENGTH 256
#define MAX_PATTERN_LENGTH 8

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
	DIR *dir;
	struct dirent *entry;

	dir = opendir(directory);
	if (dir == NULL) {
		return 0;
	}

	char pattern[MAX_PATTERN_LENGTH];
	snprintf(pattern, MAX_PATTERN_LENGTH, "*.%s", ext);

	int n_files = 0;

	while ((entry = readdir(dir)) != NULL) {
		if (fnmatch(pattern, entry->d_name, 0) == 0)
			n_files++;
	}

	closedir(dir);

	return n_files;
}

// puts the pointer to the array of pointers to malloc()'ed filenames of .<ext> files into out_files
int findFiles(char** out_files, char ext[], char* directory) {
	DIR *dir;
	struct dirent *entry;

	dir = opendir(directory);
	if (dir == NULL) {
		return -1;
	}

	char pattern[MAX_PATTERN_LENGTH];
	snprintf(pattern, MAX_PATTERN_LENGTH, "*.%s", ext);
	
	int n_foundFiles = 0;

	while ((entry = readdir(dir)) != NULL) {
		if (fnmatch(pattern, entry->d_name, 0) != 0) continue;
		char* filename = (char*) malloc(strlen(entry->d_name) + 1);
		strcpy(filename, entry->d_name);
		out_files[n_foundFiles] = filename;
		
		n_foundFiles++;
	}

	closedir(dir);

	return 0;
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
			int fd = fileno(file);
			// On Linux, newline is just '\n', so subtract only 1
			int newFileSize = ftell(file) - 1;
			ftruncate(fd, newFileSize);
		}
	}
}
