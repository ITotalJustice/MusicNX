/* I wrote this header to be as portable as possible for other homebrew switch projects */
/* The only thing that needs to be changed will be the name / number of textures you want to load */
/* If you decide to use this header and add functions, please consider opening a pr for said functions */
/* I would greatly appreaciate it :) */

#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_image.h>
#include <switch.h>

#include "sdl.h"

#define sdl_default_bitrate 48000

static SDL_Window *main_window;
static SDL_Renderer *main_renderer;

SDL_Colour colours[] = {
    { 255, 255, 255 },          //white
    { 140, 140, 140, 255 },     //grey
    { 81, 81, 81 },             //dark grey
    { 0, 0, 0 },                //black
    { 255, 192, 203 },          //pink
    { 253, 52, 131, 1 },        //neon pink
    { 255, 105, 180 },          //hotpink
    { 255, 165, 0 },            //orange
    { 255, 255, 0 },            //yellow
    { 255, 215, 0 },            //gold
    { 139, 69, 19 },            //brown
    { 255, 0, 0 },              //red
    { 139, 0, 0 },              //dark red
    { 0, 128, 0 },              //green
    { 50, 205, 50 },            //lime green
    { 0, 255, 255 },            //aqua
    { 0, 128, 128 },            //teal
    { 0, 191, 255 },            //light blue
    { 0, 0, 255 },              //blue
    { 131, 177, 218 },          //jordy blue
    { 97, 115, 255 },           //faint blue
    { 28, 33, 73 },             //dark blue
    { 160, 32, 240 },           //purple
    { 75, 0, 130 },             //indigo
    { 245, 245, 220 },          //beige
    };

SDL_Colour SDL_GetColour(int colour_option)
{
    return colours[colour_option];
}

SDL_Window *SDL_GetWindow(void)
{
    return main_window;
}

void clearRenderer()
{
    SDL_RenderClear(SDL_GetRenderer(SDL_GetWindow()));
    SDL_RenderCopy(main_renderer, black_background, NULL, NULL);
}

void updateRenderer()
{
    SDL_RenderPresent(main_renderer);
}

void imageLoad(SDL_Texture **texture, char *path)
{
	SDL_Surface *Surface = IMG_Load(path);
    SDL_ConvertSurfaceFormat(Surface, SDL_PIXELFORMAT_RGBA8888, 0);
    *texture = SDL_CreateTextureFromSurface(main_renderer, Surface);
	SDL_FreeSurface(Surface);
}

void imageLoadMem(SDL_Texture **texture, void *data, int size)
{
	SDL_Surface *surface = IMG_Load_RW(SDL_RWFromMem(data, size), 1);
	if (surface) *texture = SDL_CreateTextureFromSurface(main_renderer, surface);
	SDL_FreeSurface(surface);
}

void drawText(TTF_Font *font, int x, int y, SDL_Color colour, const char *text)
{
    SDL_Surface *Surface = TTF_RenderText_Blended_Wrapped(font, text, colour, 1280);
    SDL_Texture *Tex = SDL_CreateTextureFromSurface(main_renderer, Surface);
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = Surface ->w, pos.h = Surface->h };

    SDL_RenderCopy(main_renderer, Tex, NULL, &pos);
    SDL_DestroyTexture(Tex);
    SDL_FreeSurface(Surface);
}

void drawButton(TTF_Font *font, u_int16_t btn, int x, int y, SDL_Colour colour)
{
    SDL_Surface *Surface = TTF_RenderGlyph_Blended(font, btn, colour);
    SDL_Texture *Tex = SDL_CreateTextureFromSurface(main_renderer, Surface);
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = Surface ->w, pos.h = Surface->h };

    SDL_RenderCopy(main_renderer, Tex, NULL, &pos);
    SDL_DestroyTexture(Tex);
    SDL_FreeSurface(Surface);
}

void drawImage(SDL_Texture *texture, int x, int y)
{
    SDL_Rect pos = { pos.x = x, pos.y = y };
	SDL_QueryTexture(texture, NULL, NULL, &pos.w, &pos.h);
	SDL_RenderCopy(main_renderer, texture, NULL, &pos);
}

void drawImageScale(SDL_Texture *texture, int x, int y, int w, int h)
{
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = w, pos.h = h };
	SDL_RenderCopy(main_renderer, texture, NULL, &pos);
}

void drawShape(SDL_Colour colour, int x, int y, int w, int h)
{
    SDL_Rect pos = { pos.x = x, pos.y = y, pos.w = w, pos.h = h };
    SDL_SetRenderDrawColor(main_renderer, colour.r, colour.g, colour.b, colour.a);
    SDL_RenderFillRect(main_renderer, &pos);
}

void loadFonts()
{
    // Get the fonts from system
    PlFontData font;
    PlFontData button_data;
    plGetSharedFontByType(&font, PlSharedFontType_Standard);
    plGetSharedFontByType(&button_data, PlSharedFontType_NintendoExt);

    fntTiny         = TTF_OpenFontRW(SDL_RWFromMem(font.address, font.size), 1, 28);
    fntSmall        = TTF_OpenFontRW(SDL_RWFromMem(font.address, font.size), 1, 36);
    fntMediumIsh    = TTF_OpenFontRW(SDL_RWFromMem(font.address, font.size), 1, 40);
    fntMedium       = TTF_OpenFontRW(SDL_RWFromMem(font.address, font.size), 1, 48);
    fntLarge        = TTF_OpenFontRW(SDL_RWFromMem(font.address, font.size), 1, 72);
    fntButton       = TTF_OpenFontRW(SDL_RWFromMem(button_data.address, button_data.size), 1, 30);
    fntButtonBig    = TTF_OpenFontRW(SDL_RWFromMem(button_data.address, button_data.size), 1, 36);
}

void loadTextures()
{
    imageLoad(&black_background,    "romfs:/data/black_blackground.png");
    imageLoad(&white_background,    "romfs:/data/white_background.png");
    imageLoad(&switch_logo,         "romfs:/data/switch_logo.png");
    imageLoad(&right_arrow,         "romfs:/data/right_arrow.png");
    imageLoad(&a_button,            "romfs:/data/a_button.jpg");
    imageLoad(&b_button,            "romfs:/data/b_button.jpg");
    imageLoad(&plus_button,         "romfs:/data/plus_button.jpg");
    imageLoad(&scrollbar,           "romfs:/data/scrollbar.jpg");
    imageLoad(&white_play_button,   "romfs:/data/white_play_button.png");
    imageLoad(&grey_play_button,    "romfs:/data/grey_play_button.png");
    imageLoad(&pause_grey,          "romfs:/data/pause_grey.png");
    imageLoad(&play_grey,           "romfs:/data/play_grey.png");
    imageLoad(&play_white,          "romfs:/data/play_white.png");
    imageLoad(&pause_white,         "romfs:/data/pause_white.png");
    imageLoad(&loop1_icon,          "romfs:/data/loop1_icon.png");
    imageLoad(&loop_grey,           "romfs:/data/loop_grey.png");
    imageLoad(&loop_white,          "romfs:/data/loop_white.png");
    imageLoad(&shuffle_grey,        "romfs:/data/shuffle_grey.png");
    imageLoad(&shuffle_white,       "romfs:/data/shuffle_white.png");
    imageLoad(&skip_back,           "romfs:/data/skip_back.png");
    imageLoad(&skip_forward,        "romfs:/data/skip_forward.png");
    imageLoad(&rewind_icon,         "romfs:/data/rewind_icon.png");
    imageLoad(&musicNX,             "romfs:/data/musicNX.jpg");
    imageLoad(&empty_box_grey,      "romfs:/data/empty_box_grey.png");
    imageLoad(&tick_box,            "romfs:/data/tick_box.png");
}

void destroyTextures()
{
    TTF_CloseFont(fntSmall);
    TTF_CloseFont(fntMedium);
    TTF_CloseFont(fntLarge);
    TTF_CloseFont(fntButton);
    TTF_CloseFont(fntButtonBig);

    SDL_DestroyTexture(background);
    SDL_DestroyTexture(black_background);
    SDL_DestroyTexture(white_background);
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
    SDL_DestroyTexture(empty_box_grey);
    SDL_DestroyTexture(tick_box);
}

void sdlInit()
{
    TTF_Init();
    SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    Mix_Init(MIX_INIT_MOD | MIX_INIT_MP3 | MIX_INIT_OGG | MIX_INIT_FLAC);
    Mix_OpenAudio(sdl_default_bitrate, AUDIO_S32LSB, 2, 1024);
    Mix_VolumeMusic(64); //TODO: volume slider

    main_window = SDL_CreateWindow("totaljustice", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_W, SCREEN_H, SDL_WINDOW_SHOWN);
    main_renderer = SDL_CreateRenderer(main_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    
    // highest quality
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "2");

    loadFonts();
    loadTextures();
}

void sdlExit()
{
    destroyTextures();
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}