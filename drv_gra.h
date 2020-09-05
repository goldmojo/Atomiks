/*
 * Graphic driver for Atomiks
 * Copyright (C) Mateusz Viste 2014, 2015
 */

#ifndef drv_gra_h_sentinel
#define drv_gra_h_sentinel

#define GRA_FULLSCREEN 1

struct gra_sprite;

/* init the video subsystem */
int gra_init(int width, int height, int flags, char *windowtitle, unsigned char *titleicon, long titleicon_len);

/* close and clean up the graphic subsystem */
void gra_close(void);

/* clears the screen (should be used at each screen refresh iteration) */
void gra_clear(void);

/* switch the application fullscreen on/off */
void gra_switchfullscreen(void);

void gra_drawpartsprite(struct gra_sprite *sprite, int srcx, int srcy, int srcwidth, int srcheight, int dstx, int dsty);

void gra_drawsprite_alpha(struct gra_sprite *sprite, int x, int y, int alpha);

void gra_drawsprite(struct gra_sprite *sprite, int x, int y);

void gra_refresh(void);

/* loads a gziped bmp image from memory and returns a surface */
struct gra_sprite *loadgzbmp(unsigned char *memgz, long memgzlen);

void loadSpriteSheet(struct gra_sprite **sprites, int width, int height, int itemcount, void *memptr, int memlen);

int gra_getspritewidth(struct gra_sprite *sprite);

int gra_getspriteheight(struct gra_sprite *sprite);

int gra_drawrect(int x, int y, int width, int height, int r, int g, int b, int a, int fillflag);

#endif
