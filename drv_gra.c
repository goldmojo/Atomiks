/*
 * Graphic driver for Atomiks, using SDL as backend
 * Copyright (C) Mateusz Viste 2014, 2015
 */


#include <stdio.h>    /* puts(), printf() */
#include <stdlib.h>   /* malloc(), free() */
#include <SDL2/SDL.h>

#include "drv_gra.h" /* include self for control */

#include "gz.h"

#ifdef __GCW0__
#define SCALE 1
#else
#define SCALE 2
#endif

struct gra_sprite {
  SDL_Texture *ptr;
  int w;
  int h;
};

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;


/* loads a gziped bmp image from memory into a SDL surface */
static SDL_Surface *loadgzbmp_surface(unsigned char *memgz, long memgzlen) {
  unsigned char *rawimage;
  long rawimagelen;
  SDL_Surface *res;
  SDL_RWops *rwop;
  if (isGz(memgz, memgzlen) == 0) return(NULL);
  rawimage = ungz(memgz, memgzlen, &rawimagelen);
  rwop = SDL_RWFromMem(rawimage, rawimagelen);
  res = SDL_LoadBMP_RW(rwop, 0);
  SDL_FreeRW(rwop);
  free(rawimage);
  return(res);
}


/* init the video subsystem */
int gra_init(int width, int height, int flags, char *windowtitle, unsigned char *titleicon, long titleicon_len) {
  int sdl_video_flags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
  SDL_Surface *titleiconsurface;

  if (flags & GRA_FULLSCREEN) sdl_video_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
  window = SDL_CreateWindow(windowtitle, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, sdl_video_flags);
  renderer = SDL_CreateRenderer(window, -1, 0);
  if (renderer == NULL) {
    SDL_DestroyWindow(window);
    printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
    return(1);
  }

  /* set the window's titlebar icon */
  if ((titleicon != NULL) && (titleicon_len > 0)) {
    titleiconsurface = loadgzbmp_surface(titleicon, titleicon_len);
    if (titleicon != NULL) {
      SDL_SetWindowIcon(window, titleiconsurface);
      SDL_FreeSurface(titleiconsurface);
    }
  }

  /* set the scaling method */
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

  /* set the logical screen size */
  SDL_RenderSetLogicalSize(renderer, width, height);

  SDL_ShowCursor(SDL_DISABLE);               /* Hide the mouse cursor */
  return(0);
}


/* close and clean up the graphic subsystem */
void gra_close(void) {
  SDL_DestroyWindow(window);
  SDL_Quit();
}


void gra_clear(void) {
  SDL_RenderClear(renderer);
}


void gra_switchfullscreen(void) {
  static int fullscreenflag = 0;
  fullscreenflag ^= 1;
  if (fullscreenflag != 0) {
      SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
      SDL_SetWindowFullscreen(window, 0);
  }
  SDL_Delay(50); /* wait for 50ms - the video thread needs some time to set things up */
}


void gra_drawpartsprite(struct gra_sprite *sprite, int srcx, int srcy, int srcwidth, int srcheight, int dstx, int dsty) {
  static SDL_Rect srcrect;
  static SDL_Rect dstrect;
  dstrect.x = dstx * SCALE;
  dstrect.y = dsty * SCALE;
  dstrect.w = srcwidth * SCALE;
  dstrect.h = srcheight * SCALE;
  srcrect.x = srcx;
  srcrect.y = srcy;
  srcrect.w = srcwidth;
  srcrect.h = srcheight;
  SDL_RenderCopy(renderer, sprite->ptr, &srcrect, &dstrect);
}


void gra_drawsprite_alpha(struct gra_sprite *sprite, int x, int y, int alpha) {
  static SDL_Rect rect;
  rect.x = x * SCALE;
  alpha = alpha; /* TODO */
  rect.y = y * SCALE;
  rect.w = sprite->w * SCALE;
  rect.h = sprite->h * SCALE;
  /* if (alpha < 255) SDL_SetAlpha(sprite->ptr, SDL_SRCALPHA, alpha); TODO */
  /* SDL_BlitSurface(sprite->ptr, NULL, screen, &rect); */
  SDL_RenderCopy(renderer, sprite->ptr, NULL, &rect);
  /* if (alpha < 255) SDL_SetAlpha(sprite->ptr, SDL_SRCALPHA, 255); TODO */
}


void gra_drawsprite(struct gra_sprite *sprite, int x, int y) {
  gra_drawsprite_alpha(sprite, x, y, 255);
}


void gra_refresh(void) {
  SDL_RenderPresent(renderer);
}


/* loads a gziped bmp image from memory and returns a gra_sprite */
struct gra_sprite *loadgzbmp(unsigned char *memgz, long memgzlen) {
  SDL_Surface *surface;
  struct gra_sprite *res;
  surface = loadgzbmp_surface(memgz, memgzlen);
  if (surface == NULL) return(NULL);
  res = malloc(sizeof(struct gra_sprite));
  res->ptr = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  SDL_QueryTexture(res->ptr, NULL, NULL, &(res->w), &(res->h));
  return(res);
}


void loadSpriteSheet(struct gra_sprite **sprites, int width, int height, int itemcount, void *memptr, int memlen) {
  SDL_Surface *spritesheet, *item;
  SDL_Rect rect;
  int i;
  spritesheet = loadgzbmp_surface(memptr, memlen);
  if (spritesheet == NULL) puts("bmp is NULL!!!");
  item = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, 0xFF000000L, 0x00FF0000L, 0x0000FF00L, 0x000000FFL);
  if (item == NULL) puts("item is NULL!!!");
  for (i = 0; i < itemcount; i++) {
    rect.x = i * width;
    rect.y = 0;
    rect.w = width;
    rect.h = height;
    SDL_FillRect(item, NULL, 0);
    SDL_BlitSurface(spritesheet, &rect, item, NULL);
    sprites[i] = malloc(sizeof(struct gra_sprite));
    sprites[i]->ptr = SDL_CreateTextureFromSurface(renderer, item);
    if (sprites[i]->ptr == NULL) puts("sprites[i]->ptr is NULL!!!");
    SDL_QueryTexture(sprites[i]->ptr, NULL, NULL, &(sprites[i]->w), &(sprites[i]->h));
  }
  SDL_FreeSurface(item);
  SDL_FreeSurface(spritesheet);
}


int gra_getspritewidth(struct gra_sprite *sprite) {
  return(sprite->w);
}


int gra_getspriteheight(struct gra_sprite *sprite) {
  return(sprite->h);
}


int gra_drawrect(int x, int y, int width, int height, int r, int g, int b, int a, int fillflag) {
  SDL_Rect rect;
  int res;
  rect.x = x;
  rect.y = y;
  rect.w = width;
  rect.h = height;
  res = SDL_SetRenderDrawColor(renderer, r, g, b, a);
  if (res != 0) return(-1);
  if (fillflag == 0) {
      res = SDL_RenderDrawRect(renderer, &rect);
    } else {
      res = SDL_RenderFillRect(renderer, &rect);
  }
  if (res != 0) return(-2);
  return(0);
}
