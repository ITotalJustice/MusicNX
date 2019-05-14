#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <mpg123.h>
#include <switch.h>
#include <math.h>


bool g_IsScroll = false, id3v2Found = false, id3v2Enable = true, shuffleMode = false, secNeedsZero = false, displayMin = false, inTab = false;
int i = 0, j = 0, g_CursorList = 0, g_MaxList = 12, songListMax = 11, g_MusListY = 150, g_MusListX = 650, g_MusListMaxY = 635, g_MusListMaxX = 1000, scrollNowplay = 60, \
    g_CursorScroll = 650, g_SongInSec = -1, g_SongInMin = 0, g_ListMoveTemp = 0, g_ListMove = 0, errorCode = 0, menuTab = 0;
static double songSecTotal, songProgressBar = 0, songProgressBarIncrease = 0;
static int min, sec;
void *keyheap;

double rewindValue = -1;
char *g_MusList[2500] = {'\0'}; //Around 10gb of 4MB .mp3 files. Size can be increased if needed, just set at this for now...
static Mix_Music *music = NULL;

static mpg123_handle *handle = NULL;
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static TTF_Font *fntTiny = NULL, *fntSmall = NULL, *fntMediumIsh = NULL, *fntMedium = NULL, *fntLarge = NULL;
SDL_Colour \
            white = {255, 255, 255}, grey = {140, 140, 140, 255}, pink = {255, 192, 203}, hotPink = {255, 105, 180}, orange = {255, 165, 0}, yellow = {255, 255, 0}, \
            gold = {255, 215, 0}, brown = {139, 69, 19}, red = {255, 0, 0}, darkRed = {139, 0, 0}, green = {0, 128, 0}, limeGreen = {50, 205, 50}, \
            aqua = {0, 255, 255}, teal = {0, 128, 128}, lightBlue = {0, 191, 255}, blue = {0, 0, 255}, darkBlue = {0, 0, 139}, purple = {160, 32, 240}, \
            indigo = {75, 0, 130}, beige = {245, 245, 220}, black = {0, 0, 0}, neonPink = { 253, 52, 131, 1 }, brightGrey = { 140, 140, 140, 1 }, colour, highlight;
SDL_Texture \
            *background = NULL, *black_background = NULL, *white_background = NULL, *ams_background = NULL, *kyon = NULL, *switch_logo = NULL, *right_arrow = NULL, \
            *a_button = NULL, *b_button = NULL, *y_button = NULL, *plus_button = NULL, *scrollbar = NULL, *ID3_tag = NULL, *white_play_button = NULL, \
            *grey_play_button = NULL, *pause_grey = NULL, *play_grey = NULL, *play_white = NULL, *pause_white = NULL, *loop1_icon = NULL, *loop_grey = NULL, \
            *loop_white = NULL, *shuffle_grey = NULL, *shuffle_white = NULL, *skip_back = NULL, *skip_forward = NULL, *rewind_icon = NULL, \
            *musicNX = NULL, *vapor = NULL, *empty_box_grey, *tick_box;

int initApp()
{
    romfsInit();
    TTF_Init();
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    Mix_Init(MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG);
    Mix_OpenAudio(48000, AUDIO_S32LSB, 2, 1024);
    Mix_VolumeMusic(64); //TODO: volume slider
    mpg123_init();
    return 0;
}

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

void textDraw(TTF_Font *font, int x, int y, SDL_Color colour, const char *text)
{
    SDL_Surface *Surface = TTF_RenderText_Blended_Wrapped(font, text, colour, 1920);
    SDL_Texture *Tex = SDL_CreateTextureFromSurface(renderer, Surface);
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = Surface ->w, pos.h = Surface->h };

    if (g_IsScroll == true && pos.w + x > 1750) { g_CursorScroll = 650; x = g_CursorScroll; }

    SDL_RenderCopy(renderer, Tex, NULL, &pos);
    SDL_DestroyTexture(Tex);
    SDL_FreeSurface(Surface);
    g_IsScroll = false;
}

void imageLoad(SDL_Texture **texture, char *path)
{
	SDL_Surface *Surface = IMG_Load(path);
    SDL_ConvertSurfaceFormat(Surface, SDL_PIXELFORMAT_RGBA8888, 0);
    *texture = SDL_CreateTextureFromSurface(renderer, Surface);
	SDL_FreeSurface(Surface);
}

void imageLoadMem(SDL_Texture **texture, void *data, int size)
{
	SDL_Surface *surface = NULL;
	surface = IMG_Load_RW(SDL_RWFromMem(data, size), 1);
    /*IMG_SavePNG(surface, g_MusList[j]);
    IMG_SavePNG(surface, "sdmc:/%s.img", g_MaxList[j]);
    IMG_SaveJPG(surface, g_MusList[j], 300)*/

	if (surface) *texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_FreeSurface(surface);
}

void imageDraw(SDL_Texture *texture, int x, int y)
{
    SDL_Rect pos = { pos.x = x, pos.y = y};
	SDL_QueryTexture(texture, NULL, NULL, &pos.w, &pos.h);
	SDL_RenderCopy(renderer, texture, NULL, &pos);
}

void imageDrawScale(SDL_Texture *texture, int x, int y, int w, int h)
{
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = w, pos.h = h};
	SDL_RenderCopy(renderer, texture, NULL, &pos);
}

void shapeDraw(SDL_Colour colour, int x, int y, int w, int h)
{
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = w, pos.h = h };
    SDL_SetRenderDrawColor(renderer, colour.r, colour.g, colour.b, colour.a);
    SDL_RenderFillRect( renderer, &pos);
}

int loadTexture()
{
    imageLoad(&black_background, "romfs:/data/black_blackground.png");
    imageLoad(&white_background, "romfs:/data/white_background.png");
    imageLoad(&ams_background, "romfs:/data/ams_background.png");
    imageLoad(&kyon, "romfs:/data/kyon.jpg");
    imageLoad(&switch_logo, "romfs:/data/switch_logo.png");
    imageLoad(&right_arrow, "romfs:/data/right_arrow.png");
    imageLoad(&a_button, "romfs:/data/a_button.jpg");
    imageLoad(&b_button, "romfs:/data/b_button.jpg");
    imageLoad(&plus_button, "romfs:/data/plus_button.jpg");
    imageLoad(&scrollbar, "romfs:/data/scrollbar.jpg");
    imageLoad(&white_play_button, "romfs:/data/white_play_button.png");
    imageLoad(&grey_play_button, "romfs:/data/grey_play_button.png");
    imageLoad(&pause_grey, "romfs:/data/pause_grey.png");
    imageLoad(&play_grey, "romfs:/data/play_grey.png");
    imageLoad(&play_white, "romfs:/data/play_white.png");
    imageLoad(&pause_white, "romfs:/data/pause_white.png");
    imageLoad(&loop1_icon, "romfs:/data/loop1_icon.png");
    imageLoad(&loop_grey, "romfs:/data/loop_grey.png");
    imageLoad(&loop_white, "romfs:/data/loop_white.png");
    imageLoad(&shuffle_grey, "romfs:/data/shuffle_grey.png");
    imageLoad(&shuffle_white, "romfs:/data/shuffle_white.png");
    imageLoad(&skip_back, "romfs:/data/skip_back.png");
    imageLoad(&skip_forward, "romfs:/data/skip_forward.png");
    imageLoad(&rewind_icon, "romfs:/data/rewind_icon.png");
    imageLoad(&musicNX, "romfs:/data/musicNX.jpg");
    imageLoad(&vapor, "romfs:/data/vapor.jpg");
    imageLoad(&empty_box_grey, "romfs:/data/empty_box_grey.png");
    imageLoad(&tick_box, "romfs:/data/tick_box.png");
    //imageLoad(&, "romfs:/data/.png");
    return 0;
}

void destroyTexture()
{
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(black_background);
    SDL_DestroyTexture(white_background);
    SDL_DestroyTexture(ams_background);
    SDL_DestroyTexture(kyon);
    SDL_DestroyTexture(switch_logo);
    SDL_DestroyTexture(right_arrow);
    SDL_DestroyTexture(a_button);
    SDL_DestroyTexture(b_button);
    SDL_DestroyTexture(plus_button);
    SDL_DestroyTexture(scrollbar);
    SDL_DestroyTexture(ID3_tag);
    SDL_DestroyTexture(grey_play_button);
    SDL_DestroyTexture(white_play_button);
    SDL_DestroyTexture(pause_grey);
    SDL_DestroyTexture(play_grey);
    SDL_DestroyTexture(play_white);
    SDL_DestroyTexture(pause_white);
    SDL_DestroyTexture(loop1_icon);
    SDL_DestroyTexture(loop_grey);
    SDL_DestroyTexture(loop_white);
    SDL_DestroyTexture(shuffle_grey);
    SDL_DestroyTexture(shuffle_white);
    SDL_DestroyTexture(skip_back);
    SDL_DestroyTexture(skip_forward);
    SDL_DestroyTexture(rewind_icon);
    SDL_DestroyTexture(musicNX);
    SDL_DestroyTexture(vapor);
    SDL_DestroyTexture(empty_box_grey);
    SDL_DestroyTexture(tick_box);
    //SDL_DestroyTexture();
}

void mp3Tag()
{
    mpg123_meta_free(handle);
    handle = mpg123_new(NULL, NULL);
    mpg123_param(handle, MPG123_ADD_FLAGS, MPG123_PICTURE, 0.0);
    mpg123_open(handle, g_MusList[j]);
    songSecTotal = mpg123_framelength(handle) * mpg123_tpf(handle);//, min = songSecTotal / 60, sec = songSecTotal % 60;
    mpg123_id3v1 *v1 = NULL;
	mpg123_id3v2 *v2 = NULL;
    mpg123_seek(handle, 0, SEEK_CUR);
    id3v2Found = false;

    if (sec < 9) secNeedsZero = true;
    else secNeedsZero = false;
    printf("\ntotal song length is %f\n", songSecTotal);
    if (secNeedsZero == true) printf("total song length in min/sec is %i:0%i\n", min, sec);
    else printf("total song length in min/sec is %i:%i\n", min, sec);

    songProgressBarIncrease = 525 / songSecTotal;
    printf("\n\nsong prog is %f\n", songProgressBarIncrease);
    
    if (mpg123_meta_check(handle) & MPG123_ID3 && mpg123_id3(handle, &v1, &v2) == MPG123_OK)
    {
		if (v1 != NULL)
        {
            printf("meta 1\n");
			//print_v1(&metadata, v1);
        }
		if (v2 != NULL && id3v2Enable == true)
        {
            printf("found idv2\n");
            for (size_t count = 0; count < v2->pictures; count++)
            {
                mpg123_picture *pic = &v2->picture[count];
                char *str = pic->mime_type.p;

                if ((pic->type == 3 ) || (pic->type == 0))
                {
                    if ((!strcasecmp(str, "image/jpg")) || (!strcasecmp(str, "image/jpeg")) || (!strcasecmp(str, "image/png")))
                    {
                        id3v2Found = true;
                        imageLoadMem(&ID3_tag, pic->data, pic->size);
                        break;
                    }
                }
            }
        }
    }
}

void playMus()
{
    if (music != NULL)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        music = NULL;
        songProgressBar = 0;
    }
    if (shuffleMode == true) j = randomizer(0, i);
    music = Mix_LoadMUS(g_MusList[j]);
    if (Mix_GetMusicType(music) == 6) mp3Tag(); 
    else id3v2Found = false;
    Mix_PlayMusic(music, 0);
    { g_SongInSec = -1; g_SongInMin = 0; rewindValue = -1; }
    printf("\nNOW PLAYING... %s\n", g_MusList[j]);
    //if (strcmp((strrchr(g_MusList[j], '.')+1), "mp3") == 0) mp3Tag(); I was so proud of this, then saw sdl has an api for checking the type...
}

void skipnext()
{
    printf("\nSKIPNEXT\n");
    if (j + 1 <= i) j++;
    else j = 0;
    playMus();
}

void skipback()
{
    printf("\nSKIPBACK\n");
    if (j - 1 >= 0) j--;
    else j = i;
    playMus();
}

void musRewind(double value)
{
    if (value > 0) Mix_SetMusicPosition(value);
    else Mix_RewindMusic();
}

void touchSong(int menuTab, int x, int y)
{
    int zero = g_ListMoveTemp, touchtest = 110, touchExt = 50, touchInc = 45, counter = 0;
    while(menuTab == 0 && counter < g_MaxList && y < g_MusListMaxY && y >= 110 && x < g_MusListMaxX)
    {
        if (x >= 430 && y >= touchtest && y <= touchtest + touchExt)
        {
            printf("value of y = %i\n", y);
            j = zero;
            g_CursorList = zero;
            g_CursorScroll = g_MusListX;
            shuffleMode = false;
            playMus();
            break;
        }
        else
        {
            touchtest += touchInc;
            zero++;
            counter++;
        }
    }
    /*while (menuTab == 0 && counter < g_MaxList && y < g_MusListMaxY && y >= 110 && x < g_MusListMaxX)
    {
        if (x >= 430 && y >= touchtest && y <= touchtest + touchExt)
        {
             = zero;
            g_CursorList = zero;
            g_CursorScroll = g_MusListX;
            break;
        }
        else
        {
            touchtest += touchInc;
            zero++;
            counter++;
        }
    }*/
    
}

void sideTabDisplay()
{
    if (menuTab == 0)
    {
        textDraw(fntMedium, 90, 280, colour, "Settings");
        textDraw(fntMedium, 90, 380, colour, "Information");
        imageDrawScale(right_arrow, 60, 190, 30, 30);
        if (inTab == false) textDraw(fntMedium, 120, 180, colour, "Music Select");
        else textDraw(fntMedium, 120, 180, highlight, "Music Select");
    }
    else if (menuTab == 1)
    {
        textDraw(fntMedium, 90, 180, colour, "Music Select");
        textDraw(fntMedium, 90, 380, colour, "Information");
        imageDrawScale(right_arrow, 60, 290, 30, 30);
        if (inTab == false) textDraw(fntMedium, 120, 280, colour, "Settings");
        else textDraw(fntMedium, 120, 280, highlight, "Settings");
    }
    else
    {
        textDraw(fntMedium, 90, 180, colour, "Music Select");
        textDraw(fntMedium, 90, 280, colour, "Settings");
        imageDrawScale(right_arrow, 60, 390, 30, 30);
        if (inTab == false) textDraw(fntMedium, 120, 380, colour, "Information");
        else textDraw(fntMedium, 120, 380, highlight, "Information");
    }
}

#define SCREEN_W 1920
#define SCREEN_H 1080 //big mistake, shouldve kept it as 720p :(

int main(int argc, char **argv)
{
    socketInitializeDefault(); //DEBUG
    nxlinkStdio();
    printf("Testing...\n\nTesting...\n\nCan you see me?........\n\n");

    if (randomizerInit() != 0) printf("ERROR: randomizer failed!\n");

    char *location = {'\0'};

    FILE* conf = fopen("sdmc:/music.ini", "a+");
    if (conf == NULL) location = "sdmc:/music/";
    else
    {
        long length = 0;
		fseek (conf, 0L, SEEK_END);
		length = ftell (conf);
		fseek (conf, 0, SEEK_SET);
		location = calloc(1, length+1);
		if (location) fread (location, 1, length, conf);
        printf("the location in file is %s\n", location);
    }
    fclose(conf);

    if (chdir(location) != 0) 
    {
        printf("ERROR 1: FAILED TO OPEN DIRECTORY FROM FILE...\n");
        if (chdir("sdcm:/music/") != 0)
        {
            printf("ERROR 2: FAILED TO OPEN DEFAULT DIRECTORY 'sdmc:/music'...");
            goto errorExit;
        }
    }

	struct dirent *de;
	DIR *dr = opendir(location);

    while ((de = readdir(dr)) != NULL && i < 2500)
    {
        if (strstr(de->d_name, ".mp3") != NULL || strstr(de->d_name, ".MP3") != NULL || strstr(de->d_name, ".ogg") != NULL || strstr(de->d_name, ".OGG") != NULL \
            || strstr(de->d_name, ".wav") != NULL || strstr(de->d_name, ".WAV") != NULL || strstr(de->d_name, ".MOD") != NULL || strstr(de->d_name, ".flac") != NULL)
        {
            size_t size = strlen(de->d_name) + 1;
            g_MusList[i] = malloc (size);
            strlcpy(g_MusList[i], de->d_name, size);
            i++;
        }
    }
    i--;
    closedir(dr);
    free(location);

    if (initApp() != 0) printf("\nERROR: initApp failed!\n");

    window = SDL_CreateWindow("totaljustice", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);


    fntTiny = TTF_OpenFont("romfs:/font/NintendoStandard.ttf", 28);
    fntSmall = TTF_OpenFont("romfs:/font/NintendoStandard.ttf", 36);
    fntMediumIsh = TTF_OpenFont("romfs:/font/NintendoStandard.ttf", 40);
    fntMedium = TTF_OpenFont("romfs:/font/NintendoStandard.ttf", 48);
    fntLarge = TTF_OpenFont("romfs:/font/NintendoStandard.ttf", 72);

    if (loadTexture() != 0) printf("ERROR: Texture load failed!\n");

    SDL_Colour allColour[] = {white, grey, black, pink, hotPink, orange, yellow, gold, brown, red, darkRed, green, limeGreen, aqua, teal, lightBlue, blue, darkBlue, purple, indigo, beige};
    SDL_Texture *allThemes[] = {black_background, white_background, ams_background, vapor};
    background = allThemes[0];
    colour = allColour[0], highlight = allColour[1];
    char *allColourString[22] = {"white", "grey", "black", "pink", "hotPink", "orange", "yellow", "gold", "brown", "red", "darkRed", "green", "limeGreen", "aqua", "teal", "lightBlue", "blue", "darkBlue", "purple", "indigo", "beige", '\0'};
    char *menuTab1Options[] = {"Autoplay:", "Set Custom Music Path:", "Set Rewind / Fast Forward Value:", "Set Theme / Background:", "Set Text Colour:", "Set Highlight Colour:", "Experimental Settings:", "Reset All To default:"};

    bool loop1 = false, loopall = false, autoplay = true;
    float scrollList = 0, scrollListSpeedDefault = 5, scrollListSpeedIncrease = scrollListSpeedDefault;
    int fpsCounter = 0, secTemp = 61, touchHeldTime = 0, touchCheck = 0, touchTempx = 0, touchTempY = 0, playmode = 0;
    u32 touchCount = 0;

    int colourListMax = 20, settingsSubMenuCounter = 0, textColourCursourPosition = 0, highlightCursourPosition = 0, textColourListTemp = 0, highlightListTemp = 0;

    int settingsListTemp = 0, settingsListMax = 8, settingsCursorPosition = 0;

    int themeSelect = 0;

    if (autoplay == true) playMus();

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

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, background, NULL, NULL);

        if (id3v2Found == true) imageDrawScale(ID3_tag, 60, 515, 300, 300);
        else imageDrawScale(musicNX, 60, 515, 300, 300);

        if (Mix_PlayingMusic() && !Mix_PausedMusic()) imageDraw(pause_white, 280, 870);
        else imageDraw(play_white, 290, 870);
        imageDraw(skip_back, 60, 870);
        imageDraw(skip_forward, 480, 870);

        //Pause / play
        if (kdown & KEY_TOUCH && touch.px > 186 && touch.px < 256 && touch.py > 580 && touch.py < 640)
        {
            if (!Mix_PausedMusic()) Mix_PauseMusic();
            else Mix_ResumeMusic();
        }

        //skipback
        if (kdown & KEY_TOUCH && touch.px > 40 && touch.px < 110 && touch.py > 580 && touch.py < 640) skipback();

        //skipforward
        if (kdown & KEY_TOUCH && touch.px > 320 && touch.px < 390 && touch.py > 580 && touch.py < 640) skipnext();

        if (loopall == false && loop1 == false) imageDraw(loop_white, 380, 700);
        else if (loopall == true) imageDraw(loop_grey, 380, 700);
        else imageDraw(loop1_icon, 380, 700);

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

        if (shuffleMode == false) imageDraw(shuffle_white, 490, 700);
        else imageDraw(shuffle_grey, 490, 700);

        //shuffle on / off
        if (kdown & KEY_TOUCH && touch.px > 315 && touch.px < 410 && touch.py > 466 && touch.py < 530)
        {
            loopall = false;
            loop1 = false;
            if (shuffleMode == false) shuffleMode = true;
            else shuffleMode = false;
        }

        imageDrawScale(kyon, 60, 30, 160, 90);
        imageDrawScale(b_button, 1450, 1000, 50, 50);
        if (menuTab == 0) textDraw(fntSmall, 1510, 1010, colour, "Pause");
        else textDraw(fntSmall, 1510, 1010, colour, "Back");
        imageDrawScale(a_button, 1650, 1000, 50, 50);
        textDraw(fntSmall, 1710, 1010, colour, "Select");
        textDraw(fntMedium, scrollNowplay, g_MusListMaxX, colour, g_MusList[j]); //Now playing

        shapeDraw(brightGrey, 55, 840, 525, 15);
        shapeDraw(neonPink, 55, 840, songProgressBar, 15);

        sideTabDisplay(); // Displays the left side tab.

        if (menuTab == 0 && settingsSubMenuCounter == 0)
        {
            for (int temp = 0, tempMove = g_ListMoveTemp; temp < g_MaxList; temp++, tempMove++)
            {
                if (g_CursorList == tempMove && inTab == false) // Checks what the selected song is, then changes the colour to highlight, and x becomes the scrolling variable.
                {
                    g_IsScroll = true;
                    textDraw(fntSmall, g_CursorScroll, g_MusListY, highlight, g_MusList[tempMove]);
                }
                else textDraw(fntSmall, g_MusListX, g_MusListY, colour, g_MusList[tempMove]);
                g_MusListY += 70;
            }
        }

        else if (menuTab == 1)
        {
            if (menuTab == 1 && settingsSubMenuCounter == 0)
            {
                if (autoplay == true) imageDrawScale(tick_box, 880, 150, 60, 60);
                else imageDrawScale(empty_box_grey, 880, 150, 60, 60);

                for (int temp = 0, tempMove = settingsListTemp; temp < settingsListMax; temp++, tempMove++)
                {
                    if (settingsCursorPosition == tempMove && inTab == false) textDraw(fntMediumIsh, g_MusListX, g_MusListY, highlight, menuTab1Options[tempMove]);
                    else textDraw(fntMediumIsh, g_MusListX, g_MusListY, colour, menuTab1Options[tempMove]);
                    g_MusListY += 100;
                }
            }
            else if (settingsSubMenuCounter == 3)
            {
                imageDrawScale(allThemes[0], 650, 150, 180, 135);
                textDraw(fntMediumIsh, 900, 200, colour, "Black Theme");
                imageDrawScale(allThemes[1], 650, 325, 180, 135);
                textDraw(fntMediumIsh, 900, 375, colour, "White Theme");
                imageDrawScale(allThemes[2], 650, 500, 180, 135);
                textDraw(fntMediumIsh, 900, 550, colour, "Atmosphere Theme");
                imageDrawScale(allThemes[3], 650, 675, 180, 135);
                textDraw(fntMediumIsh, 900, 725, colour, "Vaporwave Theme");
                textDraw(fntMedium, 650, 900, colour, "Load Custom Theme...");
            }

            else if (settingsSubMenuCounter == 4) //Text Colour Menu
            {
                for (int temp = 0, tempMove = textColourListTemp; temp < g_MaxList; temp++, tempMove++)
                {
                    if (textColourCursourPosition == tempMove && inTab == false)
                    {
                        imageDrawScale(right_arrow, 650, g_MusListY, 30, 30);
                        textDraw(fntSmall, 700, g_MusListY, allColour[tempMove], allColourString[tempMove]);
                    }
                    else textDraw(fntSmall, g_MusListX, g_MusListY, allColour[tempMove], allColourString[tempMove]);
                    g_MusListY += 70;
                }
            }

            else if (settingsSubMenuCounter == 5) //Highlight Colour Menu
            {
                for (int temp = 0, tempMove = highlightListTemp; temp < g_MaxList; temp++, tempMove++)
                {
                    if (highlightCursourPosition == tempMove && inTab == false)
                    {
                        imageDrawScale(right_arrow, 650, g_MusListY, 30, 30);
                        textDraw(fntSmall, 700, g_MusListY, allColour[tempMove], allColourString[tempMove]);
                    }
                    else textDraw(fntSmall, g_MusListX, g_MusListY, allColour[tempMove], allColourString[tempMove]);
                    g_MusListY += 70;
                }
            }
        }

        if (menuTab == 2)
        {
            textDraw(fntLarge, 650, 150, colour, "BETA:");

            textDraw(fntMediumIsh, 650, 350, colour, "..................");
            textDraw(fntMediumIsh, 650, 450, colour, "Oh hey!");
            textDraw(fntMediumIsh, 650, 550, colour, "So.........");
            textDraw(fntMediumIsh, 650, 650, colour, "What do you think?");
        }
        SDL_RenderPresent(renderer);
        g_MusListY = 150;
        g_CursorScroll++;

/*#####################################################################################################################################*/

        if (kdown & KEY_X)
        {
            if (background == black_background) background = allThemes[3];
            else background = allThemes[0];
        }

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
                else skipnext();
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
                printf("time is %i:%i\n", g_SongInMin, g_SongInSec);
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
                touchSong(menuTab, touchTempx, touchTempY);
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
                if (g_CursorList + 1 > i)
                {
                    g_CursorList = 0;
                    g_ListMoveTemp = 0;
                    printf("loop to the top g_CursorList = %i = %s\n", g_CursorList, g_MusList[g_CursorList]);
                }
                else
                {
                    g_CursorList++;
                    if (g_ListMoveTemp + g_MaxList == g_CursorList) g_ListMoveTemp++;
                    printf("move down g_CursorList = %i = %s\n", g_CursorList, g_MusList[g_CursorList]);
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
                    g_CursorList = i;
                    if (i - songListMax < 0) g_ListMoveTemp = 0;
                    else g_ListMoveTemp = i - songListMax;
                    printf("loop to the bottom g_CursorList = %i = %s\n", g_CursorList, g_MusList[g_CursorList]);
                }
                else
                {
                    g_CursorList--;
                    if (g_ListMoveTemp - 1 == g_CursorList) g_ListMoveTemp--;
                    printf("move up g_CursorList = %i = %s\n", g_CursorList, g_MusList[g_CursorList]);
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
                    if (g_CursorList + 1 > i)
                    {
                        g_CursorList = 0;
                        g_ListMoveTemp = 0;
                        printf("loop to the top g_CursorList = %i = %s\n", g_CursorList, g_MusList[g_CursorList]);
                    }
                    else
                    {
                        g_CursorList++;
                        if (g_ListMoveTemp + g_MaxList == g_CursorList) g_ListMoveTemp++;
                        printf("move down g_CursorList = %i = %s\n", g_CursorList, g_MusList[g_CursorList]);
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
                        g_CursorList = i;
                        if (i - songListMax < 0) g_ListMoveTemp = 0;
                        else g_ListMoveTemp = i - songListMax;
                        printf("loop to the bottom g_CursorList = %i = %s\n", g_CursorList, g_MusList[g_CursorList]);
                    }
                    else
                    {
                        g_CursorList--;
                        if (g_ListMoveTemp - 1 == g_CursorList) g_ListMoveTemp--;
                        printf("move up g_CursorList = %i = %s\n", g_CursorList, g_MusList[g_CursorList]);
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
                j = g_CursorList;
                Mix_ResumeMusic();
                playMus();
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
                        if (settingsSubMenuCounter == 4) colour = allColour[textColourCursourPosition];
                        settingsSubMenuCounter = 4;
                        break;

                    case 5:
                        printf("tab 5\n");
                        if (settingsSubMenuCounter == 5) highlight = allColour[highlightCursourPosition];
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
            if (loop1 == true) playMus();
            else if (autoplay == true)
            {
                if (shuffleMode == true) playMus();
                else if (j + 1 <= i)
                {
                    j++;
                    playMus();
                }
                else if (loopall == true) // Loops back to the first song
                {
                    j = 0;
                    playMus();
                }
            }
        }

        if (kdown & KEY_PLUS || kdown & KEY_MINUS) break;
        if (kdown & KEY_R) skipnext();
        if (kdown & KEY_L) skipback();

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

    Mix_HaltChannel(-1);
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    destroyTexture();
    SDL_Quit();
    TTF_Quit();
    IMG_Quit();
    Mix_Quit();
    romfsExit();
    mpg123_exit();

    errorExit:
    socketExit();
    for (i = 0; i <= 2500; i++) free(g_MusList[i]);
    if (errorCode > 0) consoleExit(NULL);
    return 0;
}