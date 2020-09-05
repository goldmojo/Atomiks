/*
 * This file is part of the Atomiks project
 * Copyright (C) Mateusz Viste 2013, 2014, 2015
 *
 * opens and provides a file descriptor to the
 * application's configuration file. multiplatform.
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

#include <stdio.h>     /* FILE     */
#include <stdlib.h>    /* NULL     */
#include <SDL2/SDL.h>  /* SDL_GetPrefPath(), SDL_free() */

/* mode is the fopen file mode to use (eg. "rb"). appname is the short name
 * of your app (eg. "atomiks"). Returns an open FD ready to use, or NULL on
 * failure */
FILE *cfg_fopen(char *mode, char *appname) {
  char *prefpath;
  char filepath[4096];
  FILE *fd;
  if ((mode == NULL) || (appname == NULL)) return(NULL);
  prefpath = SDL_GetPrefPath("Mateusz Viste", appname);
  snprintf(filepath, 4096, "%s/config.dat", prefpath);
  SDL_free(prefpath);
  fd = fopen(filepath, mode);
  return(fd);
}
