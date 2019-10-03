#ifndef _MUSIC_H_
#define _MUSIC_H_


void mp3Tag(char *music);

int playMus(char **array, int file, int files_max);
int skipnext(char **array, int file, int files_max);
int skipback(char **array, int file, int files_max);
void musRewind(double value);

void musicInit();
void musicExit();

#endif