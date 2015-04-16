#include <stdio.h>
#include <stdlib.h>



//Game variables
struct playfield{
    uint32_t height;
    uint32_t width;
}playfield_a;

struct playerbat{
    uint32_t length;
    uint32_t width;
    uint32_t speed;   // How many pixels the bat
    

    uint32_t oldXpos;
    uint32_t oldYpos;

    uint32_t Xpos;
    uint32_t Ypos;
}playerbat_a,playerbat_b;

struct ball{
    uint32_t radius;

    uint32_t Xspeed;
    uint32_t Yspeed;
    
    uint32_t oldXpos;
    uint32_t oldYpos;
    uint32_t Xpos;
    uint32_t Ypos;
}ball_a;

struct score{
    int playerAscore;
    int playerBscore;
}gamescore;




int main(int argc, char *argv[])
{
	printf("Hello World, I'm game!\n");
    play();
	exit(EXIT_SUCCESS);
    return 0;
}

void play(){
    initialize(true);
    while(playerAscore < 3 && playerBscore < 3){
    //TODO: Generate a matrix from the current positions of the ball and the bats, then use the draw function with the matrix to generate an image on the screen
    //Handle scoring 
    if(input == upLeft) movebat(0);
    if (input == downLeft) movebat(1);
    if(input == upRight) movebat(2);
    if(input == downRight) movebat(3);

    moveball();
    handlePhysics();

    }
}

void movebat(int input){
    switch (input) {
        case 0; //Move left bat up
            if (playerbat_a.Ypos > 0) {   //Checks that the bat doesn't move beyond the screen
                playerbat_a.oldYpos = playerbat_a.Ypos;
                playerbat_a.Ypos --;
                //TODO: Remove last position
                //      Draw new position
            }
            break;
        case 1; //Move left bat down
            if (playerbat_a.Ypos + playerbat_a.heigth < playfield.height) {   //Checks that the bat doesn't move beyond the screen
                playerbat_a.oldYpos = playerbat_a.Ypos;
                playerbat_a.Ypos ++;
                //TODO: Remove last position
                //      Draw new position
            }
            break;
        case 2; //Move right bat up
            if (playerbat_b.Ypos > 0) {   //Checks that the bat doesn't move beyond the screen
                playerbat_b.oldYpos = playerbat_b.Ypos;
                playerbat_b.Ypos --;
                //TODO: Remove last position
                //      Draw new position
            }
            break;
        case 3; //Move right bat down
            if (playerbat_b.Ypos + playerbat_b.heigth < playfield.height) {   //Checks that the bat doesn't move beyond the screen
                playerbat_b.oldYpos = playerbat_b.Ypos;
                playerbat_b.Ypos ++;
                //TODO: Remove last position
                //      Draw new position
            }
            break;
        default:
            break;
            
    }
    return;


}

void moveball(){
    ball_a.oldXpos = ball_a.Xpos;
    ball_a.oldYpos = ball_a.Ypos;

    ball_a.Xpos = ball_a.Xpos + ball_a.Xspeed;
    ball_a.Ypos = ball_a.Ypos + ball_a.Yspeed;

    //TODO: Remove old drawn position and draw new position
    //
    //
    if(ball_a.Ypos - ball_a.radius < 0)ball_a.Yspeed = -ball_a.Yspeed; //Bounces the ball from the top
    if(ball_a.Ypos + ball_a.radius > playfield_a.height)ball_a.Yspeed = -ball_a.Yspeed; // Bounces the ball from the bottom
}

void handlePhysics(){
    char tempstring[10];
    if (ball_a.Xpos -ball_a.radius <= 2) {   //Checks if the ball hits the bat
        if (ball_a.Ypos > playerbat_a.Ypos && ball_a.Ypos < playerbat_a.Ypos + playerbat_a.length) {
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
    if (ball_a.Xpos -ball_a.radius <= 317) {   //Checks if the ball hits the bat
            if (ball_a.Ypos > playerbat_b.Ypos && ball_a.Ypos < playerbat_b.Ypos + playerbat_b.length) {
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
    playfield_a.height = 239;
    playfield_a.width = 319;
    
    ball_a.radius = 3;
    
    ball_a.Xspeed = 4;
    if(rand()%2 == 0)ball_a.Xspeed = -ball_a.Xspeed;
    ball_a.Yspeed = 0;
    ball_a.Ypos = 119;  //Starts the ball in the centre
    ball_a.Xpos = 159;

    playerbat_a.length = 30;
    playerbat_a.width = 3;
    playerbat_b.length = 30;
    playerbat_b.width = 3;
    playerbat_a.Xpos = 5;
    playerbat_a.Ypos = 119;
    playerbat_b.Xpos = 314;
    playerbat_b.Ypos = 119;
    playerbat_a.speed = 5;
    playerbat_b.speed = 5;
    
    if(first){
        playerAscore = 0;
        playerBscore = 0;
    }
}

void 








void draw(uint32_t matrix[SIZE][SIZE])
{
    //Function to draw the game, represented as a matrix to screen



    return;
}
