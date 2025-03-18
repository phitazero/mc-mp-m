#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define C_RESET "\e[0m"
#define C_CURRENT "\e[30;47m"
#define C_UNSELECTED "\e[0;90m"

#define KEY_ENTER 13
#define KEY_ESC 27

// selected[i] is a bool (formally and int) indicating whether options[i] was selected
int multichoice(int n_options, char* options[], int* selected) {
	if (n_options < 1) return -1;

	static int current = 0; // index of currently selected option
	for (;;) {
		system("cls");

		for (int i = 0; i < n_options; i++) {
			char indexColor[9]; // the color applied to the index label
			if (i == current) strcpy(indexColor, C_CURRENT);
			else strcpy(indexColor, C_RESET);

			char textColor[8]; // the color applies to the text of the option
			if (selected[i]) { strcpy(textColor, C_RESET); }
			else { strcpy(textColor, C_UNSELECTED); }

			printf("%s[%d] %s%s\n"C_RESET, indexColor, i, textColor, options[i]);
		}

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