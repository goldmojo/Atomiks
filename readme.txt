

 Atomiks - A remake of the classic Atomix game for modern platforms
 Copyright (C) Mateusz Viste 2013, 2014, 2015


 *** Introduction ***

Atomiks is a faithful remake of, and a tribute to, Atomix, a classic puzzle game created by Softtouch & RoSt and released in 1990 by the Thalion Software company. Atomiks is free software, and shares no code with the original Atomix game.


 *** What is Atomix? ***

Extract borrowed from Wikipedia, the free encyclopedia:
"Atomix takes place on a playfield, consisting of a number of walls, with the atoms scattered throughout. The player is tasked with assembling a molecule from the atoms; more specifically, the atoms must be arranged into a specific shape, identical with the shape of the molecule displayed on the left side of the screen. The player can choose an atom and move it in any of the four cardinal directions; however, a moved atom keeps sliding in one direction until it hits a wall or another atom. Solving the puzzles requires strategic planning in moving the atoms, and on later levels with little free space, even finding room for the completed molecule can be a problem."


 *** Requirements ***

Atomiks is writen with care about portability, therefore it should build on most modern platform without much hassle. It requires the following libraries to run: SDL, SDL_mixer and libmikmod.

DOS compatibility note: My version of Atomix can't be easily compiled to a native DOS binary, mostly because there's no SDL port for DOS. But you can use the Win32 binary in DOS using the HX DOS Extender.


 *** Configuration ***

The game does not require any configuration. However, it accepts a few command-line parameters:
  --fullscreen     - Run Atomiks in fullscreen mode (default is windowed mode)
  --nosound        - Disable sound


 *** License ***

The Atomiks engine is released under the GNU/GPL license, altough this license does NOT apply to level design and graphics used by Atomiks, since these remain the intellectual property of their authors, Softtouch & RoSt. Therefore you CAN'T reuse any of the level design or graphic elements, unless you get written permission from Atomix's copyright holders.

Atomiks has been created by Mateusz Viste, who has been explicitely authorized to reuse graphical and design elements from the original game by Atomix's copyright holders for the sole purpose of creating Atomiks.
