#include <stdio.h>
#include <windows.h>
#include "files.h"
#include "scui.h"

#define MAX_USERNAME_LENGTH 256
#define MAX_PATH_LENGTH 256
#define MAX_GETUSERNAME_ATTEMPTS 5

#define MINECRAFT_DIRECTORY_TEMPLATE "C:\\Users\\%s\\AppData\\Roaming\\.minecraft\\"
#define MODPACKS_DIRECTORY_TEMPLATE "C:\\Users\\%s\\AppData\\Roaming\\.minecraft\\mods\\mcmpm-modpacks\\"

#define C_LRED "\e[0;91m"
#define C_YELLOW "\e[0;33m"
#define C_RESET "\e[0m"

char MINECRAFT_DIRECTORY[MAX_PATH_LENGTH];
char MODPACKS_DIRECTORY[MAX_PATH_LENGTH];

void printHelpText(char* filename) {
	printf("here comes the help text, btw the filename is %s", filename); // TEMPORARY
}

/*
Initialization
Return codes:
-2 couldn't create vanilla.mp: fatal error
-1 no .minecraft folder: fatal error
0 mcmpm-modpacks and vanilla.mp found: success
1 mcmpm-modpacks and vanilla.mp created: success
*/
int init(char* username) {
	// set the constants
	// replace %s with <user> in path templates
	snprintf(MINECRAFT_DIRECTORY, MAX_PATH_LENGTH, MINECRAFT_DIRECTORY_TEMPLATE, username);
	snprintf(MODPACKS_DIRECTORY, MAX_PATH_LENGTH, MODPACKS_DIRECTORY_TEMPLATE, username);

	FILE* file;
	char path[MAX_PATH_LENGTH];
	snprintf(path, MAX_PATH_LENGTH, "%sclientId.txt", MINECRAFT_DIRECTORY);
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\clientId.txt

	file = fopen(path, "r");
	if (file == NULL) return -1; // if clientId.txt doesn't exist then .minecraft folder does neither
	fclose(file);

	snprintf(path, MAX_PATH_LENGTH, "%svanilla.mp", MODPACKS_DIRECTORY);
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\vanilla.mp

	file = fopen(path, "r");
	if (file != NULL) return 0; // if vanilla.mp modpack exists then modpacks folder does either
	fclose(file);
	
	int maxCmdLength = MAX_PATH_LENGTH + 8;
	char cmd[maxCmdLength];

	// create the mcmpm-modpacks folder
	snprintf(cmd, maxCmdLength, "mkdir \"%s\"", MODPACKS_DIRECTORY);
	system(cmd);

	// create the vanilla.mp modpack
	file = fopen(path, "w");
	if (file == NULL) return -2;
	fclose(file);

	return 1;
}

void freeStrArray(char** array, int n_items) {
	for (int i = 0; i < n_items; i++) {free(array[i]);}
}

int main(int argc, char* argv[])
{
	// INITIALISATION
	char* filename = argv[0];

	char username[MAX_USERNAME_LENGTH];
	unsigned long usernameLength;
	int n_attempts = 0;

	// damn GetUserName can return an empty username ("\0") and set usernameLength
	// to twice (assumption, i was getting 12 instead of 6) as real username length considering '\0'
	do { 
		GetUserName(username, &usernameLength); 
		n_attempts++;
		if (n_attempts > MAX_GETUSERNAME_ATTEMPTS) {
			printf("%sFatal error: Damn GetUserName() refuses to work normally, can't do anything about it.%s", C_LRED, C_RESET);
		}
	} while (usernameLength != strlen(username) + 1);

	char status; // not a character, just 1 byte
	status = init(username);
	
	if (status == -2) {
		printf("%sFatal error: Couldn't create vanilla.mp (default modpack).%s", C_LRED, C_RESET);
	} else if (status == -1) {	
		printf("%sFatal error: No .minecraft folder found.%s\n", C_LRED, C_RESET);
		return 0;
	} else if (status == 0) {
		printf("Found an existing configuration.\n");
	} else if (status == 1) {
		printf("\nConfiguration not found. Created a new one.\n");
	}


	// COMMAND HANDLING
	if (argc == 2) {
		char* command = argv[1];

		if (strcmp(command, "list") == 0) {
			int n_modpacks = getNFiles(MODPACKS_DIRECTORY, "mp");
			char* modpacks[n_modpacks];
			findFiles(MODPACKS_DIRECTORY, "mp", n_modpacks, modpacks);
			printf("Modpacks:\n");
			for (int i = 0; i < n_modpacks; i++) {
				char modpackName[strlen(modpacks[i])];
				strcpy(modpackName, modpacks[i]);
				modpackName[strlen(modpackName) - 3] = '\0'; // trim the .mp extension
				printf("    %s\n", modpackName);
			}
			freeStrArray(modpacks, n_modpacks); // freeing memory because it's good to free memory
			return 0;
		}

		// more command handlers here
	} else if (argc == 3) {
		char* command = argv[1];
		char* arg = argv[2];

		if (strcmp(command, "create") == 0) {
			char path[MAX_PATH_LENGTH];
			snprintf(path, MAX_PATH_LENGTH, "%s%s.mp", MODPACKS_DIRECTORY, arg);
			// the path should look like:
			// C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\<modpack name>.mp

			FILE* file;

			// check if modpack to be created already exists
			file = fopen(path, "r");
			if (file != NULL) {
				printf("%sFatal error: '%s' already exists!%s", C_LRED, arg, C_RESET);
				return 0;
			}
			fclose(file);

			file = fopen(path, "w");
			if (file == NULL) {
				printf("%sFatal error: couldn't create %s.mp%s", C_LRED, arg, C_RESET);
				return 0;
			}

			printf("Successfully created '%s'.", arg);

			return 0;

		} else if (strcmp(command, "delete") == 0) {
			char path[MAX_PATH_LENGTH];
			snprintf(path, MAX_PATH_LENGTH, "%s%s.mp", MODPACKS_DIRECTORY, arg);
			// the path should look like:
			// C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\<modpack name>.mp

			FILE* file;

			// check if modpack to be created already exists
			file = fopen(path, "r");
			if (file == NULL) {
				printf("%sFatal error: '%s' doesn't exist!%s", C_LRED, arg, C_RESET);
				return 0;
			}
			fclose(file);

			// not a character, but 1 byte
			char status = remove(path);
			if (status != 0) {
				printf("%sFatal error: couldn't delete '%s'%s", C_LRED, arg, C_RESET);
				return 0;
			}

			printf("Successfully deleted '%s'.", arg);

			return 0;
		}

		// more command handlers here
	}

	printf("\n"); // because why not

	// NOT FINISHED
	return 0;
}
