/*
 * This file is part of the Atomiks project
 * Copyright (C) Mateusz Viste 2013, 2014, 2015
 *
 * Level editor for Atomiks
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
#include <stdlib.h>    /* atoi(), malloc(), free() */
#include <SDL2/SDL.h>
#include "atomcore.h"
#include "data.h"
#include "drv_gra.h"


static void savelevel(struct atomixgame *game, int level) {
  char levelfile[64];
  FILE *fd;
  int x, y;
  sprintf(levelfile, "lev/lev%04d.dat", level);
  fd = fopen(levelfile, "wb");
  if (fd == NULL) return;
  /* write initial playfield */
  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
      fputc(game->field[x][y], fd);
    }
  }
  /* write solution */
  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
      fputc(game->solution[x][y], fd);
    }
  }
  /* write the timer */
  fputc((game->duration >> 8) & 0xFF, fd);
  fputc(game->duration & 0xFF, fd);
  /* write level descriptions */
  for (x = 0; x < 15; x++) fputc(game->level_desc_line1[x], fd);
  for (x = 0; x < 15; x++) fputc(game->level_desc_line2[x], fd);
  /* write the cursor type */
  fputc(game->cursortype, fd);
  /* write the bg id */
  fputc(game->bg, fd);
  fclose(fd);
}


static int sdlk2char(int sdlkey) {
  switch (sdlkey) {
    case SDLK_a: return('A');
    case SDLK_b: return('B');
    case SDLK_c: return('C');
    case SDLK_d: return('D');
    case SDLK_e: return('E');
    case SDLK_f: return('F');
    case SDLK_g: return('G');
    case SDLK_h: return('H');
    case SDLK_i: return('I');
    case SDLK_j: return('J');
    case SDLK_k: return('K');
    case SDLK_l: return('L');
    case SDLK_m: return('M');
    case SDLK_n: return('N');
    case SDLK_o: return('O');
    case SDLK_p: return('P');
    case SDLK_q: return('Q');
    case SDLK_r: return('R');
    case SDLK_s: return('S');
    case SDLK_t: return('T');
    case SDLK_u: return('U');
    case SDLK_v: return('V');
    case SDLK_w: return('W');
    case SDLK_x: return('X');
    case SDLK_y: return('Y');
    case SDLK_z: return('Z');
    case SDLK_PERIOD: return('.');
    default: return(' ');
  }
}


int main(int argc, char **argv) {
  int x, y, level, exitflag = 0;
  int viewmode = 0;  /* 0 = edit level / 1 = edit solution */
  int cursorx = 0, cursory = 0;
  int selectedline = 1, selectedchar = 0;
  int lastitem = 0;
  struct atomixgame *game;
  struct gra_sprite *bg[3], *wall[19], *empty, *atom[64], *tile, *cursor[3], *font[16], *font3[64];
  if (argc != 2) {
    puts("Usage: editor levelnum");
    return(1);
  }
  level = atoi(argv[1]);

  game = malloc(sizeof(struct atomixgame));
  if (game == NULL) {
    puts("Error: out of memory");
    return(2);
  }
  atomix_loadgame(game, level, ATOMIX_SRC_FILE, NULL);

  gra_init(640, 480, 0, "editor", NULL, 0);

  /* Preload all graphics */
  /* load the 'empty space' tile */
  loadSpriteSheet(&empty, 16, 16, 1, img_empty_bmp_gz, img_empty_bmp_gz_len);
  /* load backgrounds */
  loadSpriteSheet(&bg[0], 320, 240, 3, img_bg_bmp_gz, img_bg_bmp_gz_len);
  /* load atoms sprites */
  loadSpriteSheet(&atom[0], 16, 16, 49, img_atoms_bmp_gz, img_atoms_bmp_gz_len);
  /* load walls sprites */
  loadSpriteSheet(&wall[0], 16, 16, 19, img_walls_bmp_gz, img_walls_bmp_gz_len);
  /* load cursor sprites */
  loadSpriteSheet(&cursor[0], 16, 16, 3, img_cursors_bmp_gz, img_cursors_bmp_gz_len);
  /* load timer font */
  loadSpriteSheet(&font[0], 14, 15, 11, img_font2_bmp_gz, img_font2_bmp_gz_len);
  /* load generic font */
  loadSpriteSheet(&font3[0], 7, 7, 26, img_font3_bmp_gz, img_font3_bmp_gz_len);

  for (;;) {
    SDL_Event event;
    int rect_x, rect_y, rect_w, rect_h, keybuff;
    gra_drawsprite(bg[game->bg], 0, 0);    /* draw the background */
    /* Draw the timer */
    rect_x = 260;
    rect_y = 20;
    gra_drawsprite(font[game->duration / 60], rect_x, rect_y);
    rect_x += gra_getspritewidth(font[0]);
    gra_drawsprite(font[10], rect_x, rect_y);
    rect_x += gra_getspritewidth(font[0]);
    gra_drawsprite(font[(game->duration % 60) / 10], rect_x, rect_y);
    rect_x += gra_getspritewidth(font[0]);
    gra_drawsprite(font[game->duration % 10], rect_x, rect_y);
    /* Draw the descriptions */
    rect_x = 210;
    rect_y = 220;
    rect_w = gra_getspritewidth(font3[0]) * 2;
    rect_h = gra_getspriteheight(font3[0]) * 2;
    for (x = 0; x < 15; x++) {
      if ((selectedline == 1) && (selectedchar == x)) {
          gra_drawrect(rect_x * 2, rect_y * 2, rect_w, rect_h, 255, 0, 0, 255, 1);
        } else {
          gra_drawrect(rect_x * 2, rect_y * 2, rect_w, rect_h, 0x30, 0x30, 0x30, 255, 1);
      }
      if ((game->level_desc_line1[x] <= 'Z') && (game->level_desc_line1[x] >= 'A')) {
        gra_drawsprite(font3[game->level_desc_line1[x] - 'A'], rect_x, rect_y);
      }
      rect_x += gra_getspritewidth(font3[0]);
    }
    rect_x = 210;
    rect_y += gra_getspriteheight(font3[0]) + 2;
    for (x = 0; x < 15; x++) {
      if ((selectedline == 2) && (selectedchar == x)) {
          gra_drawrect(rect_x * 2, rect_y * 2, rect_w, rect_h, 255, 0, 0, 255, 1);
        } else {
          gra_drawrect(rect_x * 2, rect_y * 2, rect_w, rect_h, 0x30, 0x30, 0x30, 255, 1);
      }
      if ((game->level_desc_line2[x] <= 'Z') && (game->level_desc_line2[x] >= 'A')) {
        gra_drawsprite(font3[game->level_desc_line2[x] - 'A'], rect_x, rect_y);
      }
      rect_x += gra_getspritewidth(font3[0]);
    }
    /* Draw the playfield or solution (depends of the current mode) */
    if (viewmode == 0) { /* draw playfield */
        for (y = 0; y < 64; y++) {
          for (x = 0; x < 64; x++) {
            if ((game->field[x][y] & field_type) == field_atom) {
                tile = atom[game->field[x][y] & field_index];
              } else if ((game->field[x][y] & field_type) == field_wall) {
                tile = wall[game->field[x][y] & field_index];
              } else if ((game->field[x][y] & field_type) == field_free) {
                tile = empty;
              } else {
                tile = NULL;
            }
            if (tile != NULL) {
              rect_x = 32 + (x * gra_getspritewidth(tile));
              rect_y = 32 + (y * gra_getspriteheight(tile));
              gra_drawsprite(empty, rect_x, rect_y);
              gra_drawsprite(tile, rect_x, rect_y);
            }
          }
        }
      } else { /* draw solution */
        for (y = 0; y < 32; y++) {
          for (x = 0; x < 32; x++) {
            if ((game->solution[x][y] & field_type) == field_atom) {
                tile = atom[game->solution[x][y] & field_index];
              } else if ((game->solution[x][y] & field_type) == field_wall) {
                tile = wall[game->field[x][y] & field_index];
              } else if ((game->solution[x][y] & field_type) == field_free) {
                tile = empty;
              } else {
                tile = NULL;
            }
            if (tile != NULL) {
              rect_x = 32 + (x * gra_getspritewidth(tile));
              rect_y = 32 + (y * gra_getspriteheight(tile));
              gra_drawsprite(empty, rect_x, rect_y);
              gra_drawsprite(tile, rect_x, rect_y);
            }
          }
        }
    }
    /* draw the cursor type */
    rect_x = 300;
    rect_y = 130;
    gra_drawsprite(cursor[game->cursortype], rect_x, rect_y);
    /* draw the cursor */
    rect_x = 32 + (cursorx * gra_getspritewidth(cursor[0]));
    rect_y = 32 + (cursory * gra_getspriteheight(cursor[0]));
    gra_drawsprite(cursor[0], rect_x, rect_y);
    /* Refresh the screen */
    gra_refresh();
    /* Wait for next event */
    SDL_WaitEvent(&event);
    if (event.type == SDL_QUIT) {
        exitflag = 1;
      } else if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {
          case SDLK_ESCAPE:
            exitflag = 1;
            break;
          case SDLK_LEFT:
            if (cursorx > 0) cursorx -= 1;
            break;
          case SDLK_RIGHT:
            if (cursorx < 63) cursorx += 1;
            break;
          case SDLK_UP:
            if (cursory > 0) cursory -= 1;
            break;
          case SDLK_DOWN:
            if (cursory < 63) cursory += 1;
            break;
          case SDLK_SPACE:
            if (viewmode == 0) { /* if current view is on playfield... */
                if ((game->field[cursorx][cursory] & field_type) == field_atom) {
                    game->field[cursorx][cursory] &= field_index; /* zero out the tile type */
                    game->field[cursorx][cursory] += 1;
                    if (game->field[cursorx][cursory] > 48) game->field[cursorx][cursory] = 0;
                    game->field[cursorx][cursory] |= field_atom;
                    lastitem = game->field[cursorx][cursory];
                  } else if ((game->field[cursorx][cursory] & field_type) == field_wall) {
                    game->field[cursorx][cursory] &= field_index; /* zero out the tile type */
                    game->field[cursorx][cursory] += 1;
                    if (game->field[cursorx][cursory] > 18) game->field[cursorx][cursory] = 0;
                    game->field[cursorx][cursory] |= field_wall;
                    lastitem = game->field[cursorx][cursory];
                }
              } else {  /* otherwise we are editing the solution */
                if ((game->solution[cursorx][cursory] & field_type) == field_atom) {
                    game->solution[cursorx][cursory] &= field_index; /* zero out the tile type */
                    game->solution[cursorx][cursory] += 1;
                    if (game->solution[cursorx][cursory] > 48) {
                        game->solution[cursorx][cursory] = 0;
                      } else {
                        game->solution[cursorx][cursory] |= field_atom;
                    }
                    lastitem = game->solution[cursorx][cursory];
                  } else {
                    game->solution[cursorx][cursory] = field_atom;
                    lastitem = game->solution[cursorx][cursory];
                }
            }
            break;
          case SDLK_INSERT: /* repeat the last item */
            if (viewmode == 0) { /* if current view is on playfield... */
                game->field[cursorx][cursory] = lastitem;
              } else {  /* otherwise we are editing the solution */
                game->solution[cursorx][cursory] = lastitem;
            }
            break;
          case SDLK_RETURN:
            if (viewmode == 0) { /* only if editing field */
              if ((game->field[cursorx][cursory] & field_type) == field_free) {
                  game->field[cursorx][cursory] = field_wall;
                } else if ((game->field[cursorx][cursory] & field_type) == field_wall) {
                  game->field[cursorx][cursory] = field_atom;
                } else if ((game->field[cursorx][cursory] & field_type) == field_atom) {
                  game->field[cursorx][cursory] = 0;
                } else {
                  game->field[cursorx][cursory] = field_free;
              }
              lastitem = game->field[cursorx][cursory];
            }
            break;
          case SDLK_DELETE:
            if (viewmode == 0) {
                game->field[cursorx][cursory] = 0;
              } else {
                game->solution[cursorx][cursory] = 0;
            }
            break;
          case SDLK_TAB:
            viewmode = 1 - viewmode;
            break;
          case SDLK_KP_MINUS:
            if (game->duration > 0) game->duration -= 1;
            break;
          case SDLK_KP_PLUS:
            if (game->duration < 3600) game->duration += 1;
            break;
          case SDLK_F1:
            selectedchar++;
            if (selectedchar >= 15) {
              selectedline = 3 - selectedline;
              selectedchar = 0;
            }
            break;
          case SDLK_F2:
            game->cursortype += 1;
            if (game->cursortype > 2) game->cursortype = 1;
            break;
          case SDLK_F3:
            game->bg += 1;
            if (game->bg > 2) game->bg = 0;
            break;
          case SDLK_F5:
            savelevel(game, level);
            puts("SAVED");
            break;
          default:  /* text input */
            keybuff = sdlk2char(event.key.keysym.sym);
            if (((keybuff >= 'A') && (keybuff <= 'Z')) || (keybuff == '.')) {
              if (selectedline == 1) {
                  if (keybuff == '.') {
                      game->level_desc_line1[selectedchar] = 0;
                    } else {
                      game->level_desc_line1[selectedchar] = keybuff;
                  }
                } else {
                  if (keybuff == '.') {
                      game->level_desc_line2[selectedchar] = 0;
                    } else {
                      game->level_desc_line2[selectedchar] = keybuff;
                  }
              }
            }
            break;
        }
    }
    if (exitflag != 0) break;
  }

  free(game);

  return(0);
}
