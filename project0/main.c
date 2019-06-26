/*
Author: Fabian Caraballo
Duck ID: fpc
Tite of Assignment: CIS415 Project 0
This is my own work.
*/

#include "anagram.h"

int main(int argc, char *argv[]) {
	
	FILE *in = NULL;
	FILE *out = NULL;

	/*Roscoe's recommendation as to how to go about 
	reading and writing to files for this project. Taught us in lab2
	*/
	if (argc == 2 || argc == 3) {
		in = fopen(argv[1], "r");
	}
	else {
		in = stdin;
	}

	if (argc == 3) {
		out = fopen(argv[2], "w");
	}
	else {
		out = stdout;
	}

	struct AnagramList *AList = NULL;
	char buffer[256];
	char reader;

	int i = 0;
	while ((reader = getc(in)) != EOF) {
		if (reader == '\n') {
			buffer[i] = '\0';
			i = 0; 
			AddWordAList(&AList, buffer);
		}
		else {
			buffer[i] = reader; 
			i++;
		}
	}

	PrintAList(out, AList);
	FreeAList(&AList);
	fclose(in);
	fclose(out);
	
	return 0;
}
