/*
 * This is part of the Atomiks project.
 * Copyright (C) Mateusz Viste 2013, 2014, 2015
 *
 * Operates on a 2D array (playfield) of 64x64, filled with moveable and non-moveable elements.
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

#include <stdlib.h>  /* malloc(), NULL */
#include <stdio.h>  /* sprintf(), FILE */
#include <time.h>
#include "atomcore.h"
#include "levels.h"

/* allocate a new game structure, and fill it with empty spaces */
struct atomixgame *atomix_initgame(void) {
  struct atomixgame *game;
  game = malloc(sizeof(struct atomixgame));
  return(game);
}


/* returns the distance that the block at position x/y would travel if pushed into 'direction'. direction is 0: up / 1: right / 2: down / 3: left */
int atomix_getmovedistance(struct atomixgame *game, int direction) {
  int i, x, y;
  x = game->cursorx;
  y = game->cursory;
  if ((game->field[x][y] & field_type) != field_atom) return(0); /* non-atoms don't move at all */
  /* UP */
  if (direction == 0) {
    y -= 1;
    for (i = y; i >= 0; i--) {
      if ((game->field[x][i] & field_type) != field_free) break;
    }
    return(y - i);
  }
  /* RIGHT */
  if (direction == 1) {
    x += 1;
    for (i = x; i < 16; i++) {
      if ((game->field[i][y] & field_type) != field_free) break;
    }
    return(i - x);
  }
  /* DOWN */
  if (direction == 2) {
    y += 1;
    for (i = y; i < 16; i++) {
      if ((game->field[x][i] & field_type) != field_free) break;
    }
    return(i - y);
  }
  /* LEFT */
  if (direction == 3) {
    x -= 1;
    for (i = x; i >= 0; i--) {
      if ((game->field[i][y] & field_type) != field_free) break;
    }
    return(x - i);
  }
  /* if direction is invalid, don't move */
  return(0);
}


void atomix_loadgame(struct atomixgame *game, int level, int source, int *hiscores) {
  char levelfile[128];
  int x, y, z;
  unsigned char *memptr;
  FILE *fd;
  game->field_width = 0;
  game->field_height = 0;
  game->solution_width = 0;
  game->solution_height = 0;
  game->cursorx = 0;
  game->cursory = 0;
  game->offseth = 80;
  game->offsetv = 48;
  game->cursorstate = 0;
  game->time_end = time(NULL) + 60; /* 270; */
  game->level = level;
  game->level_desc_line1[0] = 0;
  game->level_desc_line2[0] = 0;
  game->bg = 0;
  game->score = 500;
  if (hiscores != NULL) {
      game->hiscore = hiscores[level - 1];
    } else {
      game->hiscore = 0;
  }
  if (source == ATOMIX_SRC_FILE) {
      sprintf(levelfile, "lev/lev%04d.dat", level);
      fd = fopen(levelfile, "rb");
      if (fd == NULL) return;
      memptr = malloc(4096);
      fread(memptr, 4096, 1, fd);
      fclose(fd);
    } else {
      switch (level) {
        case 1:
          memptr = lev_lev0001_dat;
          break;
        case 2:
          memptr = lev_lev0002_dat;
          break;
        case 3:
          memptr = lev_lev0003_dat;
          break;
        case 4:
          memptr = lev_lev0004_dat;
          break;
        case 5:
          memptr = lev_lev0005_dat;
          break;
        case 6:
          memptr = lev_lev0006_dat;
          break;
        case 7:
          memptr = lev_lev0007_dat;
          break;
        case 8:
          memptr = lev_lev0008_dat;
          break;
        case 9:
          memptr = lev_lev0009_dat;
          break;
        case 10:
          memptr = lev_lev0010_dat;
          break;
        case 11:
          memptr = lev_lev0011_dat;
          break;
        case 12:
          memptr = lev_lev0012_dat;
          break;
        case 13:
          memptr = lev_lev0013_dat;
          break;
        case 14:
          memptr = lev_lev0014_dat;
          break;
        case 15:
          memptr = lev_lev0015_dat;
          break;
        case 16:
          memptr = lev_lev0016_dat;
          break;
        case 17:
          memptr = lev_lev0017_dat;
          break;
        case 18:
          memptr = lev_lev0018_dat;
          break;
        case 19:
          memptr = lev_lev0019_dat;
          break;
        case 20:
          memptr = lev_lev0020_dat;
          break;
        case 21:
          memptr = lev_lev0021_dat;
          break;
        case 22:
          memptr = lev_lev0022_dat;
          break;
        case 23:
          memptr = lev_lev0023_dat;
          break;
        case 24:
          memptr = lev_lev0024_dat;
          break;
        case 25:
          memptr = lev_lev0025_dat;
          break;
        case 26:
          memptr = lev_lev0026_dat;
          break;
        case 27:
          memptr = lev_lev0027_dat;
          break;
        case 28:
          memptr = lev_lev0028_dat;
          break;
        case 29:
          memptr = lev_lev0029_dat;
          break;
        case 30:
          memptr = lev_lev0030_dat;
          break;
        default:
          return;
      }
  }
  /* read initial playfield */
  z = 0;
  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
      game->field[x][y] = memptr[z++];
      if (((game->field[x][y] & field_type) == field_atom) || ((game->field[x][y] & field_type) == field_free)) {
        if ((game->cursorx == 0) && (game->cursory == 0)) {
          game->cursorx = x;
          game->cursory = y;
        }
      }
    }
  }
  /* read the solution */
  for (y = 0; y < 16; y++) {
    for (x = 0; x < 16; x++) {
      game->solution[x][y] = memptr[z++];
    }
  }
  /* load the duration time */
  game->duration = memptr[z++];
  game->duration <<= 8;
  game->duration |= memptr[z++];
  game->time_end = time(NULL) + game->duration;
  /* load level descriptions */
  for (x = 0; x < 15; x++) game->level_desc_line1[x] = memptr[z++];
  game->level_desc_line1[x] = 0;
  for (x = 0; x < 15; x++) game->level_desc_line2[x] = memptr[z++];
  game->level_desc_line2[x] = 0;
  /* load the cursor type */
  game->cursortype = memptr[z++];
  /* load the bg type */
  game->bg = memptr[z++];
  /* compute width/height of the playfield */
  for (x = 0; x < 16; x++) {
    for (y = 0; y < 16; y++) {
      if (((game->field[x][y] & field_type) == field_atom) || ((game->field[x][y] & field_type) == field_wall)) { /* got a wall or an atom */
        if (x + 1 > game->field_width) game->field_width = x + 1;
        if (y + 1 > game->field_height) game->field_height = y + 1;
      }
    }
  }
  /* adjust the vertical and horizontal offsets */
  game->offsetv = (15 - game->field_height) * 8;
  game->offseth +=  (15 - game->field_width) * 8;
  /* compute width/height of the solution */
  for (x = 0; x < 16; x++) {
    for (y = 0; y < 16; y++) {
      if (((game->solution[x][y] & field_type) == field_atom) || ((game->solution[x][y] & field_type) == field_wall)) { /* got a wall or an atom */
        if (x + 1 > game->solution_width) game->solution_width = x + 1;
        if (y + 1 > game->solution_height) game->solution_height = y + 1;
      }
    }
  }
  if (source == ATOMIX_SRC_FILE) free(memptr);
}


/* compares two games - the first is the playfield and the second is the expected solution. Returns 0 if game is not done, non-zero otherwise. */
int atomix_checksolution(struct atomixgame *game) {
  int x, y, xx, yy, win;
  if ((game->solution_width == 0) || (game->field_width == 0)) return(0); /* no solution is possible */
  for (x = 0; x <= game->field_width - game->solution_width; x++) {
    for (y = 0; y <= game->field_height - game->solution_height; y++) {
      win = 1; /* assume we are in a win position */
      for (xx = 0; xx < game->solution_width; xx++) {
        for (yy = 0; yy < game->solution_height; yy++) {
          if ((game->solution[xx][yy] & field_type) == field_atom) { /* compare only atoms */
            if (game->solution[xx][yy] != game->field[x+xx][y+yy]) {
              win = 0;
              break;
            }
          }
        }
        if (win == 0) break;
      }
      if (win == 1) return(1);
    }
  }
  return(0);
}
