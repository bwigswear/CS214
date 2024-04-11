#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

char* query;

int listDirectoryFiles(char *directory){
    struct dirent *file;
    DIR *dir = opendir(directory);
    if (dir != NULL){
        while ((file = readdir(dir)) != NULL){
            if(file->d_name[0] != '.'){
                if (strstr(file->d_name, query)) printf("%s/%s\n", directory, file->d_name);
                if(file->d_type == DT_DIR){
                    char* newpath = malloc( strlen(directory) + strlen(file->d_name) + 2);
                    strcpy(newpath, directory);
                    strcat(newpath, "/");
                    strcat(newpath, file->d_name);
                    listDirectoryFiles(newpath);
                    free(newpath);
                }
            }
        }
        closedir(dir);
        return 0;
    }
    return 1;
}

int main(int argc, char **argv){
    if(argc < 2) return 1;
    query = argv[1];
    return listDirectoryFiles(".");
}
