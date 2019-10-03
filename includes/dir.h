#ifndef _DIR_H_
#define _DIR_H_

#define MUSIC_DIR "/music/"

typedef struct node
{
    char file_name[256];
    int position;           // not used yet...
} node;

int scanDir(char *directory);                                       // scan entire dir, return number of files found.
node* createNode(int number_of_files, char *folder_location);       // create a node sizeof (node * number_of_files).
void freeNode(node *head);                                          // free the node (remember to set it to NULL after).

#endif