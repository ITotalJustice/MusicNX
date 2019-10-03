#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <switch.h>

#include "dir.h"

int scanDir(char *directory)
{
    int files_found = 0;

    DIR *dir = opendir(directory);
    struct dirent *de;

    if (dir)
    {
        while ((de = readdir(dir)))
            if (strstr(de->d_name, ".mp3"))
                files_found++;
                
        closedir(dir);
    }
    
    return files_found;
}

node* createNode(int number_of_files, char *folder_location)
{
    DIR *dir = opendir(folder_location);
    struct dirent *de;
    struct node *files =  NULL;
    
    if (dir)
    {
        files = malloc(number_of_files * sizeof(*files) + sizeof(*files));

        int i = 0;
        while ((de = readdir(dir)))
            if (strstr(de->d_name, ".mp3"))
            {
                snprintf(files[i].file_name, 263, "%s", de->d_name);
                i++;
            }

        closedir(dir);
    }

    return files;
}

void freeNode(node *head)
{
    if (head == NULL) return;

	free(head);
}