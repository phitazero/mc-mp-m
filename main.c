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
#define ERR_TMPFILE_FAIL -4
#define ERR_NO_FILES -5
#define ERR_ABORTED -6
#define ERR_EDITED_VANILLA -7


char MINECRAFT_DIRECTORY[MAX_PATH_LENGTH];
char MODPACKS_DIRECTORY[MAX_PATH_LENGTH];

// puts C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\<modpack name> in out
void putModIndexPath(char* out, char* name) {
	snprintf(out, MAX_PATH_LENGTH, "%s%s", MODPACKS_DIRECTORY, name);
}
// puts C:\Users\<user>\AppData\Roaming\.minecraft\mods\mcmpm-modpacks\<modpack name>.mp in out
// no i won't change every putModpackPath to putModIndexPath with adding .mp
void putModpackPath(char* out, char* name) {
	snprintf(out, MAX_PATH_LENGTH, "%s%s.mp", MODPACKS_DIRECTORY, name);
}

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

	if (!isfile(path)) return ERR_NOT_FOUND; // if clientId.txt doesn't exist then .minecraft folder does neither

	putModpackPath(path, "vanilla");

	if (isfile(path)) return SUCCESS; // if vanilla.mp modpack exists then modpacks folder does either

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

void printHelpText(char* exeName) {
	printf("%s help - get help\n", exeName);
	printf("%s list - show all existing modpacks\n", exeName);
	printf("%s create <modpack> - create a modpack with name <modpack>\n", exeName);
	printf("%s delete <modpack> - delete the modpack with name <modpack>\n", exeName);
	printf("%s list <modpack> - print all mods in <modpack>\n", exeName);
}

void freeStrArray(char** array, int n_items) {
	for (int i = 0; i < n_items; i++) {free(array[i]);}
} 

void listModpacks() {
	int n_modpacks = getNFiles(MODPACKS_DIRECTORY, "mp");
	char* modpacks[n_modpacks];
	findFiles(modpacks, "mp", n_modpacks, MODPACKS_DIRECTORY);
	printf("Modpacks:\n");
	for (int i = 0; i < n_modpacks; i++) {
		char name[strlen(modpacks[i])];
		strcpy(name, modpacks[i]);
		name[strlen(name) - 3] = '\0'; // trim the .mp extension
		printf("  - %s\n", name);
	}
	freeStrArray(modpacks, n_modpacks); // freeing memory because it's good to free memory
}

int createModpack(char* name) {
	char path[MAX_PATH_LENGTH];
	putModpackPath(path, name);

	FILE* file;

	// check if modpack to be created already exists
	file = fopen(path, "r");
	if (file != NULL) { return ERR_ALREADY_EXISTS; }
	fclose(file);

	file = fopen(path, "w");
	if (file == NULL) { return ERR_OPERATION_FAIL; } // couldn't create file

	return SUCCESS;
}

int listModpackMods(char* name) {
	char path[MAX_PATH_LENGTH];
	putModpackPath(path, name);

	FILE* file;

	file = fopenNoCR(path, "r");

	// if either modpack doesn't exist or failed to create tmpfile
	if (file == NULL) {
		if (!isfile(path)) return ERR_NOT_FOUND; // modpack doesn't exist

		// if the modpack exists, then tmpfile() failed
		return ERR_TMPFILE_FAIL;
	}

	int n_lines = getNLines(file);
	char* lines[n_lines];
	freadLines(lines, n_lines, file);

	if (n_lines > 1 || lines[0][0] != '\0') {
		printf("Mods in %s:\n", name);
		for (int i = 0; i < n_lines; i++) printf("  - %s\n", lines[i]);
	} else { printf("'%s' is empty.", name); }

	return SUCCESS;
}

int deleteModpack(char* name) {
	char path[MAX_PATH_LENGTH];
	putModpackPath(path, name);

	FILE* file;

	// check if modpack exists
	if (!isfile(path)) return ERR_NOT_FOUND;

	int status = remove(path);
	if (status != 0) return ERR_OPERATION_FAIL; // if failed to remove

	return SUCCESS;
}

int addMods(char* name, char* directory) {
	// you can't edit 'vanilla' modpack
	if (strcmp(name, "vanilla") == 0) return ERR_EDITED_VANILLA;

	// check if modpack exists
	char path[MAX_PATH_LENGTH];
	putModpackPath(path, name);
	if (!isfile(path)) return ERR_NOT_FOUND;

	int n_jars = getNFiles(directory, "jar");
	if (n_jars == 0) return ERR_NO_FILES;

	// options are all available .jars + "Finish" option
	int n_options = n_jars + 1;
	char* options[n_options];

	// adding the "Finish" option to the first position
	char finishOption[] = "Finish";
	options[0] = finishOption;

	// 0th index in options is reserved for "Finish", so start writing from the 1st index
	findFiles(options + 1, "jar", n_jars, directory);

	// initialize the array of chosen option with none chosen by default
	int selectedStatuses[n_options];
	memset(selectedStatuses, 0, n_options*sizeof(int));

	for (;;) {
		int choice = multichoice(n_options, options, selectedStatuses);

		// break if selectedStatuses "Finish"
		if (choice == 0) break;
		// or if pressed ESC
		if (choice == -1) return ERR_ABORTED;

		selectedStatuses[choice] = !selectedStatuses[choice]; // invert the current state
	}

	// count selectedStatuses options
	int n_selected = 0;
	for (int i = 0; i < n_options; i++) { n_selected += selectedStatuses[i]; };

	// array of selected options
	char* selected[n_selected];
	
	// add all selected options to selected[] and free() every unselected 
	int j = 0;
	for (int i = 0; i < n_options; i++) {
		if (selectedStatuses[i]) {
			selected[j] = options[i];
			j++;
		} else {
			// we don't want to free "Finish" allocated in stack
			if (i != 0) { free(options[i]); }
		}
	}

	// copy selected files to mcmpm directory
	for (int i = 0; i < n_selected; i++) {
		char srcPath[MAX_PATH_LENGTH];
		char dstPath[MAX_PATH_LENGTH];

		char* filename = selected[i];

		// set srcPath and dstPath
		snprintf(srcPath, MAX_PATH_LENGTH, "%s%s", directory, filename);
		putModIndexPath(dstPath, filename);

		int status = copyFile(srcPath, dstPath, 0);
		if (status == 0) return ERR_OPERATION_FAIL;
	}

	// in variable path we have path to corresponding .mp
	FILE* file = fopenNoCR(path, "r");
	if (file == NULL) return ERR_OPERATION_FAIL;
	
	int n_lines = getNLines(file);

	if (n_lines == 0) { // if is empty
		fclose(file);
		file = fopen(path, "w");
		if (file == NULL) return ERR_OPERATION_FAIL;

		// write only the selected and don't carea about former contents (empty)
		fwriteLines(selected, n_selected, file);
		fclose(file);
	} else {
		char* lines[n_lines + n_selected];
		// copy already existing entries 
		freadLines(lines, n_lines, file);
		// add the new ones after the old ones
		memcpy(lines + n_lines, selected, n_selected*sizeof(char*));

		fclose(file);
		file = fopen(path, "w");
		if (file == NULL) return ERR_OPERATION_FAIL;

		fwriteLines(lines, n_lines + n_selected, file);
		fclose(file);
	}
	
	puts(""); // new line because i want it to be here

	return SUCCESS;
}

void directoryFormat(char* directory) {
	if (strcmp(directory, "index") == 0) { // 'index' means the mcmpm directory
		strcpy(directory, MODPACKS_DIRECTORY);
	} else {
		int length = strlen(directory);
		if (directory[length - 1] != '/') { // if doesn't end with / add it
			directory[length] = '/';
			directory[length + 1] = '\0';
		}
	}
}

int main(int argc, char* argv[])
{
	// PRE INITIALISATION (some important constants)
	char* exeName = argv[0];

	char username[MAX_USERNAME_LENGTH];
	unsigned long usernameLength;
	int n_attempts = 0;

	// damn GetUserName can return an empty username ("\0") and set usernameLength
	// to twice (assumption, i was getting 12 instead of 6) as real username length (with '\0')
	do { 
		GetUserName(username, &usernameLength);
		n_attempts++;
		if (n_attempts > MAX_GETUSERNAME_ATTEMPTS) {
			printf(C_LRED"Fatal error: Damn GetUserName() refuses to work normally, can't do anything about it."C_RESET);
		}
	} while (usernameLength != strlen(username) + 1);

	// INITIALISATION
	int status = init(username);
	if (status == ERR_OPERATION_FAIL) {
		printf(C_LRED"Fatal error: Couldn't create vanilla.mp (default modpack).\n"C_RESET);
		return 0;
	} else if (status == ERR_NOT_FOUND) {	
		printf(C_LRED"Fatal error: No .minecraft folder found.\n"C_RESET);
		return 0;
	} else if (status == SUCCESS) {
		printf("Found an existing configuration.\n");
	} else if (status == SUCCESS_ALT) {
		printf("\nConfiguration not found. Created a new one.\n");
	}

	puts(""); // new line because why not

	// COMMAND HANDLING
	if (argc == 1) {
		printf("MineCraft ModPack Manager\nType '%s help' for help.", exeName);
	} else if (argc == 2) {
		char* command = argv[1];

		if (strcmp(command, "list") == 0) { listModpacks(); }
		else if (strcmp(command, "help") == 0) { printHelpText(exeName); }
		else { printf(C_LRED"Unknown command: '%s'. Type '%s help' for help."C_RESET, command, exeName); }

	} else if (argc == 3) {
		char* command = argv[1];
		char* arg = argv[2];

		if (strcmp(command, "create") == 0) {
			int status = createModpack(arg);
			if (status == ERR_OPERATION_FAIL) { printf(C_LRED"Fatal error: couldn't create '%s.mp'."C_RESET, arg); }
			else if (status == ERR_ALREADY_EXISTS) { printf(C_LRED"Fatal error: '%s' already exists!"C_RESET, arg); }
			else if (status == SUCCESS) { printf("Successfully created '%s'.", arg); }

		} else if (strcmp(command, "delete") == 0) {
			printf(C_LRED"Are you sure you want to delete '%s'? (y/n): ", arg);
			char choice = getch();
			printf("%c\n"C_RESET, choice);

			if (choice == 'y' || choice == 'Y') {
				int status = deleteModpack(arg);
				if (status == ERR_OPERATION_FAIL) { printf(C_LRED"Fatal error: couldn't delete '%s'."C_RESET, arg); }
				else if (status == ERR_NOT_FOUND) { printf(C_LRED"Fatal error: '%s' doesn't exist!"C_RESET, arg); }
				else if (status == SUCCESS) { printf("Successfully deleted '%s'.", arg); }
			}
		} else if (strcmp(command, "list") == 0) {
			int status = listModpackMods(arg);
			if (status == ERR_TMPFILE_FAIL) { printf(C_LRED"Fatal error: failed to create temporary file! Try again."C_RESET); }
			else if (status == ERR_NOT_FOUND) { printf(C_LRED"Fatal error: '%s' doesn't exist!"C_RESET, arg); }
		} else { printf(C_LRED"Unknown command: '%s'. Type '%s help' for help. "C_RESET, command, exeName); }
	} else if (argc == 4) {
		char* command = argv[1];
		char* arg1 = argv[2];
		char* arg2 = argv[3];

		if (strcmp(command, "add") == 0) {
			char directory[MAX_PATH_LENGTH];
			strcpy(directory, arg2);
			directoryFormat(directory);

			int status = addMods(arg1, directory);

			if (status == ERR_NO_FILES) { printf(C_LRED"Fatal error: No mods found in '%s'!"C_RESET, arg2); }
			else if (status == ERR_EDITED_VANILLA) { printf(C_LRED"Fatal error: you can't edit 'vanilla'!"C_RESET); }
			else if (status == ERR_NOT_FOUND) { printf(C_LRED"Fatal error: '%s' doesn't exist!"C_RESET, arg1); }
			else if (status == ERR_ABORTED) {
				printf(C_LRED"Fatal error: operation aborted.");
				printf("Don't press ESC in multichoice menus, only in case of a softlock."C_RESET);
			} else if (status == ERR_OPERATION_FAIL) { printf(C_LRED"Fatal error: Couldn't copy files or write to modpack!"C_RESET); }
			else if (status == SUCCESS) { printf("Successfully added mods to '%s'.", arg1); }

		} else { printf(C_LRED"Unknown command: '%s'. Type '%s help' for help. "C_RESET, command, exeName); }
	}

	puts(""); // another new line because why not

	return 0;
}
