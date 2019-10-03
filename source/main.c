#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>

#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#include <switch.h>

#include "sdl.h"
#include "music.h"
#include "util.h"
#include "dir.h"


bool g_IsScroll = false, id3v2Found = false, id3v2Enable = true, shuffleMode = false, secNeedsZero = false, displayMin = false, inTab = false;
int number_of_files = 0, current_song = 0, g_CursorList = 0, g_MaxList = 12, songListMax = 11, g_MusListY = 150, g_MusListX = 650, g_MusListMaxY = 635, g_MusListMaxX = 1000, scrollNowplay = 60, \
    g_CursorScroll = 650, g_SongInSec = -1, g_SongInMin = 0, g_ListMoveTemp = 0, g_ListMove = 0, errorCode = 0, menuTab = 0;
static double songSecTotal, songProgressBar = 0, songProgressBarIncrease = 0;

double rewindValue = -1;

int initApp()
{
    Result rc;

    #ifdef DEBUG
    if (R_FAILED(rc = socketInitializeDefault()))           // for curl / nxlink.
        printf("socketInitializeDefault() failed: 0x%x.\n\n", rc);

    if (R_FAILED(rc = nxlinkStdio()))                       // redirect all printout to console window.
        printf("nxlinkStdio() failed: 0x%x.\n\n", rc)
    #endif

    if (R_FAILED(rc = setsysInitialize()))                  // for system version
        printf("setsysInitialize() failed: 0x%x.\n\n", rc);

    if (R_FAILED(rc = splInitialize()))                     // for atmosphere version
        printf("splInitialize() failed: 0x%x.\n\n", rc);

    if (R_FAILED(rc = plInitialize()))                      // for shared fonts.
        printf("plInitialize() failed: 0x%x.\n\n", rc);

    if (R_FAILED(rc = romfsInit()))                         // load textures from app.
        printf("romfsInit() failed: 0x%x.\n\n", rc);

    sdlInit();                                              // int all of sdl and start loading textures.

    musicInit();

    return 0;
}

void appExit()
{
    #ifdef DEBUG
    socketExit();
    #endif
    romfsExit();
    musicExit();
    sdlExit();
    plExit();
    splExit();
    setsysExit();
}

void touchSong(char **array, int current_song, int number_of_files, int menuTab, int x, int y)
{
    int zero = g_ListMoveTemp, touchtest = 110, touchExt = 50, touchInc = 45, counter = 0;

    while(menuTab == 0 && counter < g_MaxList && y < g_MusListMaxY && y >= 110 && x < g_MusListMaxX)
    {
        if (x >= 430 && y >= touchtest && y <= touchtest + touchExt)
        {
            printf("value of y = %d\n", y);
            current_song = zero;
            g_CursorList = zero;
            g_CursorScroll = g_MusListX;
            shuffleMode = false;
            current_song = playMus(array, current_song, number_of_files);
            break;
        }
        else
        {
            touchtest += touchInc;
            zero++;
            counter++;
        }
    }
}

void sideTabDisplay()
{
    if (menuTab == 0)
    {
        drawText(fntMedium, 90, 280, SDL_GetColour(white), "Settings");
        drawText(fntMedium, 90, 380, SDL_GetColour(white), "Information");
        drawImageScale(right_arrow, 60, 190, 30, 30);
        if (inTab == false) drawText(fntMedium, 120, 180, SDL_GetColour(white), "Music Select");
        else drawText(fntMedium, 120, 180, SDL_GetColour(grey), "Music Select");
    }
    else if (menuTab == 1)
    {
        drawText(fntMedium, 90, 180, SDL_GetColour(white), "Music Select");
        drawText(fntMedium, 90, 380, SDL_GetColour(white), "Information");
        drawImageScale(right_arrow, 60, 290, 30, 30);
        if (inTab == false) drawText(fntMedium, 120, 280, SDL_GetColour(white), "Settings");
        else drawText(fntMedium, 120, 280, SDL_GetColour(grey), "Settings");
    }
    else
    {
        drawText(fntMedium, 90, 180, SDL_GetColour(white), "Music Select");
        drawText(fntMedium, 90, 280, SDL_GetColour(white), "Settings");
        drawImageScale(right_arrow, 60, 390, 30, 30);
        if (inTab == false) drawText(fntMedium, 120, 380, SDL_GetColour(white), "Information");
        else drawText(fntMedium, 120, 380, SDL_GetColour(grey), "Information");
    }
}


int main(int argc, char **argv)
{
    initApp();

    printf("Testing...\n\nTesting...\n\nCan you see me?........\n\n");

    if (randomizerInit() != 0) printf("ERROR: randomizer failed!\n");

    chdir(MUSIC_DIR);
    char *files[500] = {'\0'};
    int current_song = 0;
    int number_of_files = 0;

	DIR *dr = opendir(MUSIC_DIR);
    struct dirent *de;

    for (int i = 0; (de = readdir(dr)); i++, number_of_files++)
    {
        if (strstr(de->d_name, ".mp3") != NULL || strstr(de->d_name, ".MP3") != NULL || strstr(de->d_name, ".ogg") != NULL || strstr(de->d_name, ".OGG") != NULL \
            || strstr(de->d_name, ".wav") != NULL || strstr(de->d_name, ".WAV") != NULL || strstr(de->d_name, ".MOD") != NULL || strstr(de->d_name, ".flac") != NULL)
        {
            size_t size = strlen(de->d_name) + 1;
            files[i] = malloc(size);
            snprintf(files[i], size, "%s", de->d_name);
        }
    }
    number_of_files--;
    closedir(dr);

    SDL_Texture *allThemes[] =  { black_background, white_background };
    SDL_Colour colour = SDL_GetColour(white), highlight = SDL_GetColour(grey);
    char *allColourString[22] = {"white", "grey", "black", "pink", "hotPink", "orange", "yellow", "gold", "brown", "red", "darkRed", "green", "limeGreen", "aqua", "teal", "lightBlue", "blue", "darkBlue", "purple", "indigo", "beige", '\0'};
    char *menuTab1Options[] = {"Autoplay:", "Set Custom Music Path:", "Set Rewind / Fast Forward Value:", "Set Theme / Background:", "Set Text Colour:", "Set Highlight Colour:", "Experimental Settings:", "Reset All To default:"};

    bool loop1 = false, loopall = false, autoplay = true;
    float scrollList = 0, scrollListSpeedDefault = 5, scrollListSpeedIncrease = scrollListSpeedDefault;
    int fpsCounter = 0, secTemp = 61, touchHeldTime = 0, touchCheck = 0, touchTempx = 0, touchTempY = 0, playmode = 0;
    u32 touchCount = 0;

    int colourListMax = 20, settingsSubMenuCounter = 0, textColourCursourPosition = 0, highlightCursourPosition = 0, textColourListTemp = 0, highlightListTemp = 0;

    int settingsListTemp = 0, settingsListMax = 8, settingsCursorPosition = 0;

    int themeSelect = 0;

    if (autoplay == true) current_song = playMus(files, current_song, number_of_files);

    printf("\nEntering while loop!\n\n");

    while(appletMainLoop())
    {
        hidScanInput();
        u64 kdown = hidKeysDown(CONTROLLER_P1_AUTO);
        u64 kheld = hidKeysHeld(CONTROLLER_P1_AUTO);
        u32 tch = 0;
        touchPosition touch;
        hidTouchRead(&touch, tch);
        touchCount = hidTouchCount();

        time_t unixTime = time(NULL);
        struct tm* timeStruct = gmtime((const time_t *)&unixTime);
        int seconds = timeStruct->tm_sec;
        bool songClock = true;

/*#####################################################################################################################################*/

        clearRenderer();

        if (id3v2Found == true) drawImageScale(ID3_tag, 60, 515, 300, 300);
        else drawImageScale(musicNX, 60, 515, 300, 300);

        if (Mix_PlayingMusic() && !Mix_PausedMusic()) drawImage(pause_white, 280, 870);
        else drawImage(play_white, 290, 870);
        drawImage(skip_back, 60, 870);
        drawImage(skip_forward, 480, 870);

        //Pause / play
        if (kdown & KEY_TOUCH && touch.px > 186 && touch.px < 256 && touch.py > 580 && touch.py < 640)
        {
            if (!Mix_PausedMusic()) Mix_PauseMusic();
            else Mix_ResumeMusic();
        }

        //skipback
        if (kdown & KEY_TOUCH && touch.px > 40 && touch.px < 110 && touch.py > 580 && touch.py < 640)
            current_song = skipback(files, current_song, number_of_files);

        //skipforward
        if (kdown & KEY_TOUCH && touch.px > 320 && touch.px < 390 && touch.py > 580 && touch.py < 640)
            current_song = skipnext(files, current_song, number_of_files);

        if (loopall == false && loop1 == false) drawImage(loop_white, 380, 700);
        else if (loopall == true) drawImage(loop_grey, 380, 700);
        else drawImage(loop1_icon, 380, 700);

        //loopall / loop1
        if (kdown & KEY_TOUCH && touch.px > 245 && touch.px < 340 && touch.py > 466 && touch.py < 530)
        {
            shuffleMode = false;
            if (loopall == false && loop1 == false)
            {
                loopall = true;
            }
            else if (loopall == true)
            {
                loopall = false;
                loop1 = true;
            }
            else
            {
                loop1 = false;
            }
        }

        if (shuffleMode == false) drawImage(shuffle_white, 490, 700);
        else drawImage(shuffle_grey, 490, 700);

        //shuffle on / off
        if (kdown & KEY_TOUCH && touch.px > 315 && touch.px < 410 && touch.py > 466 && touch.py < 530)
        {
            loopall = false;
            loop1 = false;
            if (shuffleMode == false) shuffleMode = true;
            else shuffleMode = false;
        }

        drawImageScale(musicNX, 60, 30, 160, 90);
        drawImageScale(b_button, 1450, 1000, 50, 50);
        if (menuTab == 0) drawText(fntSmall, 1510, 1010, SDL_GetColour(white), "Pause");
        else drawText(fntSmall, 1510, 1010, SDL_GetColour(white), "Back");
        drawImageScale(a_button, 1650, 1000, 50, 50);
        drawText(fntSmall, 1710, 1010, SDL_GetColour(white), "Select");
        drawText(fntMedium, scrollNowplay, g_MusListMaxX, SDL_GetColour(white), files[current_song]); //Now playing

        drawShape(SDL_GetColour(grey), 55, 840, 525, 15);
        drawShape(SDL_GetColour(neon_pink), 55, 840, songProgressBar, 15);

        sideTabDisplay(); // Displays the left side tab.

        if (menuTab == 0 && settingsSubMenuCounter == 0)
        {
            for (int temp = 0, tempMove = g_ListMoveTemp; temp < g_MaxList; temp++, tempMove++)
            {
                if (g_CursorList == tempMove && inTab == false) // Checks what the selected song is, then changes the colour to highlight, and x becomes the scrolling variable.
                {
                    g_IsScroll = true;
                    drawText(fntSmall, g_CursorScroll, g_MusListY, highlight, files[tempMove]);
                }
                else drawText(fntSmall, g_MusListX, g_MusListY, SDL_GetColour(white), files[tempMove]);
                g_MusListY += 70;
            }
        }

        else if (menuTab == 1)
        {
            if (menuTab == 1 && settingsSubMenuCounter == 0)
            {
                if (autoplay == true) drawImageScale(tick_box, 880, 150, 60, 60);
                else drawImageScale(empty_box_grey, 880, 150, 60, 60);

                for (int temp = 0, tempMove = settingsListTemp; temp < settingsListMax; temp++, tempMove++)
                {
                    if (settingsCursorPosition == tempMove && inTab == false) drawText(fntMediumIsh, g_MusListX, g_MusListY, highlight, menuTab1Options[tempMove]);
                    else drawText(fntMediumIsh, g_MusListX, g_MusListY, SDL_GetColour(white), menuTab1Options[tempMove]);
                    g_MusListY += 100;
                }
            }
            else if (settingsSubMenuCounter == 3)
            {
                drawImageScale(allThemes[0], 650, 150, 180, 135);
                drawText(fntMediumIsh, 900, 200, SDL_GetColour(white), "Black Theme");
                drawImageScale(allThemes[1], 650, 325, 180, 135);
                drawText(fntMediumIsh, 900, 375, SDL_GetColour(white), "White Theme");
                drawImageScale(allThemes[0], 650, 500, 180, 135);
                drawText(fntMediumIsh, 900, 550, SDL_GetColour(white), "TEMP Theme");
                drawImageScale(allThemes[1], 650, 675, 180, 135);
                drawText(fntMediumIsh, 900, 725, SDL_GetColour(white), "TEMP Theme");
                drawText(fntMedium, 650, 900, SDL_GetColour(white), "Load Custom Theme...");
            }

            else if (settingsSubMenuCounter == 4) //Text Colour Menu
            {
                for (int temp = 0, tempMove = textColourListTemp; temp < g_MaxList; temp++, tempMove++)
                {
                    if (textColourCursourPosition == tempMove && inTab == false)
                    {
                        drawImageScale(right_arrow, 650, g_MusListY, 30, 30);
                        drawText(fntSmall, 700, g_MusListY, SDL_GetColour(tempMove), allColourString[tempMove]);
                    }
                    else drawText(fntSmall, g_MusListX, g_MusListY, SDL_GetColour(tempMove), allColourString[tempMove]);
                    g_MusListY += 70;
                }
            }

            else if (settingsSubMenuCounter == 5) //Highlight Colour Menu
            {
                for (int temp = 0, tempMove = highlightListTemp; temp < g_MaxList; temp++, tempMove++)
                {
                    if (highlightCursourPosition == tempMove && inTab == false)
                    {
                        drawImageScale(right_arrow, 650, g_MusListY, 30, 30);
                        drawText(fntSmall, 700, g_MusListY, SDL_GetColour(tempMove), allColourString[tempMove]);
                    }
                    else drawText(fntSmall, g_MusListX, g_MusListY, SDL_GetColour(tempMove), allColourString[tempMove]);
                    g_MusListY += 70;
                }
            }
        }

        if (menuTab == 2)
        {
            drawText(fntLarge, 650, 150, colour, "BETA:");

            drawText(fntMediumIsh, 650, 350, colour, "..................");
            drawText(fntMediumIsh, 650, 450, colour, "Oh hey!");
            drawText(fntMediumIsh, 650, 550, colour, "So.........");
            drawText(fntMediumIsh, 650, 650, colour, "What do you think?");
        }
        updateRenderer();
        g_MusListY = 150;
        g_CursorScroll++;

/*#####################################################################################################################################*/

        if (kheld & KEY_ZL) // Rewind function.
        {
            songClock = false;
            fpsCounter += 20;
            if (fpsCounter == 60)
            {
                fpsCounter = 0;
                if (rewindValue - 1 >= -1) 
                {
                    songProgressBar -= songProgressBarIncrease;
                    rewindValue--;
                }
                if (g_SongInSec - 1 >= 0) g_SongInSec--;
                else if (g_SongInMin >= 1)
                {
                    g_SongInMin--;
                    g_SongInSec = 59;
                }
                else g_SongInSec = -1;
                musRewind(rewindValue);
            }
        }

        if (kheld & KEY_ZR) // Fast forward function.
        {
            songClock = false;
            fpsCounter += 20;
            if (fpsCounter == 60)
            {
                fpsCounter = 0;
                songProgressBar += songProgressBarIncrease;
                if (rewindValue + 1 <= songSecTotal) rewindValue++;
                else current_song = skipnext(files, current_song, number_of_files);

                if (g_SongInSec + 1 <= 59) g_SongInSec++;
                else
                {
                    g_SongInMin++;
                    g_SongInSec = 0;
                }
                Mix_SetMusicPosition(rewindValue);
            }
        }
        
        if (seconds != secTemp)
        {
            secTemp = seconds;
            if (songClock == true && Mix_PausedMusic() == 0)
            {
                songProgressBar += songProgressBarIncrease;
                g_SongInSec++;
                rewindValue++;
                printf("%f\n", rewindValue);
                if (g_SongInSec >= 60)
                {
                    g_SongInSec = 0;
                    g_SongInMin++;
                }
                printf("time is %d:%d\n", g_SongInMin, g_SongInSec);
            }
        }

        if (kdown & KEY_TOUCH) { touchHeldTime = 0; touchCheck = 0; touchTempx = 0; touchTempY = 0;}
        if (touchCheck == 1 || (touch.px > 430 && touch.py > 110 && touch.py < 630 && menuTab == 0))
        {
            if (kheld & KEY_TOUCH && touchCount == 1 && touchHeldTime < 10) {touchTempx = touch.px; touchTempY = touch.py; touchHeldTime++; touchCheck = 1;}
            else if (touchHeldTime < 10)
            {
                inTab = false;
                touchCheck = 0;
                touchSong(files, current_song, number_of_files, menuTab, touchTempx, touchTempY);
            }
            else if (kheld & KEY_TOUCH) printf("you touchheld!!\n");
        }

        if (touch.px > 60 && touch.px < 300 && touch.py > 120 && touch.py < 293)
        {
            if (touch.py > 120 && touch.py < 160)
            {
                menuTab = 0;
                inTab = true;
            }
            else if (touch.py > 186 && touch.py < 226)
            {
                menuTab = 1;
                inTab = true;
            }
            else if (touch.py > 252 && touch.py < 293)
            {
                menuTab = 2;
                inTab = true;
            }
        }

        if (kdown & KEY_DOWN && inTab == false) //|| (leftStickTab == false && kdown & KEY_DOWN))
        {
            g_CursorScroll = g_MusListX;
            scrollList = 0;
            scrollListSpeedIncrease = scrollListSpeedDefault;
            if (menuTab == 0)
            {
                if (g_CursorList + 1 > number_of_files)
                {
                    g_CursorList = 0;
                    g_ListMoveTemp = 0;
                    printf("loop to the top g_CursorList = %d = %s\n", g_CursorList, files[g_CursorList]);
                }
                else
                {
                    g_CursorList++;
                    if (g_ListMoveTemp + g_MaxList == g_CursorList) g_ListMoveTemp++;
                    printf("move down g_CursorList = %d = %s\n", g_CursorList, files[g_CursorList]);
                }
            }
            else if (menuTab == 1)
            {
                if (settingsSubMenuCounter == 0)
                {
                    if (settingsCursorPosition + 1 > 7)
                    {
                        settingsCursorPosition = 0;
                        settingsListTemp = 0;
                    }
                    else
                    {
                        settingsCursorPosition++;
                        if (settingsListTemp + settingsListMax == settingsCursorPosition) settingsListTemp++;
                    }
                }
                else if (settingsSubMenuCounter == 3)
                {
                    if (themeSelect + 1 > 3)
                    {
                        themeSelect = 0;
                    }
                    else
                    {
                        themeSelect++;
                    }
                }
                else if (settingsSubMenuCounter == 4)
                {
                    if (textColourCursourPosition + 1 > colourListMax)
                    {
                        textColourCursourPosition = 0;
                        textColourListTemp = 0;
                    }
                    else
                    {
                        textColourCursourPosition++;
                        if (textColourListTemp + g_MaxList == textColourCursourPosition) textColourListTemp++;
                    }
                }
                else if (settingsSubMenuCounter == 5)
                {
                    if (highlightCursourPosition + 1 > colourListMax)
                    {
                        highlightCursourPosition = 0;
                        highlightListTemp = 0;
                    }
                    else
                    {
                        highlightCursourPosition++;
                        if (highlightListTemp + g_MaxList == highlightCursourPosition) highlightListTemp++;
                    }
                }
            }
        }

        if (kdown & KEY_UP  && inTab == false)
        {
            scrollList = 0;
            scrollListSpeedIncrease = scrollListSpeedDefault;
            g_CursorScroll = g_MusListX;
            if (menuTab == 0)
            {
                if (g_CursorList - 1 < 0)
                {
                    g_CursorList = number_of_files;
                    if (number_of_files - songListMax < 0) g_ListMoveTemp = 0;
                    else g_ListMoveTemp = number_of_files - songListMax;
                    printf("loop to the bottom g_CursorList = %d = %s\n", g_CursorList, files[g_CursorList]);
                }
                else
                {
                    g_CursorList--;
                    if (g_ListMoveTemp - 1 == g_CursorList) g_ListMoveTemp--;
                    printf("move up g_CursorList = %d = %s\n", g_CursorList, files[g_CursorList]);
                }
            }
            else if (menuTab == 1)
            {
                if (settingsSubMenuCounter == 0)
                {
                    if (settingsCursorPosition - 1 < 0)
                    {
                        settingsCursorPosition = 7;
                        if (7 - settingsListMax < 0) settingsListTemp = 0;
                        else settingsListTemp = 7 - songListMax;
                    }
                    else
                    {
                        settingsCursorPosition--;
                        if (settingsListTemp - 1 == settingsCursorPosition) settingsListTemp--;
                    }
                }
                else if (settingsSubMenuCounter == 3)
                {
                    if (themeSelect - 1 < 0)
                    {
                        themeSelect = 3;
                    }
                    else
                    {
                        themeSelect--;
                    }
                }
                else if (settingsSubMenuCounter == 4)
                {
                    if (textColourCursourPosition - 1 < 0)
                    {
                        textColourCursourPosition = colourListMax;
                        if (colourListMax - songListMax < 0) textColourListTemp = 0;
                        else textColourListTemp = colourListMax - songListMax;
                    }
                    else
                    {
                        textColourCursourPosition--;
                        if (textColourListTemp - 1 == textColourCursourPosition) textColourListTemp--;
                    }
                }
                else if (settingsSubMenuCounter == 5)
                {
                    if (highlightCursourPosition - 1 < 0)
                    {
                        highlightCursourPosition = colourListMax;
                        if (colourListMax - songListMax < 0) highlightListTemp = 0;
                        else highlightListTemp = colourListMax - songListMax;
                    }
                    else
                    {
                        highlightCursourPosition--;
                        if (highlightListTemp - 1 == highlightCursourPosition) highlightListTemp--;
                    }
                }
            }
        }

        if (kheld & KEY_DOWN && inTab == false)
        {
            scrollList += scrollListSpeedIncrease;
            if (scrollList >= 60)
            {
                g_CursorScroll = g_MusListX;
                scrollList = 0;
                scrollListSpeedIncrease++;
                if (menuTab == 0)
                {
                    if (g_CursorList + 1 > number_of_files)
                    {
                        g_CursorList = 0;
                        g_ListMoveTemp = 0;
                        printf("loop to the top g_CursorList = %d = %s\n", g_CursorList, files[g_CursorList]);
                    }
                    else
                    {
                        g_CursorList++;
                        if (g_ListMoveTemp + g_MaxList == g_CursorList) g_ListMoveTemp++;
                        printf("move down g_CursorList = %d = %s\n", g_CursorList, files[g_CursorList]);
                    }
                }
                else if (menuTab == 1) ////////////HEERE
                {
                    if (settingsSubMenuCounter == 0)
                    {
                        if (settingsCursorPosition + 1 > 7)
                        {
                            settingsCursorPosition = 0;
                            settingsListTemp = 0;
                        }
                        else
                        {
                            settingsCursorPosition++;
                            if (settingsListTemp + settingsListMax == settingsCursorPosition) settingsListTemp++;
                        }
                    }
                    else if (settingsSubMenuCounter == 4)
                    {
                        if (textColourCursourPosition + 1 > colourListMax)
                        {
                            textColourCursourPosition = 0;
                            textColourListTemp = 0;
                        }
                        else
                        {
                            textColourCursourPosition++;
                            if (textColourListTemp + g_MaxList == textColourCursourPosition) textColourListTemp++;
                        }
                    }
                    else if (settingsSubMenuCounter == 5)
                    {
                        if (highlightCursourPosition + 1 > colourListMax)
                        {
                            highlightCursourPosition = 0;
                            highlightListTemp = 0;
                        }
                        else
                        {
                            highlightCursourPosition++;
                            if (highlightListTemp + g_MaxList == highlightCursourPosition) highlightListTemp++;
                        }
                    }
                }
            }
        }

        if (kheld & KEY_UP && inTab == false)
        {
            scrollList += scrollListSpeedIncrease;
            if (scrollList >= 60)
            {
                g_CursorScroll = g_MusListX;
                scrollList = 0;
                scrollListSpeedIncrease++;
                if (menuTab == 0)
                {
                    if (g_CursorList - 1 < 0)
                    {
                        g_CursorList = number_of_files;
                        if (number_of_files - songListMax < 0) g_ListMoveTemp = 0;
                        else g_ListMoveTemp = number_of_files - songListMax;
                        printf("loop to the bottom g_CursorList = %d = %s\n", g_CursorList, files[g_CursorList]);
                    }
                    else
                    {
                        g_CursorList--;
                        if (g_ListMoveTemp - 1 == g_CursorList) g_ListMoveTemp--;
                        printf("move up g_CursorList = %d = %s\n", g_CursorList, files[g_CursorList]);
                    }
                }
                else if (menuTab == 1)
                {
                    if (settingsSubMenuCounter == 0)
                    {
                        if (settingsCursorPosition - 1 < 0)
                        {
                            settingsCursorPosition = 7;
                            if (7 - settingsListMax < 0) settingsListTemp = 0;
                            else settingsListTemp = 7 - settingsListMax;
                        }
                        else
                        {
                            settingsCursorPosition--;
                            if (settingsListTemp - 1 == settingsCursorPosition) settingsListTemp--;
                        }
                    }
                    else if (settingsSubMenuCounter == 4)
                    {
                        if (textColourCursourPosition - 1 < 0)
                        {
                            textColourCursourPosition = colourListMax;
                            if (colourListMax - songListMax < 0) textColourListTemp--;
                            else textColourListTemp = colourListMax - songListMax;
                        }
                        else
                        {
                            textColourCursourPosition--;
                            if (textColourListTemp - 1 == textColourCursourPosition) textColourListTemp--;
                        }
                    }
                    else if (settingsSubMenuCounter == 5)
                    {
                        if (highlightCursourPosition - 1 < 0)
                        {
                            highlightCursourPosition = colourListMax;
                            if (colourListMax - songListMax < 0) highlightListTemp--;
                            else highlightListTemp = colourListMax - songListMax;
                        }
                        else
                        {
                            highlightCursourPosition--;
                            if (highlightListTemp - 1 == highlightCursourPosition) highlightListTemp--;
                        }
                    }
                }
            }
        }
///////////////////////////////////////////////////////////////////////////////////////////////////////////

        if (kdown & KEY_LEFT && inTab == false)
        {
            settingsSubMenuCounter = 0;
            inTab = true;
        }

        if (kdown & KEY_RIGHT && inTab == true)
        {
            settingsSubMenuCounter = 0;
            inTab = false;
        }

        if (kdown & KEY_DOWN && inTab == true)
        {
            if (menuTab + 1 > 2) menuTab = 0;
            else menuTab++;
        }

        if (kdown & KEY_UP && inTab == true)
        {
            if (menuTab -1 < 0) menuTab = 2;
            else menuTab--;
        }

        if (kdown & KEY_A || (kdown & KEY_TOUCH && touch.px > 1066 && touch.py > 666))
        {
            if (inTab == false && menuTab == 0)
            {
                current_song = g_CursorList;
                Mix_ResumeMusic();
                current_song = playMus(files, current_song, number_of_files);
            }
            else if (inTab == false && menuTab == 1)
            {
                switch (settingsCursorPosition)
                {
                    case 0:
                        printf("tab 0\n");
                        if (autoplay == false) autoplay = true; //TODO add red tick, and empty box for tick to go into
                        else autoplay = false;
                        settingsCursorPosition = 0;
                        break;

                    case 1:
                        printf("tab 1\n");//TODO swkbd for custom music path
                        settingsCursorPosition = 0;
                        break;

                    case 2:
                        printf("tab 2\n");//TODO chnage value of rewind / fast forward, use swkbd
                        settingsCursorPosition = 0;
                        break;

                    case 3:
                        printf("tab 3\n");//TODO display 3 themes with names underneath each theme, at the bottom, have option to load custom background.
                        if (themeSelect == 3 && themeSelect < 3) background = allThemes[themeSelect];
                        else themeSelect = 0;
                        settingsSubMenuCounter = 3;
                        break;

                    case 4:
                        printf("tab 4\n");
                        if (settingsSubMenuCounter == 4) colour = SDL_GetColour(textColourCursourPosition);
                        settingsSubMenuCounter = 4;
                        break;

                    case 5:
                        printf("tab 5\n");
                        if (settingsSubMenuCounter == 5) highlight = SDL_GetColour(highlightCursourPosition);
                        settingsSubMenuCounter = 5;
                        break;

                    case 6:
                        printf("tab 6\n"); //TODO advanced settings
                        settingsCursorPosition = 0;
                        break;

                    case 7:
                        printf("tab 7\n"); //TODO have a reset all to default config
                        settingsCursorPosition = 0;
                        break;
                    
                    default:
                        settingsCursorPosition = 0;
                        break;
                }
            }
            else
            {
                inTab = false;
                printf("meme\n");
            }
            
        }
        
        
        if (!Mix_PlayingMusic() && !Mix_PausedMusic() && (loop1 == true || autoplay == true)) // Checks if song is playing, if not, the code inside is then executed / checked.
        {
            if (loop1 == true) current_song = playMus(files, current_song, number_of_files);
            else if (autoplay == true)
            {
                if (shuffleMode == true) current_song = playMus(files, current_song, number_of_files);
                else if (current_song + 1 <= number_of_files)
                {
                    current_song++;
                    current_song = playMus(files, current_song, number_of_files);
                }
                else if (loopall == true) // Loops back to the first song
                {
                    current_song = 0;
                    current_song = playMus(files, current_song, number_of_files);
                }
            }
        }

        if (kdown & KEY_PLUS || kdown & KEY_MINUS) break;
        if (kdown & KEY_R) current_song = skipnext(files, current_song, number_of_files);
        if (kdown & KEY_L) current_song = skipback(files, current_song, number_of_files);

        if (kdown & KEY_Y) // Changes the playmode
        {
            switch(playmode)
            {
                case 0:
                    loopall = true;
                    playmode++;
                    printf("loopall\n");
                    break;
                case 1:
                    loopall = false;
                    loop1 = true;
                    playmode++;
                    printf("loop1\n");
                    break;
                case 2:
                    loop1 = false;
                    shuffleMode = true;
                    playmode++;
                    printf("shuffleMode\n");
                    break;
                default:
                    shuffleMode = false;
                    playmode = 0;
                    printf("default\n");
            }
        }

        if (kdown & KEY_B)
        {
            if (menuTab == 0) // Pauses / unpuses the song
            {
                if (!Mix_PausedMusic()) Mix_PauseMusic();
                else Mix_ResumeMusic();
            }
            else if (menuTab == 1)
            {
                inTab = false;
                settingsSubMenuCounter = 0;
                printf("B in menutab 1\n");
            }
            else if (menuTab == 2)
            {
                inTab = false;
                printf("B in menutab 2\n");
            }
        }
    }

    appExit();

    for (int i = 0; i < number_of_files; i++)
    {
        free(files[i]);
    }
    return 0;
}
