/*

Author: Fabian Caraballo
DuckID: fpc
Assignment: project0
This is my own work except for the QuickSort, Partition, loweCaser which I cited in the comments of each of the functions

*/


#include "anagram.h"



/*

Things to note:

-From Piazza, Roscoe's comment:
	-"A StringList node holds a single word, and a pointer to the next node
	-An Anagram node holds an anagram, an Slist for the words that are part of this anagram, and a pointer to the next node"


*/

 
void quicksort(char * word, int first, int last);
void lowerCaser(char * oldWord, char * newWord);
int partition(char * word, int first, int last);

/*Create a new string list node*/
struct StringList *MallocSList(char *word) {
	
	/* allocate memory for StringList structure */
	struct StringList *newSlist = (struct StringList*)malloc(sizeof(struct StringList));
	

	/* This video helped me understand how to properly malloc this struct
		https://www.youtube.com/watch?v=CZJ-6liXoMs */


	/*Allocate StringList node, free struct if fail */
	newSlist->Word = (char *)malloc(sizeof(char) * (strlen(word) +1));
	newSlist->Next = NULL; /*Assign the next node to null since the next node isn't assigned*/



	strcpy(newSlist->Word, word);

	return newSlist;
};

/*Append a string list node to the end/tail of a string list*/
void AppendSList(struct StringList **head, struct StringList *node) {

	if(*head == NULL) {
		

		*head = node; /* This checks to see if there is an empty stringlist, if there is then we immediately append *node to the **head of the StringList */ 
	
	}	

	else {
		struct StringList * newNode = *head;
		
		while (newNode->Next != NULL) {
			

			/*Pretty straight forward, just going through stringlist and once we find the null node then we can insert the *node inside of there and create a new Null node*/
			

			newNode = newNode->Next;
		}
		

		newNode->Next= node;

	}		

}

/*Free a string list, including all children*/
void FreeSList(struct StringList **node) {

	struct StringList * newNode  = *node;
	struct StringList * nextNode = NULL;
	
	while(newNode != NULL) {
		
		nextNode = newNode->Next;
                free(newNode->Word);
                free(newNode);
                newNode = nextNode;

	}

}


/*Format output to a file according to specification*/
void PrintSList(FILE *file,struct StringList *node) {

	struct StringList * newNode = node;
        while(newNode != NULL) {
		fprintf(file, "\t%s\n", newNode->Word);
		newNode = newNode->Next;
        }

}

/*Return the number of strings in the string list*/
int SListCount(struct StringList *node) {
	
	struct StringList * firstNode = node;

	int counter = 0;

	/*While loop that iterates through the list and increments by 1 for each string in the list, until the NULL node is found(Which means the list ended) */
	while(firstNode) {
		counter++;
		firstNode = firstNode->Next;
	}
	
	/*returns the count*/
	return counter;
}







/*Create a new anagram node, including the string list node with the word */
struct AnagramList* MallocAList(char *word) {

	struct AnagramList *newAlist = (struct AnagramList*)malloc(sizeof(struct AnagramList));
	newAlist->Next = NULL; /*Since we are creating a newNode, makes sure the nextnode of the newer node is null, since it hasn't been assigned a value*/
	
	int len = strlen(word)-1;
	
	newAlist->Anagram = (char*)malloc((len) * sizeof(char));
	strcpy(newAlist->Anagram, word);
	lowerCaser(word, newAlist->Anagram); /*This lowercases everything in "word" */


	quicksort(newAlist->Anagram, 0, len);

	newAlist->Words = MallocSList(word);

	return newAlist;
	

	
};

/*Free an anagram list, including anagram chuldren and string list words*/
void FreeAList(struct AnagramList **node) {
	
	struct AnagramList *newNode = NULL;
	struct AnagramList *aNode = *node;
	
	while(aNode != NULL) {
		newNode = aNode->Next;
                free(aNode->Anagram);
                FreeSList(&aNode->Words);
                free(aNode);
                aNode = newNode;
	}


}

/*Format output to a file, print anagram list with words, according to the spec*/
void PrintAList(FILE *file,struct AnagramList *node) { 

	struct AnagramList *aNode = node;

	int wordCount = SListCount(aNode->Words);


	while(aNode != NULL) {


		wordCount = SListCount(aNode->Words);


		if(wordCount > 1) { /* This if statement is so important. We need to ensure that we don't print every anagram. Just the anagrams that have 2 or more words !! */


			fprintf(file, "%s:%d\n", aNode->Anagram, wordCount);

			PrintSList(file, aNode->Words);
		}
		

		aNode = aNode->Next;
	}


}

/*Add a new word to the anagram list*/
void AddWordAList(struct AnagramList **node, char *word) { 


	 char buffer[256];
         strcpy(buffer, word);


         lowerCaser(word, buffer);
         quicksort(buffer, 0, strlen(buffer) - 1);


	if ((*node) == NULL) {
		*node = MallocAList(word);
		return;
	}
	while(*node != NULL) {

		if(strcmp(buffer, ((*node)-> Anagram))  == 0) {
			struct StringList *newNode = MallocSList(word);
			AppendSList(&(*node)->Words, newNode);
			break;
		}
		else {

			if((*node)->Next == NULL) {
				(*node)->Next = MallocAList(word);
				break;
			}
			else {
				node = &(*node)->Next;

			}
		
		}
		

	}
	
	
}




















/* 	

NOTE from project0.pdf:
	
"You will need to implement a function to un-capitalize a character array and a function to sort the character array. Quick sort is recommended, PLEASE CITE YOUR SOURCE!!!"

*/


void lowerCaser(char * oldWord, char * newWord) {

	/*

	https://stackoverflow.com/questions/2661766/how-do-i-lowercase-a-string-in-c

	This is a very simple way to lowercase the string in C that I was able to find on stackerflow. It's the first link that pops up when you type in Google "how to convert a string to all lowercase in C". It uses the <ctype.h> library in C with the function "tolower". 
	*/

	int i = 0;
	for (i = 0; oldWord[i]; i++) {
		newWord[i] = tolower(oldWord[i]);
	}	
	
}

/* Another function referenced from the geeksforgeeks which swaps two elements in the list */
void swap(char* a, char* b) {

        char t = *a;
        *a = *b;
        *b = t;

}



void quicksort(char * word, int first, int last) {

	/* Using geeksforgeeks as a reference to help with quicksort. They do a really good job of breaking down the sort as well as the partioning that will be used. The link is: https://www.geeksforgeeks.org/quick-sort/ 

	THINGS TO NOTE ABOUT THIS QUICKSORT:
	 - We are going to be looking at char instead of int, since we are going to be sorting words. */

	if(first < last) {
		int pi = partition(word, first, last);

		quicksort(word, first, pi - 1);
		quicksort(word, pi + 1, last);
	}


}

/* This is used from the same geeksfogeeks page(just to be clear), partitioning is a very important part of quick sort.

Partitioning is the main part of quick sort which changes position of the pivot and reogranizes the list of elements to ensure that the list is sorted. */

int partition(char * word, int first, int last) {
	char pivot = word[last];
	int i = (first - 1);

	
	int j = 0;
	for(j = first; j <= last - 1; j++) {
	
		if(word[j] < pivot) {

			i++;
			swap(&word[i], &word[j]);

		}

	}

	swap(&word[i+1], &word[last]);
	return (i+1);
	
}

