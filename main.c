#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include "files.h"
#include "scui.h"

#define MAX_USERNAME_LENGTH 256
#define MAX_PATH_LENGTH 256

#define MINECRAFT_DIRECTORY_TEMPLATE "/home/%s/.minecraft/"
#define MODPACKS_DIRECTORY_TEMPLATE "/home/%s/.minecraft/mods/mcmpm-modpacks/"
#define MODS_DIRECTORY_TEMPLATE "/home/%s/.minecraft/mods/"

#define VERSION "2.0"

#define C_LRED "\e[0;91m"
#define C_YELLOW "\e[0;33m"
#define C_LGREEN "\e[0;92m"
#define C_LMAGENTA "\e[0;95m"
#define C_RESET "\e[0m"

// status codes
#define SUCCESS 0
#define SUCCESS_ALT 1
#define SUCCESS_WARN 2
#define ERR_OPERATION_FAIL -1
#define ERR_ALREADY_EXISTS -2
#define ERR_NOT_FOUND -3
#define ERR_NO_FILES -5
#define ERR_ABORTED -6
#define ERR_EDITED_VANILLA -7


char MINECRAFT_DIRECTORY[MAX_PATH_LENGTH];
char MODPACKS_DIRECTORY[MAX_PATH_LENGTH];
char MODS_DIRECTORY[MAX_PATH_LENGTH];

// puts /home/<user>/.minecraft/mods/mcmpm-modpacks/<modpack name> in out
void putModIndexPath(char* out, char* name) {
	snprintf(out, MAX_PATH_LENGTH, "%s%s", MODPACKS_DIRECTORY, name);
}
// puts /home/<user>/.minecraft/mods/mcmpm-modpacks/<modpack name>.mp in out
// no i won't change every putModpackPath to putModIndexPath with adding .mp
void putModpackPath(char* out, char* name) {
	snprintf(out, MAX_PATH_LENGTH, "%s%s.mp", MODPACKS_DIRECTORY, name);
}

int init(char* username) {
	// set the constants
	// replace %s with <user> in path templates
	snprintf(MINECRAFT_DIRECTORY, MAX_PATH_LENGTH, MINECRAFT_DIRECTORY_TEMPLATE, username);
	snprintf(MODPACKS_DIRECTORY, MAX_PATH_LENGTH, MODPACKS_DIRECTORY_TEMPLATE, username);
	snprintf(MODS_DIRECTORY, MAX_PATH_LENGTH, MODS_DIRECTORY_TEMPLATE, username);

	if (!isdirectory(MINECRAFT_DIRECTORY)) return ERR_NOT_FOUND; // if clientId.txt doesn't exist then .minecraft directory does neither

	FILE* file;
	char path[MAX_PATH_LENGTH];

	putModpackPath(path, "vanilla");

	if (isfile(path)) return SUCCESS; // if vanilla.mp modpack exists then modpacks directory does either

	int maxCmdLength = MAX_PATH_LENGTH + 8;
	char cmd[maxCmdLength];

	// create the mcmpm-modpacks directory
	snprintf(cmd, maxCmdLength, "mkdir \"%s\"", MODPACKS_DIRECTORY);
	system(cmd);

	// create the vanilla.mp modpack
	file = fopen(path, "w");
	if (file == NULL) return ERR_OPERATION_FAIL;
	fclose(file);

	return SUCCESS_ALT;
}

void printHelpText(char* exeName) {
	printf("You can use _ as modpack name to select modpack with GUI.\n\n");
	printf("%s help - get help\n", exeName);
	printf("%s list - show all existing modpacks\n", exeName);
	printf("%s create <modpack> - create a modpack with name <modpack>\n", exeName);
	printf("%s delete <modpack> - delete the modpack with name <modpack>\n", exeName);
	printf("%s list <modpack> - print all mods in <modpack>\n", exeName);
	printf("%s add <modpack> <directory> - add mods from <directory> to <modpack>.\n", exeName);
	printf("(Mods in index can be accessed by using <direcory> 'index'\n");
	printf("and currently active mods are accessed using 'mods')\n");
	printf("%s edit <modpack> - remove chosen mods from <modpack>\n", exeName);
	printf("%s path - print the path to configuration directory.\n", exeName);
	printf("%s current - show currently active mods.\n", exeName);
	printf("%s load <modpack> - load <modpack>.\n", exeName);
}

void freeStrArray(char** array, int n_items) {
	for (int i = 0; i < n_items; i++) {free(array[i]);}
}

void listModpacks() {
	int n_modpacks = getNFiles(MODPACKS_DIRECTORY, "mp");
	char* modpacks[n_modpacks];
	findFiles(modpacks, "mp", MODPACKS_DIRECTORY);
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

	file = fopen(path, "w");
	if (file == NULL) { return ERR_OPERATION_FAIL; } // couldn't create file

	return SUCCESS;
}

int listModpackMods(char* name) {
	char path[MAX_PATH_LENGTH];
	putModpackPath(path, name);

	FILE* file;

	file = fopen(path, "r");

	// if modpack doesn't exist
	if (file == NULL) return ERR_NOT_FOUND;

	int n_lines = getNLines(file);
	char* lines[n_lines];
	freadLines(lines, n_lines, file);

	if (n_lines > 0) {
		printf("Mods in %s:\n", name);
		for (int i = 0; i < n_lines; i++) printf(" - %s\n", lines[i]);
	} else { printf("'%s' is empty.", name); }

	return SUCCESS;
}

int deleteModpack(char* name) {
	// you can't delete 'vanilla' modpack
	if (strcmp(name, "vanilla") == 0) return ERR_EDITED_VANILLA;

	char path[MAX_PATH_LENGTH];
	putModpackPath(path, name);

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

	// options are all available .jars + "[Finish]" option
	int n_options = n_jars + 1;
	char* options[n_options];

	// adding the "[Finish]" option to the first position
	char finishOption[] = "[Finish]";
	options[0] = finishOption;

	// 0th index in options is reserved for "[Finish]", so start writing from the 1st index
	findFiles(options + 1, "jar", directory);

	// initialize the array of chosen option with none chosen by default
	int selectedStatuses[n_options];
	memset(selectedStatuses, 0, n_options*sizeof(int));

	for (;;) {
		int choice = multichoiceWStates(n_options, options, selectedStatuses);

		// break if selectedStatuses "[Finish]"
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
			// we don't want to free "[Finish]" allocated in stack
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

		// we don't want to copy files to themselves
		if (strcmp(srcPath, dstPath) == 0) continue;

		int status = copyFile(srcPath, dstPath);
		if (status != 0) return ERR_OPERATION_FAIL;
	}

	// in variable path we have path to corresponding .mp
	FILE* file = fopen(path, "r");
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

	if (n_selected == 0) return SUCCESS_ALT;
	return SUCCESS;
}

int editModpack(char* name) {
	// you can't edit 'vanilla' modpack
	if (strcmp(name, "vanilla") == 0) return ERR_EDITED_VANILLA;

	char path[MAX_PATH_LENGTH];
	putModpackPath(path, name);

	// check if modpack exists
	if (!isfile(path)) return ERR_NOT_FOUND;

	FILE* file = fopen(path, "r");

	int n_modpacks = getNLines(file);

	// options are all modpacks + "[Finish]" option
	int n_options = n_modpacks + 1;
	char* options[n_options];

	// adding the "[Finish]" option to the first position
	char finishOption[] = "[Finish]";
	options[0] = finishOption;

	// 0th index in options is reserved for "[Finish]", so start writing from the 1st index
	freadLines(options + 1, n_modpacks, file);
	fclose(file);

	// initialize the array of chosen options with every options (except "[Finish]") chosen by default
	int selectedStatuses[n_options];
	selectedStatuses[0] = 0;
	for (int i = 1; i < n_options; i++) { selectedStatuses[i] = 1; }

	for (;;) {
		int choice = multichoiceWStates(n_options, options, selectedStatuses);

		// break if selectedStatuses "[Finish]"
		if (choice == 0) break;
		// or if pressed ESC
		if (choice == -1) return ERR_ABORTED;

		selectedStatuses[choice] = !selectedStatuses[choice]; // invert the current state
	}	

	// set every unselected option to empty string, so they will be ignored by fwriteLines
	// we can do it easily by setting first byte to \0
	for (int i = 0; i < n_options; i++) {
		if (!selectedStatuses[i]) { options[i][0] = '\0'; }
	}

	// calculate amount of mods removed
	int n_removed = n_options - 1; // don't count "[Finish]" option
	for (int i = 0; i < n_options; i++) { n_removed -= selectedStatuses[i]; }

	file = fopen(path, "w");
	if (file == NULL) return ERR_OPERATION_FAIL;
	fwriteLines(options, n_options, file);
	fclose(file);

	// start with 1 because we don't want to free "[Finish]"
	for (int i = 1; i < n_options; i++) { free(options[i]); }

	puts(""); // new line because why not

	if (n_removed == 0) return SUCCESS_ALT;
	return SUCCESS;
}

void directoryFormat(char* directory) {
	if (strcmp(directory, "index") == 0) { // "index" means the mcmpm directory
		strcpy(directory, MODPACKS_DIRECTORY);
	} else if (strcmp(directory, "mods") == 0) { // "mods" is the default minecraft mods directory
		strcpy(directory, MODS_DIRECTORY);
	}
	else {
		int length = strlen(directory);
		if (directory[length - 1] != '/') { // if doesn't end with / add it
			directory[length] = '/';
			directory[length + 1] = '\0';
		}
	}
}

// compares arrays of strings (order doesn't matter)
int strarrcmp(char** arr1, char** arr2, int length) {
	// count of mathing strings
	int n_matches = 0;

	for (int i = 0; i < length; i++) {
		for (int j = 0; j < length; j++) {
			n_matches += ( strcmp(arr1[i], arr2[j]) == 0 );
		}
	}
	return 1 - n_matches == length; // we want 0 to mean equality, as with strcmp
}

void printCurrentModpack(char* mods[], int n_mods) {
	int n_modpacks = getNFiles(MODPACKS_DIRECTORY, "mp");
	char* modpacks[n_modpacks];
	findFiles(modpacks, "mp", MODPACKS_DIRECTORY);

	for (int i = 0; i < n_modpacks; i++) {
		char* modpack = modpacks[i];

		char path[MAX_PATH_LENGTH];
		snprintf(path, MAX_PATH_LENGTH, "%s%s", MODPACKS_DIRECTORY, modpack);
		FILE* file = fopen(path, "r");

		// mpmod - a mod saved in the modpack
		// mod - a mod in mods directory
		int n_mpmods = getNLines(file);

		if (n_mpmods != n_mods) continue;

		char* mpmods[n_mpmods];
		freadLines(mpmods, n_mpmods, file);

		if (strarrcmp(mods, mpmods, n_mods) == 0) {
			modpack[strlen(modpack) - 3] = '\0'; // remove .mp at the end
			printf("Currently loaded modpack is '%s'.\n", modpack);
			return;	
		}
	}
	puts(C_LRED"None of existing modpacks matches currently loaded mods."C_RESET);
}

void listCurrentMods() {
	int n_mods = getNFiles(MODS_DIRECTORY, "jar");

	if (n_mods == 0) {
		printf("No mods found.\n\nCurrently loaded modpack is 'vanilla'.");
		return;
	}

	char* mods[n_mods];
	findFiles(mods, "jar", MODS_DIRECTORY);

	puts("Currently active mods:");
	for (int i = 0; i < n_mods; i++) { printf("   - %s\n", mods[i]); }

	puts("");

	printCurrentModpack(mods, n_mods);
}

void deleteCurrentMods() {
	int n_mods = getNFiles(MODS_DIRECTORY, "jar");

	char* mods[n_mods];
	findFiles(mods, "jar", MODS_DIRECTORY);

	for (int i = 0; i < n_mods; i++) {
		char* mod = mods[i];
		// get mod path
		char path[MAX_PATH_LENGTH];
		snprintf(path, MAX_PATH_LENGTH, "%s%s", MODS_DIRECTORY, mod);

		// delete mod and free() string
		remove(path);
		free(mod);
	}
}

int selectModpack(char* out) {
	int n_modpacks = getNFiles(MODPACKS_DIRECTORY, "mp");

	// options are all available modpacks + "[Cancel]" option
	int n_options = n_modpacks + 1;
	char* options[n_options];

	char cancelOption[] = "[Cancel]";
	options[0] = cancelOption;

	// leaving 0th place for "[Cancel]" option
	findFiles(options + 1, "mp", MODPACKS_DIRECTORY);

	// remove .mp extension at the end for all modpacks
	for (int i = 1; i < n_options; i++) {
		int length = strlen(options[i]);
		options[i][length - 3] = '\0';
	}

	// swap 1st option with 'vanilla', i. e. put 'vanilla' modpack right below "[Cancel]"
	int vanillaIndex;
	for (int i = 1; i < n_options; i++) {
		if (strcmp(options[i], "vanilla") == 0) {
			vanillaIndex = i;
			break;
		}
	}
	char* buffer = options[1]; // put the modpack to be swapped out into buffer
	options[1] = options[vanillaIndex];
	options[vanillaIndex] = buffer;

	int selected = multichoice(n_options, options);

	if (selected == 0) {
		out[0] = '\0';
		return SUCCESS;
	} else if (selected == -1) {
		return ERR_ABORTED;
	} else {
		strcpy(out, options[selected]);
		return SUCCESS;
	}
}

int loadModpack(char* name) {
	// get modpack path
	char path[MAX_PATH_LENGTH];
	putModpackPath(path, name);

	FILE* file;

	file = fopen(path, "r");

	// if modpack doesn't exist
	if (file == NULL) return ERR_NOT_FOUND;

	int n_lines = getNLines(file);

	char* lines[n_lines];
	freadLines(lines, n_lines, file);

	// check if all listed mods exist
	int allExist = 1; // by default they do
	for (int i = 0; i < n_lines; i++) {
		char* mod = lines[i];
		char modPath[MAX_PATH_LENGTH];
		snprintf(modPath, MAX_PATH_LENGTH, "%s%s", MODPACKS_DIRECTORY, mod);
		if (!isfile(modPath)) {
			allExist = 0;
			printf(C_YELLOW"Not found: %s\n"C_RESET, mod);
		}
	}

	if (!allExist) {
		printf(C_YELLOW"Not all listed mods are found. Are you sure you want to proceed? (y/n) ");
		char choice = getch();
		printf("%c\n"C_RESET, choice);

		if (!(choice == 'y' || choice == 'Y')) return ERR_ABORTED;
	}

	deleteCurrentMods();

	// copy mods
	for (int i = 0; i < n_lines; i++) {
		char* mod = lines[i];
		
		char srcPath[MAX_PATH_LENGTH];
		char dstPath[MAX_PATH_LENGTH];

		putModIndexPath(srcPath, mod);
		snprintf(dstPath, MAX_PATH_LENGTH, "%s%s", MODS_DIRECTORY, mod);

		int status = copyFile(srcPath, dstPath);
		if (status != 0) return ERR_OPERATION_FAIL;
	}

	if (allExist) return SUCCESS;
	return SUCCESS_WARN;
}

int main(int argc, char* argv[])
{
	// PRE INITIALISATION (some important constants)
	char* exeName = argv[0];

	char username[MAX_USERNAME_LENGTH];
	
	strcpy(username, getpwuid(getuid())->pw_name);

	// INITIALISATION
	int status = init(username);
	if (status == ERR_OPERATION_FAIL) {
		printf(C_LRED"Fatal error: Couldn't create vanilla.mp (default modpack).\n"C_RESET);
		return 0;
	} else if (status == ERR_NOT_FOUND) {	
		printf(C_LRED"Fatal error: No .minecraft directory found.\n"C_RESET);
		return 0;
	} else if (status == SUCCESS) {
		printf("Found an existing configuration.\n");
	} else if (status == SUCCESS_ALT) {
		printf("\nConfiguration not found. Created a new one.\n");
	}

	// if first arg is _ we replace it with modpack, selected with GUI
	if (argc > 2 && strcmp(argv[2], "_") == 0) {
		char* modpack = malloc(MAX_PATH_LENGTH);

		int status = selectModpack(modpack);
		if (status == ERR_ABORTED) {
			printf(C_LRED"User aborted."C_RESET);
			return 0;
		} else if (modpack[0] == '\0') return 0;

		argv[2] = modpack;
	}

	puts(""); // new line because why not

	// COMMAND HANDLING
	if (argc == 1) {
		puts(C_LMAGENTA"+=============================+");
		puts("|  MineCraft ModPack Manager  |");
		puts("+=============================+"C_RESET);
		puts("v"VERSION", by Phi\n");
		printf("Type '%s help' for help.", exeName);

	} else if (argc == 2) {
		char* command = argv[1];

		if (strcmp(command, "list") == 0) {
			listModpacks();
		} else if (strcmp(command, "help") == 0) {
			printHelpText(exeName);
		} else if (strcmp(command, "path") == 0) {
			puts(MODPACKS_DIRECTORY);
		} else if (strcmp(command, "current") == 0) {
			listCurrentMods();
		} else {
			printf(C_LRED"Unknown command or syntax: '%s'. Type '%s help' for help."C_RESET, command, exeName);
		}

	} else if (argc == 3) {
		char* command = argv[1];
		char* arg = argv[2];

		if (strcmp(command, "create") == 0) {
			int status = createModpack(arg);
			if (status == ERR_OPERATION_FAIL) {
				printf(C_LRED"Fatal error: couldn't create '%s.mp'."C_RESET, arg);
			} else if (status == ERR_ALREADY_EXISTS) {
				printf(C_LRED"Fatal error: '%s' already exists!"C_RESET, arg);
			} else if (status == SUCCESS) {
				printf(C_LGREEN"Successfully created '%s'."C_RESET, arg);
			}

		} else if (strcmp(command, "delete") == 0) {
			printf(C_LRED"Are you sure you want to delete '%s'? (y/n): ", arg);
			char choice = getch();
			printf("%c\n"C_RESET, choice);

			if (choice == 'y' || choice == 'Y') {
				int status = deleteModpack(arg);
				if (status == ERR_OPERATION_FAIL) {
					printf(C_LRED"Fatal error: couldn't delete '%s'."C_RESET, arg);
				} else if (status == ERR_EDITED_VANILLA) {
					printf(C_LRED"Fatal error: you can't delete 'vanilla'!"C_RESET);
				} else if (status == ERR_NOT_FOUND) {
					printf(C_LRED"Fatal error: '%s' doesn't exist!"C_RESET, arg);
				} else if (status == SUCCESS) {
					printf(C_LGREEN"Successfully deleted '%s'."C_RESET, arg);
				}

			}
		} else if (strcmp(command, "list") == 0) {
			int status = listModpackMods(arg);
			if (status == ERR_NOT_FOUND) {
				printf(C_LRED"Fatal error: '%s' doesn't exist!"C_RESET, arg);
			}

		} else if (strcmp(command, "edit") == 0) {
			int status = editModpack(arg);
			if (status == ERR_EDITED_VANILLA) {
				printf(C_LRED"Fatal error: you can't edit 'vanilla'!"C_RESET);
			} else if (status == ERR_NOT_FOUND) {
				printf(C_LRED"Fatal error: '%s' doesn't exist!"C_RESET, arg);
			} else if (status == ERR_OPERATION_FAIL) {
				printf(C_LRED"Fatal error: Couldn't write to modpack!"C_RESET);
			} else if (status == SUCCESS) {
				printf(C_LGREEN"Successfully edited '%s'."C_RESET, arg);
			} else if (status == SUCCESS_ALT) {
				printf("Nothing changed.");
			}

		} else if (strcmp(command, "load") == 0) {
			int status = loadModpack(arg);
			if (status == ERR_OPERATION_FAIL) {
				printf(C_LRED"Fatal error: Failed to copy some files!"C_RESET);
			} else if (status == ERR_ABORTED) {
				printf(C_LRED"User aborted."C_RESET);
			} else if (status == SUCCESS) {
				printf(C_LGREEN"Successfully loaded '%s'"C_RESET, arg);
			} else if (status == ERR_NOT_FOUND) {
				printf(C_LRED"Fatal error: '%s' doesn't exist!"C_RESET, arg);
			} else if (status == SUCCESS_WARN) {
				printf(C_YELLOW"Loaded '%s' with some mods missing.", arg);
			}

		} else {
			printf(C_LRED"Unknown command or syntax: '%s'. Type '%s help' for help. "C_RESET, command, exeName);
		}

	} else if (argc == 4) {
		char* command = argv[1];
		char* arg1 = argv[2];
		char* arg2 = argv[3];

		if (strcmp(command, "add") == 0) {
			char directory[MAX_PATH_LENGTH];
			strcpy(directory, arg2);
			directoryFormat(directory);

			int status = addMods(arg1, directory);

			if (status == ERR_NO_FILES) {
				printf(C_LRED"Fatal error: No mods found in '%s'!"C_RESET, arg2);
			} else if (status == ERR_EDITED_VANILLA) {
				printf(C_LRED"Fatal error: you can't edit 'vanilla'!"C_RESET);
			} else if (status == ERR_NOT_FOUND) {
				printf(C_LRED"Fatal error: '%s' doesn't exist!"C_RESET, arg1);
			}  else if (status == ERR_OPERATION_FAIL) {
				printf(C_LRED"Fatal error: Couldn't copy files or write to modpack!"C_RESET);
			} else if (status == SUCCESS) {
				printf(C_LGREEN"Successfully added mods to '%s'."C_RESET, arg1);
			} else if (status == SUCCESS_ALT) {
				printf("Nothing changed.");
			}

		} else {
			printf(C_LRED"Unknown command or syntax: '%s'. Type '%s help' for help. "C_RESET, command, exeName);
		}
	}

	puts(""); // another new line because why not

	return 0;
}
