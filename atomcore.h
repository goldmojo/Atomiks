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

#ifndef atomcore_h_sentinel
#define atomcore_h_sentinel

  #define field_free 128
  #define field_atom 64
  #define field_wall 192
  #define field_type 192
  #define field_index 63

  #define ATOMIX_SRC_FILE 1
  #define ATOMIX_SRC_MEM 2

  struct atomixgame {
    unsigned char field_width;
    unsigned char field_height;
    unsigned char solution_width;
    unsigned char solution_height;
    unsigned char field[64][64]; /* every byte is composed of few parts: ffiiiiii where ff are type flags, and iiiiii is the index */
    unsigned char solution[32][32];
    unsigned char cursorx;
    unsigned char cursory;
    unsigned char cursorstate; /* 0 = normal cursor / 1 = selected cursor */
    int level;
    time_t time_end;
    unsigned int duration;
    int offseth;
    int offsetv;
    char level_desc_line1[16];
    char level_desc_line2[16];
    unsigned char cursortype;  /* 0 = normal cursor / 1 = bonus-like cursor */
    int bg;
    int score;
    int hiscore;
  };

  struct atomixgame *atomix_initgame(void);

  void atomix_loadgame(struct atomixgame *game, int level, int source, int *hiscores);

  /* returns the distance that the block at position x/y would travel if pushed into 'direction'. direction is 0: up / 1: right / 2: down / 3: left */
  int atomix_getmovedistance(struct atomixgame *game, int direction);

  /* compares two games - the first is the playfield and the second is the expected solution. Returns 0 if game is not done, non-zero otherwise. */
  int atomix_checksolution(struct atomixgame *game);

#endif
