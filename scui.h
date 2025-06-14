#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define C_RESET "\e[0m"
#define C_CURRENT "\e[30;47m"
#define C_UNSELECTED "\e[0;90m"

#define KEY_ENTER 10

int getch() {
	struct termios oldt, newt;
	int ch;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	return ch;
}

// selected[i] is a bool (formally an int) indicating whether options[i] was selected
int multichoiceWStates(int n_options, char* options[], int* selected) {
	if (n_options < 1) return -1;

	static int current = 0; // index of currently selected option
	for (;;) {
		system("clear");

		for (int i = 0; i < n_options; i++) {
			char indexColor[9]; // the color applied to the index label
			if (i == current) strcpy(indexColor, C_CURRENT);
			else strcpy(indexColor, C_UNSELECTED);

			char textColor[8]; // the color applies to the text of the option
			if (selected[i]) { strcpy(textColor, C_RESET); }
			else { strcpy(textColor, C_UNSELECTED); }

			printf("%s[%d] %s%s\n"C_RESET, indexColor, i, textColor, options[i]);
		}

		unsigned char input = getch();
		if (input == KEY_ENTER) return current;
		else if (input == 27) { // KEY_UP and KEY_DOWN are 3 bytes wide, so we need two more getch()'s
			unsigned char input2 = getch();
			unsigned char input3 = getch();

			if (input2 != '[') continue;

			// KEY_DOWN
			if (input3 == 'B' && current < n_options - 1) current++;
			// KEY_UP
			else if (input3 == 'A' && current > 0) current--;
		}
		else if ('0' <= input && input <= '9') {
			int index = input - '0'; // convert ['0'-'9'] to [0-9]
			if (index < n_options) current = index;
		}
	}
}

int multichoice(int n_options, char* options[]) {
	if (n_options < 1) return -1;

	static int current = 0; // index of currently selected option
	for (;;) {
		system("clear");

		for (int i = 0; i < n_options; i++) {
			char indexColor[9]; // the color applied to the index label
			if (i == current) strcpy(indexColor, C_CURRENT);
			else strcpy(indexColor, C_UNSELECTED);

			printf("%s[%d]"C_RESET" %s\n"C_RESET, indexColor, i, options[i]);
		}

		unsigned char input = getch();
		if (input == KEY_ENTER) return current;
		else if (input == 27) { // KEY_UP and KEY_DOWN are 3 bytes wide, so we need two more getch()'s
			unsigned char input2 = getch();
			unsigned char input3 = getch();

			if (input2 != '[') continue;

			// KEY_DOWN
			if (input3 == 'B' && current < n_options - 1) current++;
			// KEY_UP
			else if (input3 == 'A' && current > 0) current--;
		}
		else if ('0' <= input && input <= '9') {
			int index = input - '0'; // convert ['0'-'9'] to [0-9]
			if (index < n_options) current = index;
		}
	}
}
