/*
 * gzBMP - a SDL2 helper that provides support for gziped BMP files.
 * Copyright (C) Mateusz Viste 2014, 2015
 */

#ifndef gz_h_sentinel
#define gz_h_sentinel
  unsigned char *ungz(unsigned char *memgz, long memgzlen, long *resultlen);
  int isGz(unsigned char *memgz, long memgzlen);
#endif
