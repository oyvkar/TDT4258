#ifndef GAME_H
#define GAME_H

void play();
void movebat();
void moveball();
void handlePhysics();
void initialize(bool first);
void initialize_screen();
void update_screen();
void single_color(uint16_t color);
void draw_rectangle(int Xpos, int Ypos, int width, int height, uint16_t color, bool do_update);
void input_handler();
void open_controller();
void close_controller();
void close_screen();
#endif
