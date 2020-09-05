/*
 * Atomiks -- a modern version of the 1990 Atomix logic game
 * Copyright (C) 2013,2014,2015 Mateusz Viste
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "atomcore.h"
#include "data.h"
#include "gz.h"
#include "cfg.h"

#include "drv_inp.h"  /* input driver */
#include "drv_gra.h"  /* graphic driver */
#include "drv_snd.h"  /* sound driver */
#include "drv_tim.h"  /* timer driver */


#define PVER "v1.0.4.1"

#define TILESIZE 16  /* the TILESIZE is the elementary unit of measurement */
                     /* in the game. */


struct spritesstruct {
  struct gra_sprite *atom[49];
  struct gra_sprite *satom[49];
  struct gra_sprite *wall[19];
  struct gra_sprite *explosion[8];
  struct gra_sprite *empty;
  struct gra_sprite *preview[2];
  struct gra_sprite *bg[3];
  struct gra_sprite *completed;
  struct gra_sprite *black;
  struct gra_sprite *cursor[4];
  struct gra_sprite *font1[37];
  struct gra_sprite *font2[11];
  struct gra_sprite *font3[26];
};


struct soundsstruct {
  struct snd_wav *selected;
  struct snd_wav *bzzz;
  struct snd_wav *explode;
  int soundflag;
};


struct loosetile_t {
  int atom;
  int x;
  int y;
  int origpos_x;
  int origpos_y;
};


static enum atomiks_keys pollkey(void) {
  return(inp_waitkey(-1));
}


/* waits for a keypress up to timeout miliseconds. returns 1 if an 'exit'
 * action is requested, 0 otherwise.
 * If timeout is negative, then only polling is performed. If timeout is 0,
 * then the function waits forever.
 * The gotkey pointer, if not NULL, is set with the key that have been
 * received. */
static int waitforanykey(int timeout, enum atomiks_keys *gotkey) {
  int nloop = 0;
  enum atomiks_keys keyevent;
  for (;;) {
    gra_refresh();
    keyevent = inp_waitkey(200);
    if (gotkey != NULL) *gotkey = keyevent;
    switch (keyevent) {
      case atomiks_fullscreen:
        gra_switchfullscreen();
        gra_refresh();
        break;
      case atomiks_esc:
      case atomiks_quit:
        return(1);
      case atomiks_none:
        nloop += 200;
        if ((nloop >= timeout) && (timeout > 0)) return(0);
        break;
      case atomiks_gotfocus:
      case atomiks_lostfocus:
        break;
      default:
        return(0);
    }
  }
}


static void draw_playfield_tile(struct atomixgame *game, int x, int y, struct spritesstruct *sprites, struct gra_sprite *tile) {
  if (tile == NULL) {
    if ((game->field[x][y] & field_type) == field_atom) {
        tile = sprites->atom[game->field[x][y] & field_index];
      } else if ((game->field[x][y] & field_type) == field_wall) {
        tile = sprites->wall[game->field[x][y] & field_index];
      } else if ((game->field[x][y] & field_type) == field_free) {
        tile = sprites->empty;
      } else { /* empty space */
        tile = NULL;
    }
  }
  if (tile != NULL) {
    int xx, yy;
    xx = game->offseth + (x * TILESIZE);
    yy = game->offsetv + (y * TILESIZE);
    gra_drawsprite(sprites->empty, xx, yy);
    gra_drawsprite(tile, xx, yy);
  }
}


static void draw_anim_explosions(struct atomixgame *game, struct spritesstruct *sprites, struct soundsstruct *sounds) {
  int listlen = 0;
  int listx[64];
  int listy[64];
  int i, x, y;
  /* compute the list of atoms on the playfield */
  for (y = 0; y < game->field_height; y++) {
    for (x = 0; x < game->field_width; x++) {
      if ((game->field[x][y] & field_type) == field_atom) {
        listx[listlen] = x;
        listy[listlen] = y;
        if (listlen < 62) listlen++;
      }
    }
  }
  if (listlen > 1) {
    /* randomize the list */
    srand(time(NULL));
    for (i = 0; i < listlen; i++) { /* randomize as many times as many items we have in the list */
      x = (rand() % listlen);
      if (x != i) {
        listx[63] = listx[i];
        listy[63] = listy[i];
        listx[i] = listx[x];
        listy[i] = listy[x];
        listx[x] = listx[63];
        listy[x] = listy[63];
      }
    }
  }
  /* animate explosions of atoms */
  for (x = 0; x < listlen; x++) {
    if (sounds->soundflag != 0) snd_playwav(sounds->explode, 0);
    for (i = 0; i < 8; i++) {
      draw_playfield_tile(game, listx[x], listy[x], sprites, sprites->empty);
      draw_playfield_tile(game, listx[x], listy[x], sprites, sprites->explosion[i]);
      gra_refresh();
      tim_delay(40);
    }
    game->field[listx[x]][listy[x]] = field_free; /* update the playfield to mark the area free */
    draw_playfield_tile(game, listx[x], listy[x], sprites, sprites->empty);
    gra_refresh();
    tim_delay(150);
  }
}


/* moves the cursor up by one place up */
static void move_cursor_up(struct atomixgame *game, struct spritesstruct *sprites) {
  int y, x, y1, y2;
  unsigned long sleepuntilticks;
  if (game->cursory <= 0) return;
  if (game->field[game->cursorx][game->cursory - 1] == 0) return;
  x = game->offseth + (game->cursorx * TILESIZE);
  y1 = game->offsetv + (game->cursory * TILESIZE);
  y2 = game->offsetv + ((game->cursory - 1) * TILESIZE);
  sleepuntilticks = tim_getticks();
  for (y = y1; y >= y2; y--) {
    if (y % 4 == 0) {
      sleepuntilticks += 10;
      draw_playfield_tile(game, game->cursorx, game->cursory, sprites, NULL);
      draw_playfield_tile(game, game->cursorx, game->cursory - 1, sprites, NULL);
      gra_drawsprite(sprites->cursor[0], x, y);
      gra_refresh();
      tim_wait_until_tick(sleepuntilticks, NULL);
    }
  }
  game->cursory -= 1;
}


/* moves the cursor by one place right */
static void move_cursor_right(struct atomixgame *game, struct spritesstruct *sprites) {
  int y, x, x1, x2;
  unsigned long sleepuntilticks;
  if (game->field[game->cursorx + 1][game->cursory] == 0) return;
  y = game->offsetv + (game->cursory * TILESIZE);
  x1 = game->offseth + (game->cursorx * TILESIZE);
  x2 = game->offseth + ((game->cursorx + 1) * TILESIZE);
  sleepuntilticks = tim_getticks();
  for (x = x1; x <= x2; x++) {
    if (x % 4 == 0) {
      sleepuntilticks += 10;
      draw_playfield_tile(game, game->cursorx, game->cursory, sprites, NULL);
      draw_playfield_tile(game, game->cursorx + 1, game->cursory, sprites, NULL);
      gra_drawsprite(sprites->cursor[0], x, y);
      gra_refresh();
      tim_wait_until_tick(sleepuntilticks, NULL);
    }
  }
  game->cursorx += 1;
}

/* moves the cursor by one place down */
static void move_cursor_down(struct atomixgame *game, struct spritesstruct *sprites) {
  int y, x, y1, y2;
  unsigned long sleepuntilticks;
  if (game->field[game->cursorx][game->cursory + 1] == 0) return;
  x = game->offseth + (game->cursorx * TILESIZE);
  y1 = game->offsetv + (game->cursory * TILESIZE);
  y2 = game->offsetv + ((game->cursory + 1) * TILESIZE);
  sleepuntilticks = tim_getticks();
  for (y = y1; y <= y2; y++) {
    if (y % 4 == 0) {
      sleepuntilticks += 10;
      draw_playfield_tile(game, game->cursorx, game->cursory, sprites, NULL);
      draw_playfield_tile(game, game->cursorx, game->cursory + 1, sprites, NULL);
      gra_drawsprite(sprites->cursor[0], x, y);
      gra_refresh();
      tim_wait_until_tick(sleepuntilticks, NULL);
    }
  }
  game->cursory += 1;
}

/* moves the cursor up by one place left */
static void move_cursor_left(struct atomixgame *game, struct spritesstruct *sprites) {
  int y, x, x1, x2;
  unsigned long sleepuntilticks;
  if (game->cursorx <= 0) return;
  if (game->field[game->cursorx - 1][game->cursory] == 0) return;
  y = game->offsetv + (game->cursory * TILESIZE);
  x1 = game->offseth + (game->cursorx * TILESIZE);
  x2 = game->offseth + ((game->cursorx - 1) * TILESIZE);
  sleepuntilticks = tim_getticks();
  for (x = x1; x >= x2; x--) {
    if (x % 4 == 0) {
      sleepuntilticks += 10;
      draw_playfield_tile(game, game->cursorx, game->cursory, sprites, NULL);
      draw_playfield_tile(game, game->cursorx - 1, game->cursory, sprites, NULL);
      gra_drawsprite(sprites->cursor[0], x, y);
      gra_refresh();
      tim_wait_until_tick(sleepuntilticks, NULL);
    }
  }
  game->cursorx -= 1;
}


/* reads an ascii char and returns the index of the related glyph within font1 (A..Z0..9) */
static int ascii2font1(char c) {
  if ((c >= 'A') && (c <= 'Z')) return(c - 'A');
  if ((c >= 'a') && (c <= 'z')) return(c - 'a');
  if ((c >= '0') && (c <= '9')) return(26 + (c - '0'));
  return(36);
}


static void drawstring3(struct spritesstruct *sprites, char *str, int x, int y) {
  char *curpos;
  for (curpos = str; *curpos != 0; curpos++) {
    gra_drawsprite(sprites->font3[*curpos - 'A'], x, y);
    x += gra_getspritewidth(sprites->font3[0]);
  }
}


static void draw_game_screen(struct atomixgame *game, struct spritesstruct *sprites, int skipcursor, time_t curtime, long curtick, struct loosetile_t *loosetile) {
  int x, y;
  int rect_x, rect_y;
  struct gra_sprite *tile;
  unsigned int timeleft = 0;
  unsigned char font1_width[] = {5,5,4,5,4,4,5,5,2,4,4,4,6,5,5,5,5,5,5,4,5,4,6,4,5,4,5,4,4,4,4,4,5,4,5,5}; /* provides the width of every single glyph in the font1 set */
  char tmpstring[16];
  gra_clear();  /* first clear out the screen */
  gra_drawsprite(sprites->bg[game->bg], 0, 0);  /* draw background */
  if (curtime <= game->time_end) {
    timeleft = game->time_end - curtime;
  }
  /* Draw the playfield */
  for (y = 0; y < game->field_height; y++) {
    for (x = 0; x < game->field_width; x++) {
      draw_playfield_tile(game, x, y, sprites, NULL);
    }
  }
  if (loosetile != NULL) {
    /* if it's a selected atom, draw it first */
    if (loosetile->atom >= 0) {
      gra_drawsprite(sprites->empty, loosetile->x, loosetile->y);
      gra_drawsprite(sprites->atom[loosetile->atom & field_index], loosetile->x, loosetile->y);
    }
    /* now draw the cursor */
    gra_drawsprite(sprites->cursor[game->cursorstate], loosetile->x, loosetile->y);
  }
  /* draw the cursor */
  if (skipcursor == 0) {
    rect_x = game->offseth + (game->cursorx * gra_getspritewidth(sprites->cursor[0]));
    rect_y = game->offsetv + (game->cursory * gra_getspriteheight(sprites->cursor[0]));
    gra_drawsprite(sprites->cursor[game->cursorstate], rect_x, rect_y);
  }
  /* Draw text ("HISCORE") */
  rect_x = TILESIZE / 2;
  rect_y = TILESIZE / 2;
  drawstring3(sprites, "HISCORE", rect_x, rect_y);
  /* Draw text (actual hiscore) */
  rect_y += gra_getspriteheight(sprites->font3[0]) * 1.40;
  snprintf(tmpstring, 16, "%d", game->hiscore);
  for (x = 0; tmpstring[x] != 0; x++) {
    gra_drawsprite(sprites->font2[(unsigned)tmpstring[x] - '0'], rect_x, rect_y);
    rect_x += gra_getspritewidth(sprites->font2[0]);
  }
  /* Draw text ("SCORE") */
  rect_x = TILESIZE / 2;
  rect_y += TILESIZE * 2;
  drawstring3(sprites, "SCORE", rect_x, rect_y);
  /* Draw text (actual score) */
  rect_y += gra_getspriteheight(sprites->font3[0]) * 1.40;
  snprintf(tmpstring, 16, "%d", game->score);
  for (x = 0; tmpstring[x] != 0; x++) {
    gra_drawsprite(sprites->font2[(unsigned)tmpstring[x] - '0'], rect_x, rect_y);
    rect_x += gra_getspritewidth(sprites->font2[0]);
  }
  /* Draw text ("LEVEL") */
  rect_x = TILESIZE / 2;
  rect_y += TILESIZE * 2;
  drawstring3(sprites, "LEVEL", rect_x, rect_y);
  /* Draw text (level number) */
  rect_y += gra_getspriteheight(sprites->font3[0]) * 1.40;
  gra_drawsprite(sprites->font2[game->level / 10], rect_x, rect_y);
  rect_x += gra_getspritewidth(sprites->font2[0]);
  gra_drawsprite(sprites->font2[game->level % 10], rect_x, rect_y);
  /* Draw text ("TIME") */
  rect_x = TILESIZE / 2;
  rect_y += TILESIZE * 2;
  drawstring3(sprites, "TIME", rect_x, rect_y);
  /* Draw text (time left) */
  rect_x = TILESIZE / 2;
  rect_y += gra_getspriteheight(sprites->font3[0]) * 1.40;
  gra_drawsprite(sprites->font2[timeleft / 60], rect_x, rect_y);
  rect_x += gra_getspritewidth(sprites->font2[0]);
  gra_drawsprite(sprites->font2[10], rect_x, rect_y);
  rect_x += gra_getspritewidth(sprites->font2[0]);
  gra_drawsprite(sprites->font2[(timeleft % 60) / 10], rect_x, rect_y);
  rect_x += gra_getspritewidth(sprites->font2[0]);
  gra_drawsprite(sprites->font2[timeleft % 10], rect_x, rect_y);
  /* draw the preview window (alternate the sprite every 800ms) */
  rect_x = 0;
  rect_y = 240 - 71;
  gra_drawsprite(sprites->preview[(curtick % 1600) / 800], rect_x, rect_y);
  /* draw the molecule's description - line 1 */
  y = 0; /* compute the pixel length of the first line */
  for (x = 0; x < (int)strlen(game->level_desc_line1); x++) {
    y += font1_width[ascii2font1(game->level_desc_line1[x])];
  }
  rect_x = 36 - (y / 2);
  rect_y = 181;
  for (x = 0; x < 16; x++) {
    gra_drawsprite(sprites->font1[ascii2font1(game->level_desc_line1[x])], rect_x, rect_y);
    rect_x += (font1_width[ascii2font1(game->level_desc_line1[x])]);
  }
  /* draw the molecule's description - line 2 */
  y = 0; /* compute the pixel length of the second line */
  for (x = 0; x < (int)strlen(game->level_desc_line2); x++) {
    y += font1_width[ascii2font1(game->level_desc_line2[x])];
  }
  rect_x = 36 - (y / 2);
  rect_y += gra_getspriteheight(sprites->font1[0]) + 2;
  for (x = 0; x < 16; x++) {
    gra_drawsprite(sprites->font1[ascii2font1(game->level_desc_line2[x])], rect_x, rect_y);
    rect_x += (font1_width[ascii2font1(game->level_desc_line2[x])]);
  }
  /* Draw the solution preview inside the preview window */
  for (y = 0; y < game->solution_height; y++) {
    for (x = 0; x < game->solution_width; x++) {
      if ((game->solution[x][y] & field_type) == field_atom) {
        int preview_offset_x, preview_offset_y;
        tile = sprites->satom[game->solution[x][y] & field_index];
        preview_offset_x = ((8 - game->solution_width) * (TILESIZE / 2)) / 2;
        preview_offset_y = ((7 - game->solution_height) * (TILESIZE / 2)) / 2;
        if (game->level_desc_line2[0] == 0) { /* if the molecule's name uses two lines, adapt the offset of the preview */
          preview_offset_y -= 8;
        }
        rect_x = 4 + preview_offset_x + (x * TILESIZE / 2);
        rect_y = 16 + (240 - 71) + preview_offset_y + (y * TILESIZE / 2);
        gra_drawsprite(tile, rect_x, rect_y);
      }
    }
  }
  /* Refresh the screen */
  gra_refresh();
}


/* moves an atom from one position of the field to another */
static void move_atom(struct atomixgame *game, int x_from, int y_from, int x_to, int y_to, struct soundsstruct *sounds, struct spritesstruct *sprites) {
  int x, y, kierunekx = 0, kieruneky = 0, sndchannel;
  /* int rect_x, rect_y; */
  struct loosetile_t loosetile;
  if (x_from > x_to) {
      kierunekx = -1;
    } else if (x_from < x_to) {
      kierunekx = 1;
    } else if (y_from > y_to) {
      kieruneky = -1;
    } else if (y_from < y_to) {
      kieruneky = 1;
    } else {  /* we're not moving at all! */
      return;
  }
  x = game->offseth + (x_from * TILESIZE);
  y = game->offsetv + (y_from * TILESIZE);
  /* rect_x = 0; */
  if (sounds->soundflag != 0) {
      sndchannel = snd_playwav(sounds->bzzz, -1);
    } else {
      sndchannel = -1;
  }
  /* move the moving tile from the playfield into a loosetile struct */
  loosetile.origpos_x = x;
  loosetile.origpos_y = y;
  loosetile.atom = game->field[x_from][y_from];
  game->field[x_from][y_from] = field_free;
  /* draw the moving */
  for (; y != game->offsetv + (y_to * TILESIZE); y += kieruneky) {
    /* if (rect_x != 0) gra_drawsprite(empty, rect_x, rect_y); */
    loosetile.x = x;
    loosetile.y = y;
    /* draw the screen */
    if (y % 3 == 0) {
      draw_game_screen(game, sprites, 1, time(NULL), tim_getticks(), &loosetile);
      tim_delay(20);
    }
  }
  /* rect_x = 0; */
  for (; x != game->offseth + (x_to * TILESIZE); x += kierunekx) {
    /* if (rect_x != 0) gra_drawsprite(empty, rect_x, rect_y); */
    loosetile.x = x;
    loosetile.y = y;
    /* draw the screen */
    if (x % 3 == 0) {
      draw_game_screen(game, sprites, 1, time(NULL), tim_getticks(), &loosetile);
      tim_delay(20);
    }
  }
  /* place the tile at its final position */
  game->field[x_to][y_to] = loosetile.atom;
  /* make sure the screen is up to date */
  draw_game_screen(game, sprites, 0, time(NULL), tim_getticks(), NULL);
  if (sndchannel != -1) snd_wavstop(sndchannel, 100);
}


/* displays the level selection screen. returns the selected level to load, or -1 on QUIT request */
static int selectlevel(int curlevel, int max_auth_level, int last_level, struct gra_sprite *infoscreen, struct gra_sprite *levsel, struct gra_sprite *levsel2, struct spritesstruct *sprites, int *hiscores) {
  enum atomiks_keys event;
  struct gra_sprite *tile;
  int x, y;
  struct atomixgame *game = atomix_initgame();

  if (curlevel > max_auth_level) curlevel = max_auth_level;
  for (;;) {
    atomix_loadgame(game, curlevel, ATOMIX_SRC_MEM, hiscores);
    gra_clear();
    gra_drawsprite(infoscreen, 0, 0);
    gra_drawsprite(levsel, 0, 0);
    if (max_auth_level > 1) gra_drawsprite(levsel2, 0, 0);

    /* Draw the solution preview inside the preview window */
    for (y = 0; y < game->solution_height; y++) {
      for (x = 0; x < game->solution_width; x++) {
        if ((game->solution[x][y] & field_type) == field_atom) {
          int preview_offset_y;
          int rect_x, rect_y;
          tile = sprites->satom[game->solution[x][y] & field_index];
          preview_offset_y = (7 - game->solution_height) * TILESIZE / 4;
          rect_x = (x * TILESIZE / 2) + (320 / 2) - (game->solution_width * TILESIZE / 4);
          rect_y = 95 + preview_offset_y + (y * TILESIZE >> 1);
          gra_drawsprite(tile, rect_x, rect_y);
        }
      }
    }

    /* draw the level number */
    x = (320 / 2) - gra_getspritewidth(sprites->font2[0]);
    gra_drawsprite(sprites->font2[curlevel / 10], x, 185);
    x += gra_getspritewidth(sprites->font2[0]);
    gra_drawsprite(sprites->font2[curlevel % 10], x, 185);

    /* draw 'completed' over the level number, if completed indeed */
    if (curlevel < max_auth_level) {
      gra_drawsprite(sprites->completed, 10 + (320 / 2) - (gra_getspritewidth(sprites->completed) / 2), 110);
    }
    /* refresh the screen */
    gra_refresh();
    /* Get keypress */
    event = inp_waitkey(500);
    switch (event) {
      case atomiks_quit:
        return(-1);
        break;
      case atomiks_left:
        if (curlevel > 1) curlevel -= 1;
        break;
      case atomiks_right:
        if ((curlevel < max_auth_level) && (curlevel < last_level)) curlevel += 1;
        break;
      case atomiks_home:
        curlevel = 1;
        break;
      case atomiks_end:
        curlevel = max_auth_level;
        if (curlevel > last_level) curlevel = last_level;
        break;
      case atomiks_enter:
        return(curlevel);
        break;
      case atomiks_fullscreen:
        gra_switchfullscreen();
        break;
      case atomiks_esc:
        return(-1);
        break;
      default:
        break;
    }
  }
}


static void getcfg(int *max_auth_level, int *hiscores, int last_level) {
  FILE *fd;
  int x, buff1, buff2;
  fd = cfg_fopen("rb", "Atomiks");
  for (x = 0; x < last_level; x++) {
    hiscores[x] = 0;
  }
  if (fd == NULL) {
      *max_auth_level = 1;
    } else {
      *max_auth_level = fgetc(fd);
      if (*max_auth_level < 1) *max_auth_level = 1;
      for (x = 0; x < last_level; x++) {
        buff1 = fgetc(fd);
        buff2 = fgetc(fd);
        if ((buff1 < 0) || (buff2 < 0)) break;
        hiscores[x] = (buff1 << 8) | buff2;
      }
      fclose(fd);
  }
}


static void savecfg(int max_auth_level, int *hiscores, int last_level) {
  FILE *fd;
  int x;
  fd = cfg_fopen("wb", "Atomiks");
  if (fd == NULL) return;
  fputc(max_auth_level, fd);
  for (x = 0; x < last_level; x++) {
    fputc((hiscores[x] >> 8) & 0xFF, fd);
    fputc(hiscores[x] & 0xFF, fd);
  }
  fclose(fd);
}



#define last_level 30

int main(int argc, char **argv) {
  struct spritesstruct sprites;
  struct gra_sprite *title, *infoscreen, *instructions, *intro[3], *levsel, *levsel2, *timeoutscreen, *pausedscreen, *creditscreen;
  enum atomiks_keys event;
  struct atomixgame *game;
  int x, exitflag = 0, gamejuststarted;
  long nextscreenrefresh = 0;
  int max_auth_level;
  int hiscores[last_level];
  struct snd_mod *music_title, *music_end;
  struct soundsstruct sounds;
  int videoflags = 0;

  getcfg(&max_auth_level, hiscores, last_level);
  sounds.soundflag = 1;

  /* Look for command-line parameters (fullscreen only for now) */
  for (x = 1; x < argc; x++) {
    if (strcmp(argv[x], "--fullscreen") == 0) videoflags |= GRA_FULLSCREEN;
    if (strcmp(argv[x], "--nosound") == 0) sounds.soundflag = 0;
  }

  /* Init SDL and set the video mode */
  if (gra_init(640, 480, videoflags, "Atomiks " PVER, img_tinyicon_bmp_gz, img_tinyicon_bmp_gz_len) != 0) {
    puts("Error: unable to init screen!");
    return(1);
  }

  /* init the audio system */
  if (snd_init() != 0) puts("Could not initialize the sound subsystem!");

  /* load music and sound effects */
  music_title = snd_loadmod(snd_title_mod, snd_title_mod_len);
  if (music_title == NULL) puts("Ooops! loading title module failed :/");
  music_end = snd_loadmod(snd_end_mod, snd_end_mod_len);
  if (music_end == NULL) puts("Ooops! loading end module failed :/");
  sounds.bzzz = snd_loadwav(snd_bzzz_wav, snd_bzzz_wav_len);
  sounds.explode = snd_loadwav(snd_explode_wav, snd_explode_wav_len);
  sounds.selected = snd_loadwav(snd_selected_wav, snd_selected_wav_len);

  /* Preload all graphics */
  title = loadgzbmp(img_title_bmp_gz, img_title_bmp_gz_len);
  creditscreen = loadgzbmp(img_credits_bmp_gz, img_credits_bmp_gz_len);
  timeoutscreen = loadgzbmp(img_timeout_bmp_gz, img_timeout_bmp_gz_len);
  infoscreen = loadgzbmp(img_infoscreen_bmp_gz, img_infoscreen_bmp_gz_len);
  pausedscreen = loadgzbmp(img_pausedscreen_bmp_gz, img_pausedscreen_bmp_gz_len);
  instructions = loadgzbmp(img_instructs_bmp_gz, img_instructs_bmp_gz_len);
  intro[0] = loadgzbmp(img_intro1_bmp_gz, img_intro1_bmp_gz_len);
  intro[1] = loadgzbmp(img_intro2_bmp_gz, img_intro2_bmp_gz_len);
  intro[2] = loadgzbmp(img_intro3_bmp_gz, img_intro3_bmp_gz_len);
  levsel = loadgzbmp(img_levsel_bmp_gz, img_levsel_bmp_gz_len);
  levsel2 = loadgzbmp(img_levsel2_bmp_gz, img_levsel2_bmp_gz_len);
  sprites.completed = loadgzbmp(img_completed_bmp_gz, img_completed_bmp_gz_len);

  /* load backgrounds */
  loadSpriteSheet(sprites.bg, 320, 240, 3, img_bg_bmp_gz, img_bg_bmp_gz_len);
  loadSpriteSheet(&sprites.black, 320, 240, 1, img_black_bmp_gz, img_black_bmp_gz_len);
  /* load the preview windows */
  loadSpriteSheet(&sprites.preview[0], 71, 71, 1, img_preview_bmp_gz, img_preview_bmp_gz_len);
  loadSpriteSheet(&sprites.preview[1], 71, 71, 1, img_preview2_bmp_gz, img_preview2_bmp_gz_len);
  /* load the 'empty space' tile */
  loadSpriteSheet(&sprites.empty, 16, 16, 1, img_empty_bmp_gz, img_empty_bmp_gz_len);
  /* load atoms sprites */
  loadSpriteSheet(sprites.atom, 16, 16, 49, img_atoms_bmp_gz, img_atoms_bmp_gz_len);
  /* load 'small' atoms sprites */
  loadSpriteSheet(sprites.satom, 8, 8, 49, img_satoms_bmp_gz, img_satoms_bmp_gz_len);
  /* load the explosion sprites */
  loadSpriteSheet(sprites.explosion, 16, 16, 8, img_explosion_bmp_gz, img_explosion_bmp_gz_len);
  /* load walls sprites */
  loadSpriteSheet(sprites.wall, 16, 16, 19, img_walls_bmp_gz, img_walls_bmp_gz_len);
  /* load cursor sprites */
  loadSpriteSheet(sprites.cursor, 16, 16, 3, img_cursors_bmp_gz, img_cursors_bmp_gz_len);
  /* load fonts */
  loadSpriteSheet(sprites.font1, 5, 5, 37, img_font1_bmp_gz, img_font1_bmp_gz_len);
  loadSpriteSheet(sprites.font2, 14, 16, 11, img_font2_bmp_gz, img_font2_bmp_gz_len);
  loadSpriteSheet(sprites.font3, 7, 8, 26, img_font3_bmp_gz, img_font3_bmp_gz_len);

  /* start playing the module in an infinite loop */
  if (sounds.soundflag != 0) {
    if (snd_playmod(music_title, -1, 0) != 0) puts("snd_playmod() error!");
  }

  /* show the title screen, refreshing it 4x a second, just in case the user
   * messes with the display (like switching to fullscreen) */
  for (x = 0; (x < 16) && (exitflag == 0); x++) {
    enum atomiks_keys tempevent;
    gra_clear();
    gra_drawsprite(title, 0, 0);
    gra_refresh();
    exitflag = waitforanykey(250, &tempevent);
    if ((tempevent != atomiks_none) && (tempevent != atomiks_fullscreen)) break;
  }
  inp_flush_events();

  for (x = 0; (x < 3) && (exitflag == 0); x++) {
    while (exitflag == 0) {
      enum atomiks_keys tempevent;
      gra_clear();
      gra_drawsprite(infoscreen, 0, 0);
      gra_drawsprite(intro[x], 0, 0);
      gra_refresh();
      exitflag = waitforanykey(250, &tempevent);
      if ((tempevent != atomiks_none) && (tempevent != atomiks_fullscreen)) break;
    }
  }
  inp_flush_events();

  /* Init the game and start on level 1 */
  game = atomix_initgame();
  if (exitflag == 0) {
    if ((game->level = selectlevel(1, max_auth_level, last_level, infoscreen, levsel, levsel2, &sprites, hiscores)) < 1) {
      exitflag = 1;
      game->level = 1;
    }
    atomix_loadgame(game, game->level, ATOMIX_SRC_MEM, hiscores);
  }

  snd_modstop(2000);   /* slowly stop playing the background music */
  gamejuststarted = 1;
  nextscreenrefresh = 0; /* force a first refresh */

  while (exitflag == 0) {
    time_t pausedtime;
    int cursorx_backup = game->cursorx;
    int cursory_backup = game->cursory;
    int tmp;
    /* Wait for next event, while keeping the screen refreshed */
    while (exitflag == 0) {
      long curtick = tim_getticks();
      /* keep redrawing the screen every 0.2s */
      if (curtick > nextscreenrefresh) {
        nextscreenrefresh = curtick + 200;
        draw_game_screen(game, &sprites, 0, time(NULL), curtick, NULL); /* draw the game only if we have time */
        /* if we are starting Atomix experience, display a short notice */
        if ((game->level == 1) && (max_auth_level == 1) && (gamejuststarted == 1)) {
          gra_drawsprite(instructions, 0, 0);
          gra_refresh();
          tim_delay(1000);
          gamejuststarted = 0;
          inp_flush_events();
          exitflag = waitforanykey(0, NULL);
          game->time_end = time(NULL) + game->duration;
        }
        if (game->time_end < time(NULL)) {
          gra_drawsprite(timeoutscreen, 0, 0);
          gra_refresh();
          tim_delay(1000);
          inp_flush_events();
          exitflag = waitforanykey(0, NULL);
          atomix_loadgame(game, game->level, ATOMIX_SRC_MEM, hiscores);
        }
      }
      event = pollkey();
      if (event != atomiks_none) break;
      tim_delay(20);
    }
    switch (event) {
      case atomiks_quit:
        exitflag = 1;
        break;
      case atomiks_lostfocus:
        pausedtime = time(NULL);
        gra_drawsprite(pausedscreen, 0, 0);  /* draw paused screen */
        for (;;) {
          gra_refresh();
          event = inp_waitkey(500);
          if (event == atomiks_quit) {
              exitflag = 1;
              break;
            } else if (event == atomiks_gotfocus) {
              game->time_end += (time(NULL) - pausedtime);
              break;
          }
        }
        break;
      case atomiks_left:
        if (game->cursorstate == 0) {
            move_cursor_left(game, &sprites);
          } else {
            tmp = atomix_getmovedistance(game, 3);
            if (tmp != 0) {
              if (game->score >= 5) game->score -= 5;
              game->cursorx -= tmp;
              move_atom(game, cursorx_backup, cursory_backup, game->cursorx, game->cursory, &sounds, &sprites);
            }
        }
        break;
      case atomiks_right:
        if (game->cursorstate == 0) {
            move_cursor_right(game, &sprites);
          } else {
            tmp = atomix_getmovedistance(game, 1);
            if (tmp != 0) {
              if (game->score >= 5) game->score -= 5;
              game->cursorx += tmp;
              move_atom(game, cursorx_backup, cursory_backup, game->cursorx, game->cursory, &sounds, &sprites);
            }
        }
        break;
      case atomiks_up:
        if (game->cursorstate == 0) {
            move_cursor_up(game, &sprites);
          } else {
            tmp = atomix_getmovedistance(game, 0);
            if (tmp != 0) {
              if (game->score >= 5) game->score -= 5;
              game->cursory -= tmp;
              move_atom(game, cursorx_backup, cursory_backup, game->cursorx, game->cursory, &sounds, &sprites);
            }
        }
        break;
      case atomiks_down:
        if (game->cursorstate == 0) {
            move_cursor_down(game, &sprites);
          } else {
            tmp = atomix_getmovedistance(game, 2);
            if (tmp != 0) {
              if (game->score >= 5) game->score -= 5;
              game->cursory += tmp;
              move_atom(game, cursorx_backup, cursory_backup, game->cursorx, game->cursory, &sounds, &sprites);
            }
        }
        break;
      case atomiks_esc:
        if (sounds.soundflag != 0) {
          if (snd_playmod(music_title, -1, 0) != 0) printf("snd_playmod() error!\n");
        }
        if ((game->level = selectlevel(game->level, max_auth_level, last_level, infoscreen, levsel, levsel2, &sprites, hiscores)) < 0) {
            exitflag = 1;
          } else {
            snd_modstop(2000);
            atomix_loadgame(game, game->level, ATOMIX_SRC_MEM, hiscores);
        }
        break;
      case atomiks_enter:
        if ((game->field[game->cursorx][game->cursory] & field_type) == field_atom) {
          if (game->cursorstate == 0) {
              game->cursorstate = game->cursortype;
              if (sounds.soundflag != 0) snd_playwav(sounds.selected, 0);
            } else {
              game->cursorstate = 0;
          }
        }
        /* force the screen to refresh immediately so the user feedback is instant */
        nextscreenrefresh = 0;
        break;
      case atomiks_fullscreen:
        gra_switchfullscreen();
        break;
      default:
        break;
    }
    /* After every move, check if we are in a winning position */
    if (atomix_checksolution(game) != 0) {
      time_t tmptime;
      if (game->level == max_auth_level) max_auth_level += 1;
      draw_game_screen(game, &sprites, 1, time(NULL), tim_getticks(), NULL);
      draw_anim_explosions(game, &sprites, &sounds); /* animate atoms explosion */
      tim_delay(750);
      for (tmptime = time(NULL); tmptime <= game->time_end; tmptime++) {
        game->score += 10;
        draw_game_screen(game, &sprites, 1, tmptime, tim_getticks(), NULL);
        if (tmptime % 2) tim_delay(10);
      }
      if (game->score > hiscores[game->level - 1]) hiscores[game->level - 1] = game->score;
      tim_delay(1000);
      if (game->level >= last_level) { /* catch final level for congrats screen! */
        int rectcredits_x, rectcredits_y, rectscreen_x, rectscreen_y, rectcredits_w, rectcredits_h;
        int firstloop = 1, quitcredits = 0;
        if (sounds.soundflag != 0) {
          if (snd_playmod(music_end, -1, 200) != 0) printf("snd_playmod() error!\n");
        }
        rectcredits_w = gra_getspritewidth(creditscreen);
        rectcredits_h = 181;
        rectscreen_x = 160 - (rectcredits_w >> 1);
        rectscreen_y = 36;
        rectcredits_x = 0;
        rectcredits_y = 0;
        while ((exitflag == 0) && (quitcredits == 0)) {
          gra_drawsprite(infoscreen, 0, 0);
          gra_drawpartsprite(creditscreen, rectcredits_x, rectcredits_y, rectcredits_w, rectcredits_h, rectscreen_x, rectscreen_y);
          gra_refresh();
          if (firstloop != 0) {
            tim_delay(5000);
            firstloop = 0;
          }
          tim_delay(100);
          event = pollkey();
          switch (event) {
            case atomiks_quit:
              exitflag = 1;
              break;
            case atomiks_esc:
            case atomiks_enter:
              quitcredits = 1;
              break;
            default:
              break;
          }
          if (rectcredits_y + rectcredits_h < gra_getspriteheight(creditscreen)) rectcredits_y += 1;
        }
        snd_modstop(2000);
        inp_flush_events();
        game->level = 0;
        tim_delay(2000);
      }
      if (exitflag == 0) {
        if (sounds.soundflag != 0) {
          if (snd_playmod(music_title, -1, 0) != 0) printf("snd_playmod() error!\n");
        }
        inp_flush_events();
        if ((game->level = selectlevel(game->level + 1, max_auth_level, last_level, infoscreen, levsel, levsel2, &sprites, hiscores)) < 0) {
          exitflag = 1;
          game->level = 1;
        }
        snd_modstop(2000);
        atomix_loadgame(game, game->level, ATOMIX_SRC_MEM, hiscores);
      }
    }
  }

  inp_flush_events();

  /* if some music is playing, fade it out */
  snd_modstop(500);

  /* fade out the screen by applying a black surface with decreasing transparency */
  for (x = 0; x < 16; x++) {
    gra_drawsprite_alpha(sprites.black, 0, 0, 30);
    gra_refresh();
    tim_delay(30);
  }

  savecfg(max_auth_level, hiscores, last_level);

  /* cleaning up stuff */
  free(game);
  /* SDL_FreeSurface(sprites.bg[0]);
  SDL_FreeSurface(creditscreen);
  SDL_FreeSurface(title);
  for (x = 0; x < 18; x++) SDL_FreeSurface(sprites.atom[x]);
  for (x = 0; x < 8; x++) SDL_FreeSurface(sprites.wall[x]);
  for (x = 0; x < 3; x++) SDL_FreeSurface(intro[x]); */
  snd_modfree(music_title);
  snd_wavfree(sounds.bzzz);
  snd_wavfree(sounds.explode);
  snd_wavfree(sounds.selected);
  snd_close();  /* this one takes a long time (~2s) when using the PulseAudio driver... This is a known bug, there's not much I can do about this */
  gra_close();
  return(0);
}
