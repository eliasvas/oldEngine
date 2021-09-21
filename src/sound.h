#ifndef SOUND_H
#define SOUND_H

//we implement these functions in our platform layer
//TODO(ilias): make them part of the asset system (boring)
extern void play_sound(char *path);
extern void play_sound_loop(char *path);
#endif
