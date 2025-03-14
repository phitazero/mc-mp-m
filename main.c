#include <stdio.h>
#include <windows.h>
#include "files.h"
#include "scui.h"

#define MAX_USERNAME_LENGTH 256
#define MAX_PATH_LENGTH 256

#define MINECRAFT_FOLDER_PATH_TEMPLATE "C:\\Users\\%s\\AppData\\Roaming\\.minecraft\\"
#define MODPACKS_FOLDER_PATH_TEMPLATE "C:\\Users\\%s\\AppData\\Roaming\\.minecraft\\mods\\mcmpm-modpacks\\"

#define C_LRED "\e[0;91m"
#define C_YELLOW "\e[0;33m"
#define C_RESET "\e[0m"

char MINECRAFT_FOLDER_PATH[MAX_PATH_LENGTH];
char MODPACKS_FOLDER_PATH[MAX_PATH_LENGTH];

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
	// set the constants
	// replace %s with <user> in path templates
	snprintf(MINECRAFT_FOLDER_PATH, MAX_PATH_LENGTH, MINECRAFT_FOLDER_PATH_TEMPLATE, username);
	snprintf(MODPACKS_FOLDER_PATH, MAX_PATH_LENGTH, MODPACKS_FOLDER_PATH_TEMPLATE, username);

	FILE* file;
	char path[MAX_PATH_LENGTH];
	snprintf(path, MAX_PATH_LENGTH, "%sclientId.txt", MINECRAFT_FOLDER_PATH);
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\clientId.txt

	file = fopen(path, "r");
	if (file == NULL) return -1; // if clientId.txt doesn't exist then .minecraft folder does neither
	fclose(file);

	snprintf(path, MAX_PATH_LENGTH, "%svanilla.mp", MODPACKS_FOLDER_PATH);
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\vanilla.mp

	file = fopen(path, "r");
	if (file != NULL) return 0; // if vanilla.mp modpack exists then modpacks folder does either
	fclose(file);

	char folderPath[MAX_PATH_LENGTH];
	snprintf(folderPath, MAX_PATH_LENGTH, MODPACKS_FOLDER_PATH, username); // replace %s with <user> in path
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\

	int maxCmdLength = MAX_PATH_LENGTH + 13;
	char cmd[maxCmdLength];

	// create the mcmpm-modpacks folder
	snprintf(cmd, maxCmdLength, "mkdir \"%s\"", folderPath);
	system(cmd);

	// create the vanilla.mp modpack
	snprintf(cmd, maxCmdLength, "copy /Y NUL \"%s\"", path);
	system(cmd);

	return 1;
}

int main(int argc, char* argv[])
{
	char* filename = argv[0];

	char username[MAX_USERNAME_LENGTH];
	unsigned long usernameLength; // dummy, not used anywhere, required for GetUserName
	GetUserName(username, &usernameLength);

	// INITIALISATION
	char status; // not a character, just 1 byte
	status = init(username);
	if (status == -1) {	
		printf("%sFatal error: No .minecraft folder found!%s\n", C_LRED, C_RESET);
		return 0;
	} else if (status == 0) {
		printf("Found an existing configuration.\n");
	} else if (status == 1) {
		printf("\nConfiguration not found. Created a new one.\n");
	}

	if (argc == 2) {
		char* arg1 = argv[1];
		// command handlers here
	} else if (argc == 3) {
		char* arg1 = argv[1];
		char* arg2 = argv[2];
		// command handlers here
	}

	// NOT FINISHED
	return 0;
}
