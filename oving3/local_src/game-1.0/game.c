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

Playerbat_t playerbat[2];

typedef struct {
    uint16_t radius;

    uint16_t Xspeed;
    uint16_t Yspeed;
    
    uint16_t oldXpos;
    uint16_t oldYpos;
    uint16_t Xpos;
    uint16_t Ypos;
}Ball_t;

Ball_t ball;

struct score{
    int playerAscore;
    int playerBscore;
}gamescore;

const uint16_t white = 0xffff;
const uint16_t black = 0;
FILE* gamepad;
long oflags;
volatile int input[2];

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
        bat();
        moveball();
        handlePhysics();
        nanosleep((struct timespec[]){{0, 100000000}}, NULL);
    }
}

void bat(void){
    int i;
    for(i=0; i < 2; i++) {
        switch (input[i]) {
            case 1: //Move  bat up
                moveBat(&playerbat[i],0);
                break;
            case 2: //Move bat down
                moveBat(&playerbat[i],1);
                break;
        }
    }
}

bool checkBatBounds(Playerbat_t *bat, bool down) {
    if (down) 
        return (bat->Ypos + bat->length + bat->speed < playfield_a.height);   //Checks that the bat doesn't move beyond the screen
    
    return ((bat->Ypos - bat->speed) > 0);  //Checks that the bat doesn't move beyond the screen
}
void moveBat(Playerbat_t *bat, bool down) {
    if ( !checkBatBounds(bat, down))
        return;

    if(down) {
        draw_rectangle(bat->Xpos, bat->Ypos + bat->speed, bat->width, bat->speed, black);
        draw_rectangle(bat->Xpos, bat->Ypos + bat->length + bat->speed, bat->width, bat->speed, white);
        bat->oldYpos = bat->Ypos;
        bat->Ypos += bat->speed;
    } else {
        printf("down: %i\n",down);
        printf("Bat: ypos: %i\tLength: %i\tSpeed: %i\n", bat->Ypos, bat->width, bat->speed);
        printf("ypos + len: %i\typos+len+speed %i\typos-len %i\n", bat->Ypos + bat->length, bat->Ypos + bat->length + bat->speed, bat->Ypos - bat->length);
        draw_rectangle(bat->Xpos, bat->Ypos, bat->width, bat->length, black);
        bat->Ypos -= bat->speed;
        draw_rectangle(bat->Xpos, bat->Ypos, bat->width, bat->length, white);
    }
}

void moveball(){

    //Draw a black ball to erase the current ball
    draw_rectangle(ball.oldXpos-ball.radius, ball.oldYpos+ball.radius, ball.radius*2 + 1,ball.radius*2 + 1, black);

    //Update current position
    ball.oldXpos = ball.Xpos;
    ball.oldYpos = ball.Ypos;
    //Calculate next position
    ball.Xpos = ball.Xpos + ball.Xspeed;
    ball.Ypos = ball.Ypos + ball.Yspeed;

    //Draw a new white ball
    draw_rectangle(ball.oldXpos-ball.radius, ball.oldYpos+ball.radius, ball.radius*2 + 1,ball.radius*2 + 1, white);
 
    //Handle some ball collisions
    if(ball.Ypos + ball.radius < 30)ball.Yspeed = -ball.Yspeed; //Bounces the ball from the top
    if(ball.Ypos - ball.radius > playfield_a.height - 30)ball.Yspeed = -ball.Yspeed; // Bounces the ball from the bottom
}

void handlePhysics(){
    if (ball.Xpos - ball.radius <= 30) {   //Checks if the ball hits the bat
       if ((playerbat[0].Ypos <= ball.Ypos) && (ball.Ypos <= (playerbat[0].Ypos + playerbat[0].length)))  {
            ball.Xspeed = -ball.Xspeed; //Ball was hit, reverse the speed
            ball.Yspeed = rand()%4;//Random Y speed
            if(rand()%2 == 0)ball.Yspeed = -ball.Yspeed;//Random Y direction
        }
        else{  //If the ball does not hit the bat, score is awarded
           gamescore.playerBscore ++; 
           initialize(false);
        }
        return;
    }
    if (ball.Xpos + ball.radius >= 295) {   //Checks if the ball hits the bat
            if ((playerbat[1].Ypos <= ball.Ypos) && (ball.Ypos <= (playerbat[1].Ypos + playerbat[1].length)))  {
                ball.Xspeed = -ball.Xspeed; //Ball was hit, reverse the speed
                ball.Yspeed = rand()%4;//Random Y speed
                if(rand()%2 == 0)ball.Yspeed = -ball.Yspeed;//Random Y direction
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
    
    ball.radius = 5;
    
    ball.Xspeed = 6;
    if(rand()%2 == 0)
        ball.Xspeed = -ball.Xspeed;
    ball.Yspeed = 0;
    ball.Ypos = 119;  //Starts the ball in the centre
    ball.oldYpos = ball.Ypos;
    ball.Xpos = 159;
    ball.oldXpos = ball.Xpos;

    playerbat[0].length = 60;
    playerbat[0].width = 10;
    playerbat[1].length = 60;
    playerbat[1].width = 10;
    playerbat[0].Xpos = 20;
    playerbat[0].oldXpos = playerbat[0].Xpos;
    playerbat[0].Ypos = 119;
    playerbat[0].oldYpos = playerbat[0].Ypos;
    playerbat[1].Xpos = 300;
    playerbat[1].oldXpos = playerbat[1].Xpos;
    playerbat[1].Ypos = 119;
    playerbat[1].oldYpos = playerbat[1].Ypos;
    playerbat[0].speed = 5;
    playerbat[1].speed = 5;
    
    if(first){
        gamescore.playerAscore = 0;
        gamescore.playerBscore = 0;
    }
    single_color(0);//sets the playfield to black
    draw_rectangle(playerbat[0].Xpos,playerbat[0].Ypos,playerbat[0].width,playerbat[0].length, white);
    draw_rectangle(playerbat[1].Xpos,playerbat[1].Ypos,playerbat[1].width,playerbat[1].length, white);
    update_screen();
}



// IO related functions below this line //

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

void input_handler(int singal_no){
    char buffer[30];
    while ( fgets(buffer, 30, gamepad) != NULL ) {
       if (strlen(buffer) != 7)
            break;
        if ((buffer[2] == '2') && (buffer[5] == '1') )  input[0] = 1;
        if ((buffer[2] == '2') && (buffer[5] == '0') )  input[0] = 0;
        if ((buffer[2] == '4') && (buffer[5] == '1') )  input[0] = 2;
        if ((buffer[2] == '4') && (buffer[5] == '0') )  input[0] = 0;
        if ((buffer[2] == '6') && (buffer[5] == '1') )  input[1] = 1;
        if ((buffer[2] == '6') && (buffer[5] == '0') )  input[1] = 0;
        if ((buffer[2] == '8') && (buffer[5] == '1') )  input[1] = 2;
        if ((buffer[2] == '8') && (buffer[5] == '0') )  input[1] = 0;
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

void close_screen() {
    close(fb);
}

void update_screen(){
    ioctl(fb,0x4680,&rect);
    return;
}
void single_color(uint16_t color){
    int i;
    for (i = 0; i < 320*240; i++) {
        screen[i] = color;
    }
    update_screen();
}

void draw_rectangle(int Xpos, int Ypos,int width, int height, uint16_t color){
    int i, j;
    for (i = Xpos; i < width + Xpos; i++) {
        for (j = height + Ypos; j > Ypos; j--) {
            screen[i+j*320] = color;
        }
    }
    printf("DRAW: Xpos: %i\t Ypos: %i\t width: %i\t height: %i\n", Xpos, Ypos, width, height);
    struct fb_copyarea update;
    update.dx = Xpos;
    update.dy = Ypos;
    update.width = width;
    update.height = height;
    ioctl(fb,0x4680,&update);
}
