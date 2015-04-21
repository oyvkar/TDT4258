#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <math.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "game.h"


//Game variables
struct playfield{
    uint16_t height; //Using uint16 here to be on the safe side, some of these could be uint8 to reduce memory strain
    uint16_t width;
}playfield_a;

struct playerbat{
    uint16_t length;
    uint16_t width;
    uint16_t speed;   
    

    uint16_t oldXpos;
    uint16_t oldYpos;

    uint16_t Xpos;
    uint16_t Ypos;
}playerbat_a,playerbat_b;

struct ball{
    uint16_t radius;

    uint16_t Xspeed;
    uint16_t Yspeed;
    
    uint16_t oldXpos;
    uint16_t oldYpos;
    uint16_t Xpos;
    uint16_t Ypos;
}ball_a;

struct score{
    int playerAscore;
    int playerBscore;
}gamescore;

uint16_t white = 0xffff;
uint16_t black = 0;
FILE* gamepad;
long oflags;
int input_a;
int input_b;

uint16_t *screen;
struct fb_copyarea rect;
struct fb_var_screeninfo screen_info;
int fb;
int screen_size;


int main(int argc, char *argv[])
{
	printf("Hello World, I'm game!\n");
    open_controller();    
    initialize_screen();//Initializes the screen
    play();
    printf("Done playing \n");
    close_controller();
    close_screen();
    exit(EXIT_SUCCESS);
    return 0;
}

void play(){
    initialize(true);
    while(gamescore.playerAscore < 3 && gamescore.playerBscore < 3){
        movebat();
        moveball();
        handlePhysics();
        update_screen();  // After all updates to the game have been made, update the display
    }
}

void movebat(void){
    switch (input_a) {
        case 1: //Move left bat up
            if (playerbat_a.Ypos > 0) {   //Checks that the bat doesn't move beyond the screen
                draw_rectangle(playerbat_a.oldXpos,playerbat_a.oldYpos,playerbat_a.width,playerbat_a.length, black, false);  //Erase the old position
                playerbat_a.oldYpos = playerbat_a.Ypos;
                playerbat_a.Ypos = playerbat_a.Ypos - playerbat_a.speed;
           }
            break;
        case 2: //Move left bat down
            if (playerbat_a.Ypos + playerbat_a.length < playfield_a.height) {   //Checks that the bat doesn't move beyond the screen
                draw_rectangle(playerbat_a.oldXpos,playerbat_a.oldYpos,playerbat_a.width,playerbat_a.length, black, false);  //Erase the old position 
                playerbat_a.oldYpos = playerbat_a.Ypos;
                playerbat_a.Ypos = playerbat_a.Ypos + playerbat_a.speed;
               }
            break;
        default:
            break;
    }
    switch (input_b) {
        case 1: //Move right bat up
            if(playerbat_b.Ypos > 0) {    
                draw_rectangle(playerbat_b.oldXpos,playerbat_b.oldYpos,playerbat_b.width,playerbat_b.length, black, false);  //Erase the old position
                playerbat_b.oldYpos = playerbat_b.Ypos;
                playerbat_b.Ypos = playerbat_b.oldYpos - playerbat_b.speed;
                }
           break;
        case 2: //Move right bat down
            if (playerbat_b.Ypos + playerbat_b.length < playfield_a.height) {   //Checks that the bat doesn't move beyond the screen
                draw_rectangle(playerbat_b.oldXpos,playerbat_b.oldYpos,playerbat_b.width,playerbat_b.length, black, false);  //Erase the old position
                playerbat_b.oldYpos = playerbat_b.Ypos;
                playerbat_b.Ypos = playerbat_b.oldYpos + playerbat_b.speed;
           }
            break;
        default:
            break;
            
        }
    draw_rectangle(playerbat_a.oldXpos,playerbat_a.oldYpos,playerbat_a.width,playerbat_a.length, white, false); // Draws the new positions for the paddles
    draw_rectangle(playerbat_b.oldXpos,playerbat_b.oldYpos,playerbat_b.width,playerbat_b.length, white, false);
    return;


}
void moveball(){

    //Draw a black ball to erase the current ball
    draw_rectangle(ball_a.oldXpos-ball_a.radius, ball_a.oldYpos+ball_a.radius, ball_a.radius*2 + 1,ball_a.radius*2 + 1, black, false);

    //Update current position
    ball_a.oldXpos = ball_a.Xpos;
    ball_a.oldYpos = ball_a.Ypos;
    //Calculate next position
    ball_a.Xpos = ball_a.Xpos + ball_a.Xspeed;
    ball_a.Ypos = ball_a.Ypos + ball_a.Yspeed;

    //Draw a new white ball
    draw_rectangle(ball_a.oldXpos-ball_a.radius, ball_a.oldYpos+ball_a.radius, ball_a.radius*2 + 1,ball_a.radius*2 + 1, white,false);
 
    //Handle some ball collisions
    if(ball_a.Ypos + ball_a.radius < 30)ball_a.Yspeed = -ball_a.Yspeed; //Bounces the ball from the top
    if(ball_a.Ypos - ball_a.radius > playfield_a.height - 30)ball_a.Yspeed = -ball_a.Yspeed; // Bounces the ball from the bottom
}

void handlePhysics(){
    if (ball_a.Xpos - ball_a.radius <= 30) {   //Checks if the ball hits the bat
        //printf("Ball YPOS: %i\tBat YPOS: %i\tBat len: %i\n", ball_a.Ypos, playerbat_a.Ypos, playerbat_a.length);
       // if ((ball_a.Ypos < playerbat_a.Ypos) && (ball_a.Ypos > (playerbat_a.Ypos + playerbat_a.length))) {
       if ((playerbat_a.Ypos <= ball_a.Ypos) && (ball_a.Ypos <= (playerbat_a.Ypos + playerbat_a.length)))  {
            ball_a.Xspeed = -ball_a.Xspeed; //Ball was hit, reverse the speed
            ball_a.Yspeed = rand()%4;//Random Y speed
            if(rand()%2 == 0)ball_a.Yspeed = -ball_a.Yspeed;//Random Y direction
        }
        else{  //If the ball does not hit the bat, score is awarded
           gamescore.playerBscore ++; 
           initialize(false);
        }
        return;
    }
    if (ball_a.Xpos + ball_a.radius >= 295) {   //Checks if the ball hits the bat
            if ((playerbat_b.Ypos <= ball_a.Ypos) && (ball_a.Ypos <= (playerbat_b.Ypos + playerbat_b.length)))  {
                ball_a.Xspeed = -ball_a.Xspeed; //Ball was hit, reverse the speed
                ball_a.Yspeed = rand()%4;//Random Y speed
                if(rand()%2 == 0)ball_a.Yspeed = -ball_a.Yspeed;//Random Y direction
            }
             else{  //If the ball does not hit the bat, score is awarded
                gamescore.playerAscore ++; 
                initialize(false);
            }
         return;
        }   
}


void initialize(bool first)
{
    // Starts the game with the ball in the centre of the playfield, and the ball moving towards the player who won the previous round
    printf("Initializing game\n");
    playfield_a.height = 239;
    playfield_a.width = 319;
    
    ball_a.radius = 5;
    
    ball_a.Xspeed = 6;
    if(rand()%2 == 0)ball_a.Xspeed = -ball_a.Xspeed;
    ball_a.Yspeed = 0;
    ball_a.Ypos = 119;  //Starts the ball in the centre
    ball_a.oldYpos = ball_a.Ypos;
    ball_a.Xpos = 159;
    ball_a.oldXpos = ball_a.Xpos;

    playerbat_a.length = 60;
    playerbat_a.width = 10;
    playerbat_b.length = 60;
    playerbat_b.width = 10;
    playerbat_a.Xpos = 20;
    playerbat_a.oldXpos = playerbat_a.Xpos;
    playerbat_a.Ypos = 119;
    playerbat_a.oldYpos = playerbat_a.Ypos;
    playerbat_b.Xpos = 300;
    playerbat_b.oldXpos = playerbat_b.Xpos;
    playerbat_b.Ypos = 119;
    playerbat_b.oldYpos = playerbat_b.Ypos;
    playerbat_a.speed = 5;
    playerbat_b.speed = 5;
    
    if(first){
        gamescore.playerAscore = 0;
        gamescore.playerBscore = 0;
    }
    single_color(0);//sets the playfield to black
    draw_rectangle(playerbat_a.Xpos,playerbat_a.Ypos,playerbat_a.width,playerbat_a.length, white, false);
    draw_rectangle(playerbat_b.Xpos,playerbat_b.Ypos,playerbat_b.width,playerbat_b.length, white, false);
}

void open_controller(){
    gamepad = fopen("/dev/gamepad", "rb");
    if(!gamepad){
        printf("Failed to open gamepad driver! Exiting\n");
        exit(EXIT_FAILURE);
    }
     
    if(signal(SIGIO, &input_handler) == SIG_ERR){
        printf("Failed to create signal handler\n");
        exit(EXIT_FAILURE);
    }
    if(fcntl(fileno(gamepad), F_SETOWN, getpid()) == -1){
        printf("Faild to set owner\n");
        exit(EXIT_FAILURE);
    }
    long oflags = fcntl(fileno(gamepad), F_GETFL);
    if(fcntl(fileno(gamepad), F_SETFL, oflags | FASYNC) == -1){
        printf("Error setting FASYNC flag\n");
        exit(EXIT_FAILURE);
    }    
}

void close_controller(){
    fclose(gamepad);
}

void close_screen() {
    close(fb);
}

void input_handler(int singal_no){
    char buffer[30];
    while ( fgets(buffer, 30, gamepad) != NULL ) {
       if (strlen(buffer) != 7)
            break;
        if ((buffer[2] == '2') && (buffer[5] == '1') )  input_a = 1;
        if ((buffer[2] == '2') && (buffer[5] == '0') )  input_a = 0;
        if ((buffer[2] == '4') && (buffer[5] == '1') )  input_a = 2;
        if ((buffer[2] == '4') && (buffer[5] == '0') )  input_a = 0;
        if ((buffer[2] == '6') && (buffer[5] == '1') )  input_b = 1;
        if ((buffer[2] == '6') && (buffer[5] == '0') )  input_b = 0;
        if ((buffer[2] == '8') && (buffer[5] == '1') )  input_b = 2;
        if ((buffer[2] == '8') && (buffer[5] == '0') )  input_b = 0;
    }
}
void initialize_screen(){
    
    rect.dx = 0;
    rect.dy = 0;
    rect.width = 320;
    rect.height = 240;
    
    fb = open("/dev/fb0", O_RDWR);
    if (!fb) {
        printf("Could not open file containing framebuffer device \n");
        exit(EXIT_FAILURE);
    }
    if(ioctl(fb, FBIOGET_VSCREENINFO, &screen_info) == -1){
        printf("Could not aquire screen info \n");
        exit(EXIT_FAILURE);
    }


    screen_size = (320*240) * screen_info.bits_per_pixel/8;


    screen = (uint16_t*) mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    printf("Screen successfully initialized\n");
}

void update_screen(){
    ioctl(fb,0x4680,&rect);
    return;
}
void single_color(uint16_t color){
    int i;
    for (i = 0; i < 320*260; i++) {
        screen[i] = color;
    }
    update_screen();
}

void draw_rectangle(int Xpos, int Ypos,int width, int height, uint16_t color, bool do_update){
    int i, j;
    for (i = Xpos; i < width + Xpos; i++) {
        for (j = height + Ypos; j > Ypos; j--) {
            screen[i+j*320] = color;
        }
    }
    if (do_update) {
        update_screen();
    }
}
