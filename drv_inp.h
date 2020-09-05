/*
 * Input (keyboard) driver for Atomiks
 * Copyright (C) Mateusz Viste 2014, 2015
 */

#ifndef drv_inp_h_sentinel
#define drv_inp_h_sentinel

enum atomiks_keys {
  atomiks_none,
  atomiks_left,
  atomiks_right,
  atomiks_up,
  atomiks_down,
  atomiks_home,
  atomiks_end,
  atomiks_esc,
  atomiks_enter,
  atomiks_quit,
  atomiks_gotfocus,
  atomiks_lostfocus,
  atomiks_fullscreen,
  atomiks_unknown
};


/* flush input buffers */
void inp_flush_events(void);

/* Waits for a key up to timeout seconds, and returns the pressed key.
 * If timeout is negative, then only polling is performed.
 * Returns atomix_none if no key pressed. */
enum atomiks_keys inp_waitkey(int timeout);

#endif
