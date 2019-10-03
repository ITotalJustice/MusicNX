#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

int randomizerInit()
{
    struct timeval time; 
    gettimeofday(&time, NULL);
    srand((time.tv_sec * 1000) + (time.tv_usec / 1000));
    return 0;
}

int randomizer(int low, int high)
{
    return rand() % (high - low + 1) + low;
}