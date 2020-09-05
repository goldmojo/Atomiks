/*
 * Sound driver for Atomiks
 * Copyright (C) Mateusz Viste 2014, 2015
 */

#include <stdio.h>    /* puts(), printf() */
#include <stdlib.h>   /* malloc(), free() */
#include <SDL2/SDL_mixer.h>

#include "drv_snd.h"  /* include self for control */

struct snd_wav {
  Mix_Chunk *ptr;
};

struct snd_mod {
  Mix_Music *ptr;
};


/* inits the sound system. returns 0 on success, non-zero otherwise. */
int snd_init(void) {
  if ((Mix_Init(MIX_INIT_MOD) & MIX_INIT_MOD) == 0) {
    puts(Mix_GetError());
  }
  if (Mix_OpenAudio(44100, AUDIO_S16SYS, 1, 2048) != 0) {
    puts(Mix_GetError());
    return(-1);
  }
  Mix_AllocateChannels(16);  /* allocate 16 mixing channels (we won't need that much anyway...) */
  return(0);
}

/* loads a WAVE file from memory. returns a pointer to the allocated struct. */
struct snd_wav *snd_loadwav(unsigned char *memptr, long memlen) {
  struct snd_wav *res;
  SDL_RWops *rwop;
  res = malloc(sizeof(struct snd_wav));
  if (res == NULL) return(NULL);
  rwop = SDL_RWFromMem(memptr, memlen);
  res->ptr = Mix_LoadWAV_RW(rwop, 0);
  SDL_FreeRW(rwop);
  return(res);
}


/* loads a MODule file from memory. returns a pointer to the allocated struct. */
struct snd_mod *snd_loadmod(unsigned char *memptr, long memlen) {
  struct snd_mod *res;
  SDL_RWops *rwop;
  res = malloc(sizeof(struct snd_mod));
  if (res == NULL) return(NULL);
  rwop = SDL_RWFromMem(memptr, memlen);
  res->ptr = Mix_LoadMUS_RW(rwop, 0);
  SDL_FreeRW(rwop);
  return(res);
}


/* plays a WAVE sample n+1 times (-1 for loop). returns a channel id, or -1 on error */
int snd_playwav(struct snd_wav *wav, int n) {
  return(Mix_PlayChannel(-1, wav->ptr, n));
}


/* plays the mod module n+1 times (-1 for loop), with a 'fade' miliseconds fade-in effect. returns 0 on success, non-zero otherwise. */
int snd_playmod(struct snd_mod *mod, int n, int fade) {
  return(Mix_FadeInMusic(mod->ptr, n, fade));
}


/* stops playing music, applying a n-miliseconds fadeout (0 for none) */
void snd_modstop(int n) {
  Mix_FadeOutMusic(n);
}


/* stops playing a WAVE on channel sndchannel, with fade out of n miliseconds */
void snd_wavstop(int sndchannel, int n) {
  Mix_FadeOutChannel(sndchannel, n);
}


/* free the memory used by a WAVE sample */
void snd_wavfree(struct snd_wav *wav) {
  if (wav != NULL) {
    Mix_FreeChunk(wav->ptr);
    free(wav);
  }
}


/* free the memory used by a MODule */
void snd_modfree(struct snd_mod *mod) {
  if (mod != NULL) {
    Mix_FreeMusic(mod->ptr);
    free(mod);
  }
}


/* closes the sound system */
void snd_close(void) {
  Mix_CloseAudio();
}
