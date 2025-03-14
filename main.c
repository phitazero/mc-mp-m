#include <stdio.h>
#include <windows.h>
#include "files.h"
#include "scui.h"

#define MAX_USERNAME_LENGTH 256
#define MAX_PATH_LENGTH 256

#define MINECRAFT_FOLDER_PATH "C:\\Users\\%s\\AppData\\Roaming\\.minecraft\\"
#define MODPACKS_FOLDER_PATH "C:\\Users\\%s\\AppData\\Roaming\\.minecraft\\mods\\mcmpm-modpacks\\"

#define C_RED "\e[0;31m"
#define C_RESET "\e[0m"

void printHelpText(char* filename) {
	printf("here comes the help text, btw the filename is %s", filename); // TEMPORARY
}

/*
Initialization
Return codes:
-1 no .minecraft folder: fatal error
0 mcmpm-modpacks and vanilla.mp found: success
1 mcmpm-modpacks and vanilla.mp created: success
*/
int init(char* username) {
	FILE* file;
	char path[MAX_PATH_LENGTH];
	char buffer[MAX_PATH_LENGTH];
	strcpy(path, MINECRAFT_FOLDER_PATH);
	snprintf(buffer, MAX_PATH_LENGTH, path, username); // replace %s with <user> in path
	snprintf(path, MAX_PATH_LENGTH, "%sclientId.txt", buffer);
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\clientId.txt

	file = fopen(path, "r");
	if (file == NULL) return -1; // if clientId.txt doesn't exist then .minecraft folder does neither
	fclose(file);

	strcpy(path, MODPACKS_FOLDER_PATH);
	snprintf(buffer, MAX_PATH_LENGTH, path, username); // replace %s with <user> in path
	snprintf(path, MAX_PATH_LENGTH, "%svanilla.mp", buffer);
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\vanilla.mp

	file = fopen(path, "r");
	if (file != NULL) return 0; // if vanilla.mp modpack exists then modpacks folder does either
	fclose(file);

	char folderPath[MAX_PATH_LENGTH];
	strcpy(buffer, MODPACKS_FOLDER_PATH);
	snprintf(folderPath, MAX_PATH_LENGTH, buffer, username); // replace %s with <user> in path
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\

	int maxCmdLength = MAX_PATH_LENGTH + 13;
	char cmd[maxCmdLength];

	// creating the mcmpm-modpacks folder
	snprintf(cmd, maxCmdLength, "mkdir \"%s\"", folderPath);
	system(cmd);

	// creating the vanilla.mp modpack
	snprintf(cmd, maxCmdLength, "copy /Y NUL \"%s\"", path);
	system(cmd);

	return 1;
}

int main(int argc, char* argv[])
{
	char* filename = argv[0];

	// display help if program runned without args
	if (argc == 1) {
		printHelpText(filename);
		return 0;
	}

	char username[MAX_USERNAME_LENGTH];
	unsigned long usernameLength; // dummy, not used anywhere, required for GetUserName
	GetUserName(username, &usernameLength);

	// INITIALISATION
	char status; // not a character, just 1 byte
	status = init(username);
	if (status == -1) {	
		printf("%sFatal error: No .minecraft folder found!%s\n", C_RED, C_RESET);
		return 0;
	} else if (status == 0) {
		printf("Found an existing configuration.\n");
	} else if (status == 1) {
		printf("\nConfiguration not found. Created a new one.\n");
	}

	// NOT FINISHED
	return 0;
}
