#ifndef PLAY_H
#define PLAY_H

void Play_init(char* path);
void Play_change_scroll_speed(int diff);
void Play_update(double dt);
void Play_draw();

#endif
