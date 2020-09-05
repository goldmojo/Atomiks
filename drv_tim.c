/*
 * Timer driver for Atomiks
 * Copyright (C) Mateusz Viste 2014, 2015
 */

#include <SDL2/SDL.h>

#include "drv_tim.h" /* include self for control */


/* waits until a specific time. */
void tim_wait_until_tick(unsigned long tick, long *overtime) {
  if (overtime != NULL) tick -= *overtime;
  while (SDL_GetTicks() < tick) {
    SDL_Delay(1);
  }
  if (overtime != NULL) *overtime = SDL_GetTicks() - tick;
}


/* waits for a number of miliseconds */
void tim_delay(long ms) {
  unsigned long waituntil;
  waituntil = SDL_GetTicks() + ms;
  tim_wait_until_tick(waituntil, NULL);
}


/* returns the current ticks value */
long tim_getticks(void) {
  return(SDL_GetTicks());
}
