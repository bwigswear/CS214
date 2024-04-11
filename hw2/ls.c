#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>
#include <pwd.h>
#include <time.h>

typedef struct node{
	struct dirent *file;
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

node* newnode(struct dirent *a){
	node* new = malloc(sizeof(node));
	new->file = a;
	new->left = NULL;
	new->right = NULL;
	return new;
}

node* insert(node* root, struct dirent *a){
	if(root == NULL){
		return newnode(a);
	}else if(strcmp2(a->d_name, root->file->d_name) > 0){
		root->right = insert(root->right, a);
	}else{
		root->left = insert(root->left, a);
	}
	return root;
}

void printtree(node* root){
	if(root != NULL){
		printtree(root->left);
		printf("%s\n", root->file->d_name);
		printtree(root->right);
		free(root);
	}
}

void printtree2(node* root){
	if(root != NULL){
		printtree2(root->left);
		struct stat filedata;
		int a = stat(root->file->d_name, &filedata);

		//INSTRUCTIONS SAY TO GRAB USER AND GROUP IDS IF NAMES ARE NOT AVAILABLE
		struct passwd *b = getpwuid(filedata.st_uid);
		struct group *c = getgrgid(filedata.st_gid);

		char time[25];
		strcpy(time, ctime(&filedata.st_mtime));
		char subtime[13];
		memcpy(subtime, &time[4], 12);
		subtime[12] = '\0';

		printf((S_ISDIR(filedata.st_mode)) ? "d" : "-");
		printf((S_IRUSR & filedata.st_mode) ? "r" : "-");
		printf((S_IWUSR & filedata.st_mode) ? "w" : "-");
		printf((S_IXUSR & filedata.st_mode) ? "x" : "-");
		printf((S_IRGRP & filedata.st_mode) ? "r" : "-");
		printf((S_IWGRP & filedata.st_mode) ? "w" : "-");
		printf((S_IXGRP & filedata.st_mode) ? "x" : "-");
		printf((S_IROTH & filedata.st_mode) ? "r" : "-");
		printf((S_IWOTH & filedata.st_mode) ? "w" : "-");
		printf((S_IXOTH & filedata.st_mode) ? "x" : "-");

		if(b->pw_name == NULL){printf(" %X", b->pw_uid);}else{printf(" %s", b->pw_name);}
		if(c->gr_name == NULL){printf(" %X", c->gr_gid);}else{printf(" %s", c->gr_name);}
		printf(" %ld %s %s\n", filedata.st_size, subtime, root->file->d_name);

		//printf(" %s %s %ld %s %s\n", b->pw_name, c->gr_name, filedata.st_size, subtime, root->file->d_name);

		printtree2(root->right);
		free(root);
	}
}

int main(int argv, char** argc){
	DIR *dir;
	dir = opendir(".");
	struct dirent *current = readdir(dir);
	node* root = NULL;

	if(current->d_name[0] != 46){root = insert(root,current);}

	while((current = readdir(dir)) != NULL){

		/*if(strcmp(current->d_name, "..") && strcmp(current->d_name, ".") ){
			root = insert(root,current);
		}*/
		if(current->d_name[0] != 46){
			root = insert(root, current);
		}
	}

	if(argv > 1 && (strcmp(argc[1], "-l") == 0)){
		printtree2(root);
	}else{
		printtree(root);
	}
	int a = closedir(dir);
}

