#include <stdio.h>
#include "jars.h"
#include "scui.h"

char helpText[] =
#include "helptext.txt" 


int main(int argc, char const *argv[])
{
	if (argc == 1) {
		printf(helpText);
		return 0;
	}

	return 0;
}