#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ctype.h>

int strcmp2(char* a, char* b){
    for(int i = 0; ; i++){
        int compare = tolower((unsigned char) a[i]) - tolower((unsigned char) b[i]);
        if (compare != 0 || !a[i]) return compare;
    }
}

struct node {
    struct dirent *file;
    struct node *left;
    struct node *right;
};

struct node* new_node(struct dirent *newfile)
{
    struct node *new;
    new = malloc(sizeof(struct node));
    new->file = newfile;
    new->left = NULL;
    new->right = NULL;
    return new;
}

struct node* insert(struct node *root, struct dirent *newfile)
{
    if(root == NULL){
        return new_node(newfile);
    }else if(strcmp2(root->file->d_name, newfile->d_name) > 0){
        root->left = insert(root->left, newfile);
    }else{
        root->right = insert(root->right, newfile);
    }
    return root;
}

int listDirectoryFiles();

int printTree(struct node *root, char *directory, int level){
    if(root!=NULL)
        {
            printTree(root->left, directory, level);
            free(root->left);
            for(int i = 0; i < level; i++){ printf("  "); }
            printf("- %s\n", root->file->d_name);
            if(root->file->d_type == DT_DIR){
                char* newpath = malloc( strlen(directory) + strlen(root->file->d_name) + 2);
                strcpy(newpath, directory);
                strcat(newpath, "/");
                strcat(newpath, root->file->d_name);
                listDirectoryFiles(newpath, level + 1);  
                free(newpath);
            }
            printTree(root->right, directory, level);
            free(root->right);
            return 0;
        }
    return 1;
}

int listDirectoryFiles(char *directory, int level){
    struct dirent *file;
    DIR *dir = opendir(directory);
    struct node *root = NULL;
    if (dir != NULL){
        while ((file = readdir(dir)) != NULL){
            if(file->d_name[0] != '.'){
                root = insert(root, file);  
            }
        }
        printTree(root, directory, level);
        free(root);
        closedir(dir);
        return 0;
    }
    return 1;
}


int main(void){
    printf(".\n");
    return listDirectoryFiles(".", 0);
}
