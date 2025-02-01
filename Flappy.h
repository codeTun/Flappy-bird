#ifndef FLAPPY_H
#define FLAPPY_H

#include <SDL.h>  
#include <SDL_ttf.h>  


void setup();
void new_game();
void game_over_func();
void update_stuff();
void update_pipe(int i);
void render_text(const char *msg, int height);
void render_score(int current_score, int height);
void render_high_score(int high_score, int height);
void draw_stuff();

#endif 