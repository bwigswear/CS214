#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

typedef struct node{
	int data;
	char data2[1000];
	struct node *right;
	struct node *left;
}node;

int strcmp2(char* a, char* b){
	for(int i = 0;; i++){
		int compare = tolower((unsigned char)a[i]) - tolower((unsigned char)b[i]);
		if(compare != 0 || !a[i]){
			return compare;
		}
	}
}

node* newnode(int a){
	node* new = malloc(sizeof(node));
	new->data = a;
	new->left = NULL;
	new->right = NULL;
	return new;
}

node* newnode2(char a[1000]){
	node* new = malloc(sizeof(node));
	strcpy(new->data2, a);
	new->left = NULL;
	new->right = NULL;
	return new;
}


node* insert(node* root, int a){
	if(root == NULL){
		return newnode(a);
	}else if(a > root->data){
		root->right = insert(root->right, a);
	}else{
		root->left = insert(root->left, a);
	}
	return root;
}

node* insert2(node* root, char a[1000]){
	if(root == NULL){
		return newnode2(a);
	}else if(strcmp2(a, root->data2) > 0){
		root->right = insert2(root->right, a);
	}else{
		root->left = insert2(root->left, a);
	}
	return root;
}

void printtree(node* root){

	if(root != NULL){
		printtree(root->left);
		printf("%d\n", root->data);
		printtree(root->right);
		free(root);
	}
}


void printtree2(node* root){

	if(root != NULL){
		printtree2(root->left);
		printf("%s\n", root->data2);
		printtree2(root->right);
		free(root);
	}
}

int main(int argv, char** argc){
	if(argv == 0){return 0;}
	node* root;
	if(argc[1] != NULL && strcmp(argc[1], "-n") == 0){
		int current;
		if(scanf("%d", &current) == 1){
			root = newnode(current);
			while(scanf("%d", &current) == 1){
				root = insert(root, current);
			}
			printtree(root);
		}
	}else{
		char current[1000];
		if(scanf("%s", current) == 1){
			root = newnode2(current);
			while(scanf("%s", current) == 1){
				root = insert2(root, current);
			}
			printtree2(root);
		}
	}
}
