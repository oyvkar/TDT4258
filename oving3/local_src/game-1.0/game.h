#ifndef GAME_H
#define GAME_H


typedef struct {
    uint16_t length;
    uint16_t width;
    uint16_t speed;   
    

    uint16_t oldXpos;
    uint16_t oldYpos;

    uint16_t Xpos;
    uint16_t Ypos;
}Playerbat_t;



void play();
void bat(void);
bool checkBatBounds(Playerbat_t *bat, bool up);
void moveBat(Playerbat_t *bat, bool up);
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
