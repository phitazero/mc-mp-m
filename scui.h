#include <conio.h>

#define C_RESET "\e[0m"
#define C_SELECTED "\e[47m\e[0;30m"

#define KEY_ENTER 13
#define KEY_ESC 27

// n_sth - number of sth, e.g. n_options - number of options 

int multichoice(int n_options, char* options[], char* preText) {
	if (n_options < 1) return -1;

	int current = 0;	
	for (;;) {
		system("cls");
		if (preText) printf("%s\n", preText);

		for (int i = 0; i < n_options; i++) {
			if (i == current) printf(C_SELECTED);
			printf("[%d] > %s <\n", i, options[i]);
			printf(C_RESET);
		}

		char input = getch();
		if (input == KEY_ENTER) return current;
		else if (input == KEY_ESC) return -1; // used only in case of a softlock
		else if (input == 224) { // KEY_UP and KEY_DOWN are 2 bytes wide, so we need a second getch()
			char input2 = getch();
			// KEY_DOWN
			if (input2 == 72 && current < n_options - 1) current++;
			// KEY_UP
			else if (input2 == 80 && current > 0) current--;
		}
		else if ('0' < input && input < '9') {
			int index = input - '0'; // convert ['0'-'9'] to [0-9]
			if (index < n_options) current = index;
		}
	}
}