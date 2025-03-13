#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C_RESET "\e[0m"
#define C_SELECTED "\e[30;47m"

#define KEY_ENTER 13
#define KEY_ESC 27

// n_sth - number of sth, e.g. n_options - number of options 

int multichoice(int n_options, char* options[], char* postText) {
	if (n_options < 1) return -1;

	int current = 0; // index of currently selected option
	for (;;) {
		system("cls");

		for (int i = 0; i < n_options; i++) {
			char color[9];
			if (i == current) strcpy(color, C_SELECTED);
			else strcpy(color, C_RESET);
			printf("[%d] %s%s\n", i, color, options[i]);
			printf(C_RESET);
		}

		if (postText) printf("\n%s\n", postText);

		unsigned char input = getch();
		if (input == KEY_ENTER) return current;
		else if (input == KEY_ESC) return -1; // used only in case of a softlock
		else if (input == 224) { // KEY_UP and KEY_DOWN are 2 bytes wide, so we need a second getch()
			unsigned char input2 = getch();
			// KEY_UP
			if (input2 == 80 && current < n_options - 1) current++;
			// KEY_DOWN
			else if (input2 == 72 && current > 0) current--;
		}
		else if ('0' <= input && input <= '9') {
			int index = input - '0'; // convert ['0'-'9'] to [0-9]
			if (index < n_options) current = index;
		}
	}
}