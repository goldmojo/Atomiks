/*
 * This file is part of the Atomiks project
 * Copyright (C) Mateusz Viste 2013, 2014, 2015
 *
 * opens and provides a file descriptor to the
 * application's configuration file. multiplatform.
 */


#ifndef cfg_h_sentinel
  #define cfg_h_sentinel
  /* mode is the fopen file mode to use (eg. "rb"). appname is the short name of your app (eg. "atomiks"). Returns an open FD ready to use, or NULL on failure */
  FILE *cfg_fopen(char *mode, char *appname);
#endif
