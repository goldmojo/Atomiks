/*
 * Input (keyboard) driver for Atomiks
 * Copyright (C) Mateusz Viste 2014, 2015
 */

#include <time.h>
#include <SDL2/SDL.h>

#include "drv_tim.h"
#include "drv_inp.h"  /* include self for control */


static enum atomiks_keys sdlkey2atomiks(int sdlkey) {
  switch (sdlkey) {
    case SDLK_LEFT:
    case SDLK_KP_4:
      return(atomiks_left);
    case SDLK_RIGHT:
    case SDLK_KP_6:
      return(atomiks_right);
    case SDLK_UP:
    case SDLK_KP_8:
      return(atomiks_up);
    case SDLK_DOWN:
    case SDLK_KP_2:
      return(atomiks_down);
    #ifdef __GCW0__
    case SDLK_LCTRL: /*A*/
    case SDLK_LALT: /*B*/
    #else
    case SDLK_RETURN:
    case SDLK_KP_5:
    case SDLK_KP_ENTER:
      if (SDL_GetModState() & KMOD_ALT) return(atomiks_fullscreen);
    #endif
      return(atomiks_enter);
    #ifdef __GCW0__
    case SDLK_LSHIFT: /*Y*/
    case SDLK_TAB: /*L1*/
    #else
    case SDLK_HOME:
    case SDLK_KP_7:
    #endif
      return(atomiks_home);
    #ifdef __GCW0__
    case SDLK_SPACE: /*X*/
    case SDLK_BACKSPACE: /*R1*/
    #else
    case SDLK_END:
    case SDLK_KP_1:
    #endif
      return(atomiks_end);
    case SDLK_ESCAPE: /*SELECT*/
      return(atomiks_esc);
    #ifndef __GCW0__
    case SDLK_LALT: /* ALT presses shall be ignored */
    case SDLK_RALT:
      return(atomiks_none);
    #endif
    default:
      return(atomiks_unknown);
  }
}


void inp_flush_events(void) {
  SDL_Event event;
  while (SDL_PollEvent(&event) != 0);
}


/* Waits for a key up to timeout miliseconds, and returns the pressed key.
 * If timeout is negative, then only polling is performed.
 * Returns atomix_none if no key pressed. */
enum atomiks_keys inp_waitkey(int timeout) {
  SDL_Event event;
  int evres;
  long timeouttime;
  timeouttime = tim_getticks() + timeout;
  for (;;) {
    if (timeout > 0) {
      evres = SDL_WaitEventTimeout(&event, 250);
    } else {
      evres = SDL_PollEvent(&event);
    }
    if (evres != 0) {
      if (event.type == SDL_QUIT) {
          return(atomiks_quit);
        } else if (event.type == SDL_KEYDOWN) {
          return(sdlkey2atomiks(event.key.keysym.sym));
        } else if (event.type == SDL_WINDOWEVENT) { /* might be an indicator of lost/gained focus */
          if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) { /* lost focus */
              return(atomiks_lostfocus);
            } else if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) { /* gained focus */
              return(atomiks_gotfocus);
          }
      }
    }
    if (timeout < 0) return(atomiks_none);
    if ((timeout > 0) && (tim_getticks() >= timeouttime)) return(atomiks_none);
  }
}
