#include <stdio.h>
#include "jars.h"
#include "scui.h"

void printHelpText(char* filename) {
	printf("here comes the help text, btw the filename is %s", filename); // TEMPORARY
}

int main(int argc, char *argv[])
{
	if (argc == 1) {
		printHelpText(argv[0]);
		return 0;
	}

	return 0;
}