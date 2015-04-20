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


int main(int argc, char *argv[])
{
	printf("Hello World, I'm game!\n");
    play();
    printf("Done playing \n");
	exit(EXIT_SUCCESS);
    return 0;
}

void play(){
    initialize(true);
    while(gamescore.playerAscore < 3 && gamescore.playerBscore < 3){
        //TODO:
        //Handle Inputs
    //    if(input == upLeft) movebat(0);
    //    if (input == downLeft) movebat(1);
    //    if(input == upRight) movebat(2);
    //    if(input == downRight) movebat(3);
       // single_color(rand()%256);
        moveball();
        handlePhysics();
        draw_rectangle(playerbat_a.Xpos,playerbat_a.Ypos,playerbat_a.width,playerbat_a.length, white, false);
        draw_rectangle(playerbat_b.Xpos,playerbat_b.Ypos,playerbat_b.width,playerbat_b.length, white, true); //Waits until all commits to screen have been added, then calls the update
   
        //sleep(0.01);  // Value does not represent real time
    }
}

void movebat(int input){
    switch (input) {
        case 0: //Move left bat up
            if (playerbat_a.Ypos > 0) {   //Checks that the bat doesn't move beyond the screen
                //Draw a black rectangle to erase the previous position
                //draw_rectangle(playerbat_a.oldXpos,playerbat_a.oldYpos,playerbat_a.width,playerbat_a.length, black);
                playerbat_a.oldYpos = playerbat_a.Ypos;
                playerbat_a.Ypos --;
                //Draw a white rectangle a the new position
                //draw_rectangle(playerbat_a.oldXpos,playerbat_a.oldYpos,playerbat_a.width,playerbat_a.length, white);
            }
            break;
        case 1: //Move left bat down
            if (playerbat_a.Ypos + playerbat_a.length < playfield_a.height) {   //Checks that the bat doesn't move beyond the screen
               
                //Draw a black rectangle to erase the previous position
                //draw_rectangle(playerbat_a.oldXpos,playerbat_a.oldYpos,playerbat_a.width,playerbat_a.length, black);
                playerbat_a.oldYpos = playerbat_a.Ypos;
                playerbat_a.Ypos ++;
                //Draw a white rectangle a the new position
                //draw_rectangle(playerbat_a.oldXpos,playerbat_a.oldYpos,playerbat_a.width,playerbat_a.length, white);  
                }
            break;
        case 2: //Move right bat up
                //Draw a black rectangle to erase the previous position
                //draw_rectangle(playerbat_b.oldXpos,playerbat_b.oldYpos,playerbat_b.width,playerbat_b.length, black);
                playerbat_b.oldYpos = playerbat_b.Ypos;
                playerbat_b.Ypos --;
                //Draw a white rectangle a the new position
                //draw_rectangle(playerbat_b.oldXpos,playerbat_b.oldYpos,playerbat_b.width,playerbat_b.length, white);
            break;
        case 3: //Move right bat down
            if (playerbat_b.Ypos + playerbat_b.length < playfield_a.height) {   //Checks that the bat doesn't move beyond the screen
               
                //Draw a black rectangle to erase the previous position
                //draw_rectangle(playerbat_b.oldXpos,playerbat_b.oldYpos,playerbat_b.width,playerbat_b.length, black);  
                playerbat_b.oldYpos = playerbat_b.Ypos;
                playerbat_b.Ypos ++;
                //Draw a white rectangle a the new position
                //draw_rectangle(playerbat_b.oldXpos,playerbat_b.oldYpos,playerbat_b.width,playerbat_b.length, white);
            }
            break;
        default:
            break;
            
        }
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
        if (ball_a.Ypos < playerbat_a.Ypos && ball_a.Ypos < (playerbat_a.Ypos - playerbat_a.length)) {
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
            if (ball_a.Ypos < playerbat_b.Ypos && ball_a.Ypos <( playerbat_b.Ypos - playerbat_b.length)) {
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
    ball_a.Xpos = 159;

    playerbat_a.length = 90;
    playerbat_a.width = 15;
    playerbat_b.length = 90;
    playerbat_b.width = 15;
    playerbat_a.Xpos = 20;
    playerbat_a.Ypos = 119;
    playerbat_b.Xpos = 300;
    playerbat_b.Ypos = 119;
    playerbat_a.speed = 5;
    playerbat_b.speed = 5;
    
    if(first){
        gamescore.playerAscore = 0;
        gamescore.playerBscore = 0;
        initialize_screen();//Initializes the screen
    }
    single_color(0);//sets the playfield to black
    draw_rectangle(playerbat_a.Xpos,playerbat_a.Ypos,playerbat_a.width,playerbat_a.length, white, false);
    draw_rectangle(playerbat_b.Xpos,playerbat_b.Ypos,playerbat_b.width,playerbat_b.length, white, true);
}

uint16_t *screen;
struct fb_copyarea rect;
struct fb_var_screeninfo screen_info;
int fd;
int screen_size;

void initialize_screen(){
    
    rect.dx = 0;
    rect.dy = 0;
    rect.width = 320;
    rect.height = 240;
    
    fd = open("/dev/fb0", O_RDWR);
    if (!fd) {
        printf("Could not open file containing framebuffer device \n");
        exit(EXIT_FAILURE);
    }
    if(ioctl(fd, FBIOGET_VSCREENINFO, &screen_info) == -1){
        printf("Could not aquire screen info \n");
        exit(EXIT_FAILURE);
    }


    screen_size = (320*240) * screen_info.bits_per_pixel/8;


    screen = (uint16_t*) mmap(NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    printf("Screen successfully initialized\n");
}

void update_screen(){
    ioctl(fd,0x4680,&rect);
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


