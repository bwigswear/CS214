#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node{
	struct node* next;
	int count;
	char word[1000];
}node;

int main(int argv, char** argc){
	node* head = malloc(sizeof(node));
	head->count = 0;
	node* traverse;
	node* current;
	char in[1000];
	while(scanf("%s", in) > 0){
		if(head->count == 0){
			head->count = 1; strcpy(head->word, in); head->next = malloc(sizeof(node)); head->next->count = 0; current = head;
		}else{
				if(strcmp(current->word, in) == 0){
					current->count++;
				}else{
					current->next->count = 1;
					strcpy(current->next->word, in);
					current->next->next = malloc(sizeof(node));
					current->next->next->count = 0;
					current = current->next;
				}


				/*traverse = head;
				if(traverse->next->count == 0){traverse->next->next = malloc(sizeof(node)); traverse->next->count = 1; strcpy(traverse->next->word, in); traverse = traverse->next; traverse->next->count = 0;}
				while(traverse->next->count != 0){
					if(strcmp(traverse->next->word, in) == 0){traverse->next->count = traverse->next->count + 1; break;}
					traverse = traverse->next;
					if(traverse->next->count == 0){traverse->next->count = 1; strcpy(traverse->next->word, in); traverse->next->next = malloc(sizeof(node)); traverse->next->next->count = 0; break;}
				}*/
		}
	}
	traverse = head->next;
	node* traversefree = traverse;
	printf("%d %s\n", head->count, head->word);
	free(head);
	if(traverse->count != 0){
		while(traverse->next->count != 0){
			traversefree = traverse;
			printf("%d %s\n", traverse->count, traverse->word);
			traverse = traverse->next;
			free(traversefree);
		}
		printf("%d %s\n", traverse->count, traverse->word);
		free(traverse->next);
	}
	free(traverse);
}
