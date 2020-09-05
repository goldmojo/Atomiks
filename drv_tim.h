/*
 * Timer driver for Atomiks
 * Copyright (C) Mateusz Viste 2014, 2015
 */

#ifndef drv_tim_h_sentinel
#define drv_tim_h_sentinel

/* waits until a specific time. */
void tim_wait_until_tick(unsigned long tick, long *overtime);

/* waits for a number of miliseconds */
void tim_delay(long ms);

/* returns the current ticks value */
long tim_getticks(void);

#endif
