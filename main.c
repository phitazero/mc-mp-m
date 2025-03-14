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

// status codes
#define SUCCESS 0
#define SUCCESS_ALT 1
#define ERR_OPERATION_FAIL -1
#define ERR_ALREADY_EXISTS -2
#define ERR_NOT_FOUND -3


char MINECRAFT_DIRECTORY[MAX_PATH_LENGTH];
char MODPACKS_DIRECTORY[MAX_PATH_LENGTH];

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
	if (file == NULL) return ERR_NOT_FOUND; // if clientId.txt doesn't exist then .minecraft folder does neither
	fclose(file);

	snprintf(path, MAX_PATH_LENGTH, "%svanilla.mp", MODPACKS_DIRECTORY);
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\vanilla.mp

	file = fopen(path, "r");
	if (file != NULL) return SUCCESS; // if vanilla.mp modpack exists then modpacks folder does either
	fclose(file);
	
	int maxCmdLength = MAX_PATH_LENGTH + 8;
	char cmd[maxCmdLength];

	// create the mcmpm-modpacks folder
	snprintf(cmd, maxCmdLength, "mkdir \"%s\"", MODPACKS_DIRECTORY);
	system(cmd);

	// create the vanilla.mp modpack
	file = fopen(path, "w");
	if (file == NULL) return ERR_OPERATION_FAIL;
	fclose(file);

	return SUCCESS_ALT;
}

void printHelpText(char* filename) {
	printf("here comes the help text, btw the filename is %s", filename); // TEMPORARY
}

void freeStrArray(char** array, int n_items) {
	for (int i = 0; i < n_items; i++) {free(array[i]);}
} 

void listModpacks() {
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
}

int createModpack(char* name) {
	char path[MAX_PATH_LENGTH];
	snprintf(path, MAX_PATH_LENGTH, "%s%s.mp", MODPACKS_DIRECTORY, name);
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\<modpack name>.mp

	FILE* file;

	// check if modpack to be created already exists
	file = fopen(path, "r");
	if (file != NULL) { return ERR_ALREADY_EXISTS; }
	fclose(file);

	file = fopen(path, "w");
	if (file == NULL) { return ERR_OPERATION_FAIL; } // couldn't create file

	return SUCCESS;
}

int deleteModpack(char* name) {
	char path[MAX_PATH_LENGTH];
	snprintf(path, MAX_PATH_LENGTH, "%s%s.mp", MODPACKS_DIRECTORY, name);
	// the path should look like:
	// C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\<modpack name>.mp

	FILE* file;

	// check if modpack exists
	file = fopen(path, "r");
	if (file == NULL) { return ERR_ALREADY_EXISTS; }
	fclose(file);

	int status = remove(path);
	if (status != 0) { return ERR_OPERATION_FAIL; } // if failed to remove

	return SUCCESS;
}


int main(int argc, char* argv[])
{
	// PRE INITIALISATION (some important constants)
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

	// INITIALISATION
	int status = init(username);
	if (status == ERR_OPERATION_FAIL) {
		printf("%sFatal error: Couldn't create vanilla.mp (default modpack).%s", C_LRED, C_RESET);
	} else if (status == ERR_NOT_FOUND) {	
		printf("%sFatal error: No .minecraft folder found.%s\n", C_LRED, C_RESET);
		return 0;
	} else if (status == SUCCESS) {
		printf("Found an existing configuration.\n");
	} else if (status == SUCCESS_ALT) {
		printf("\nConfiguration not found. Created a new one.\n");
	}

	puts(""); // new line because why not

	// COMMAND HANDLING
	if (argc == 2) {
		char* command = argv[1];

		if (strcmp(command, "list") == 0) { listModpacks(); }
		else if (strcmp(command, "help") == 0) { printHelpText(filename); }

	} else if (argc == 3) {
		char* command = argv[1];
		char* arg = argv[2];

		if (strcmp(command, "create") == 0) {
			int status = createModpack(arg);
			if (status == ERR_OPERATION_FAIL) { printf("%sFatal error: couldn't create %s.mp%s", C_LRED, arg, C_RESET); }
			else if (status == ERR_ALREADY_EXISTS) { printf("%sFatal error: '%s' already exists!%s", C_LRED, arg, C_RESET); }
			else if (status == SUCCESS) { printf("Successfully created '%s'.", arg); }

		} else if (strcmp(command, "delete") == 0) {
			int status = deleteModpack(arg);
			if (status == ERR_OPERATION_FAIL) { printf("%sFatal error: couldn't delete '%s'%s", C_LRED, arg, C_RESET); }
			else if (status == ERR_NOT_FOUND) { printf("%sFatal error: '%s' doesn't exist!%s", C_LRED, arg, C_RESET); }
			else if (status == SUCCESS) { printf("Successfully deleted '%s'.", arg); }
		}

		// more command handlers here
	}

	puts(""); // another new line because why not

	// NOT FINISHED
	return 0;
}
