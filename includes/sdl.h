/* I wrote this header to be as portable as possible for other homebrew switch projects */
/* The only thing that needs to be changed will be the name / number of textures you want to load */
/* If you decide to use this header and add functions, please consider opening a pr for said functions */
/* I would greatly appreaciate it :) */

#ifndef _SDL_MEMES_H_
#define _SDL_MEMES_H_

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define SCREEN_W    1920                                                            //width of the window
#define SCREEN_H    1080                                                            //hight of the window

#define white       0
#define grey        1
#define dark_grey   2
#define black       3
#define pink        4
#define neon_pink   5
#define hot_pink    6
#define orange      7
#define yellow      8
#define gold        9
#define brown       10
#define red         11
#define dark_red    12
#define green       13
#define lime_green  14
#define aqua        15
#define teal        16
#define light_blue  17
#define blue        18
#define jordy_blue  19
#define faint_blue  20
#define dark_blue   21
#define purple      22
#define indigo      23
#define beige       24

// buttons from NintendoExt
#define BUTTON_A            0xE0E0
#define BUTTON_B            0xE0E1
#define BUTTON_X            0xE0E2
#define BUTTON_Y            0xE0E3
#define BUTTON_L            0xE0E4
#define BUTTON_R            0xE0E5
#define BUTTON_ZL           0xE0E6
#define BUTTON_ZR           0xE0E7
#define BUTTON_SL           0xE0E8
#define BUTTON_SR           0xE0E9
#define BUTTON_UP           0xE0EB
#define BUTTON_DOWN         0xE0EC
#define BUTTON_LEFT         0xE0ED
#define BUTTON_RIGHT        0xE0EE
#define BUTTON_PLUS         0xE0EF
#define BUTTON_MINUS        0xE0F0
#define BUTTON_HOME         0xE0F4
#define BUTTON_SCREENSSHOT  0xE0F5
#define BUTTON_LS           0xE101
#define BUTTON_RS           0xE102
#define BUTTON_L3           0xE104
#define BUTTON_R3           0xE105

TTF_Font *fntTiny, *fntSmall, *fntMediumIsh, *fntMedium, *fntLarge, *fntButton, *fntButtonBig;
SDL_Texture *background, *black_background, *white_background, *switch_logo, *right_arrow, \
            *a_button, *b_button, *y_button, *plus_button, *scrollbar, *ID3_tag, *white_play_button, \
            *grey_play_button, *pause_grey, *play_grey, *play_white, *pause_white, *loop1_icon, *loop_grey, \
            *loop_white, *shuffle_grey, *shuffle_white, *skip_back, *skip_forward, *rewind_icon, \
            *musicNX, *empty_box_grey, *tick_box;


SDL_Colour SDL_GetColour(int colour_option);                                        //pass the name of colour, returns the colour
SDL_Window* SDL_GetWindow(void);                                                    //get sdl window

void clearRenderer(void);                                                           //clear the screen
void updateRenderer(void);                                                          //update the screen

void imageLoad(SDL_Texture **texture, char *path);                                  //load image from texture
void imageLoadMem(SDL_Texture **texture, void *data, int size);                     //load image from memory

void drawText(TTF_Font *font, int x, int y, SDL_Color colour, const char *text);    //draw text to screen
void drawButton(TTF_Font *font, u_int16_t btn, int x, int y, SDL_Colour colour);    //draw button to screen
void drawImage(SDL_Texture *texture, int x, int y);                                 //draw image to screen from texture
void drawImageScale(SDL_Texture *texture, int x, int y, int w, int h);              //scale the image drawn to screen
void drawShape(SDL_Colour colour, int x, int y, int w, int h);                      //draw shap (rect)

void loadFonts(void);                                                               //load all fonts
void loadTextures(void);                                                            //load all textures
void destroyTextures(void);                                                         //destroy all textures

void sdlInit();                                                                     //init all sdl stuff
void sdlExit();                                                                     //clean and exit

#endif