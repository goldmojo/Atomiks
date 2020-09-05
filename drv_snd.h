/*
 * Sound driver for Atomiks
 * Copyright (C) Mateusz Viste 2014, 2015
 */


#ifndef drv_snd_h_sentinel
#define drv_snd_h_sentinel

struct snd_wav;

struct snd_mod;


/* inits the sound system. returns 0 on success, non-zero otherwise. */
int snd_init(void);

/* loads a WAVE file from memory. returns a pointer to the allocated struct. */
struct snd_wav *snd_loadwav(unsigned char *memptr, long memlen);

/* loads a MODule file from memory. returns a pointer to the allocated struct. */
struct snd_mod *snd_loadmod(unsigned char *memptr, long memlen);

/* plays a WAVE sample n+1 times (-1 for loop). returns a channel id, or -1 on error */
int snd_playwav(struct snd_wav *wav, int n);

/* plays the mod module n+1 times (-1 for loop), with a 'fade' miliseconds fade-in effect. returns 0 on success, non-zero otherwise. */
int snd_playmod(struct snd_mod *mod, int n, int fade);

/* stops playing music, applying a n-miliseconds fadeout (0 for none) */
void snd_modstop(int n);

/* stops playing a WAVE on channel sndchannel, with fade out of n miliseconds */
void snd_wavstop(int sndchannel, int n);

/* this should be called from time to time, to let the library do its work */
void snd_update(void);

/* free the memory used by a WAVE sample */
void snd_wavfree(struct snd_wav *wav);

/* free the memory used by a MODule */
void snd_modfree(struct snd_mod *mod);

/* closes the sound system */
void snd_close(void);

#endif
