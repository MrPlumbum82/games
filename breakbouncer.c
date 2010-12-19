/* main.c
 * Author: Ben Wright
 * Date: 12th December 2009
 * Description: A simple breakout clone mixed with pong written in SDL.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

#include "SDL.h"
#include "SDL_opengl.h"

#define SCREEN_HEIGHT 480
#define SCREEN_WIDTH 640
#define SCREEN_BPP 32

#define BAT_WIDTH 60
#define BAT_HEIGHT 20

#define BAT_WALL_SPACE (SCREEN_HEIGHT - BAT_HEIGHT)
#define BAT_SPEED 10

#define BALL_WIDTH 20
#define BALL_HEIGHT 20
#define BALL_SPEED 3

#define AI_MOVE_RATIO 2 // 1 move every five cycles.

#define NUM_BLOCKS (NUM_BLOCK_ROWS*NUM_BLOCK_COLUMNS)
#define NUM_BLOCK_ROWS 5
#define NUM_BLOCK_COLUMNS 12
#define BLOCK_WIDTH 40
#define BLOCK_HEIGHT 20
#define BLOCK_SPACE_WIDTH (15+BLOCK_WIDTH)
#define BLOCK_SPACE_HEIGHT (15+BLOCK_HEIGHT)
#define BLOCK_SPACER (7*BLOCK_HEIGHT)

#define SCREEN_LEFT 0
#define SCREEN_RIGHT (SCREEN_WIDTH - BAT_WIDTH)

#define SCREEN_TOP 0

#define MAX_SCORE 10
#define NUM_LIVES 3

typedef struct game* Game;

typedef struct {
	int directionx;
	int directiony;
	SDL_Rect node; // The resource that's drawn.
} ball;

typedef struct {
	bool playable; // Determine whether a computer or keyboard input will play this bat.
	int score; // Each bat has it's own score.
	int lives;
	int moveRatio;
	SDL_Rect node; // The resource that's drawn.
} bat;


// Stores all objects to be used in game
struct game {
	bool running;
	bat bat1;
	bat bat2;
	ball ball1;
	ball ball2;
	SDL_Rect blocks[NUM_BLOCK_ROWS][NUM_BLOCK_COLUMNS];
	Uint8 *key; // Array that holds what keys are pressed
	SDL_Event event;
	SDL_Surface* screen;
};

Game initGame();
void closeGame(Game gameToBeClosed);

void handleEvents(Game breakout);
void update(Game breakout);
void draw(Game breakout);

void updateBall(Game breakout);
void checkBallReset(Game breakout);
void resetBalls(Game breakout);

void loadBlocks(Game breakout);

bool hasCollided(SDL_Rect block1ToCheck, SDL_Rect  block2ToCheck);
int batAI(Game breakout, bat batToControl);

int main(int argc, char *argv[]) {
	Game breakout;
	breakout = initGame();
	
	while ( breakout->running ) {
		handleEvents(breakout);
		update(breakout);
		draw(breakout);
	}
	
	SDL_Quit();
	closeGame(breakout);
	
	return EXIT_SUCCESS;
}

Game initGame() {
	Game newGame = malloc(sizeof(struct game));
	
	SDL_Init(SDL_INIT_EVERYTHING);
	newGame->screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE); 
	SDL_WM_SetCaption("BreakBouncer",NULL);
	
	resetBalls(newGame);
	
	newGame->bat1.node.w = BAT_WIDTH;
	newGame->bat1.node.h = BAT_HEIGHT;
	newGame->bat1.node.x = SCREEN_WIDTH / 2;
	newGame->bat1.node.y = BAT_WALL_SPACE - BAT_HEIGHT;
	newGame->bat1.lives = NUM_LIVES;
	
	newGame->bat2.node.w = BAT_WIDTH;
	newGame->bat2.node.h = BAT_HEIGHT;
	newGame->bat2.node.x = SCREEN_WIDTH / 2;
	newGame->bat2.node.y = BAT_HEIGHT;
	newGame->bat2.lives = NUM_LIVES;
	newGame->bat2.moveRatio = 0;
	
	loadBlocks(newGame);
	
	newGame->key = SDL_GetKeyState(NULL);
	
	newGame->running = true;
	
	return newGame;
}

void closeGame(Game gameToBeClosed) {
	free(gameToBeClosed);
}

void handleEvents(Game breakout) {
	while ( SDL_PollEvent(&breakout->event) ) {
		switch ( breakout->event.type ) {
			case SDL_QUIT:
				breakout->running = false;
		}
	}
	
	if ( breakout->key[SDLK_a] && breakout->bat1.node.x > SCREEN_LEFT ) {
		breakout->bat1.node.x -= BAT_SPEED;
	} else if ( breakout->key[SDLK_d] && breakout->bat1.node.x < SCREEN_RIGHT ) {
		breakout->bat1.node.x += BAT_SPEED;
	}
	
	if ( breakout->key[SDLK_LEFT] && breakout->bat2.node.x > SCREEN_LEFT ) {
		breakout->bat2.node.x -= BAT_SPEED;
	} else if ( breakout->key[SDLK_RIGHT] && breakout->bat2.node.x < SCREEN_RIGHT ) {
		breakout->bat2.node.x += BAT_SPEED;
	} else {
		breakout->bat2.node.x += batAI(breakout, breakout->bat2);
	}

}

void update(Game breakout) {
	breakout->key = SDL_GetKeyState(NULL);
	if ( breakout->bat1.lives <= 0  || breakout->bat2.lives <=0) {
		resetBalls(breakout);
		loadBlocks(breakout);
		breakout->bat1.lives = NUM_LIVES;
		breakout->bat2.lives = NUM_LIVES;
	}
	updateBall(breakout);
}

void draw(Game breakout) {
	SDL_FillRect(breakout->screen , NULL , SDL_MapRGB(breakout->screen->format , 0 , 0 , 0 ));
	
	SDL_FillRect(breakout->screen , &breakout->bat1.node , SDL_MapRGB(breakout->screen->format , 0 , 255 , 50 ) );
	SDL_FillRect(breakout->screen , &breakout->bat2.node , SDL_MapRGB(breakout->screen->format , 0 , 255 , 50 ) );
	
	SDL_FillRect(breakout->screen , &breakout->ball1.node , SDL_MapRGB(breakout->screen->format ,  0 , 255 , 50 ) );
	SDL_FillRect(breakout->screen , &breakout->ball2.node , SDL_MapRGB(breakout->screen->format ,  0 , 255 , 50 ) );

	
	int row, column;
	for ( row=0; row < NUM_BLOCK_ROWS; row++ ) {
		for ( column=0; column < NUM_BLOCK_COLUMNS; column++ ) {
			if (breakout->blocks[row][column].w != 0) {
				SDL_FillRect(breakout->screen , &breakout->blocks[row][column] , SDL_MapRGB(breakout->screen->format ,  150 , row*20 , column*20) );
			}
		}
	}
	
	SDL_Flip(breakout->screen);
}

void updateBall(Game breakout) {
	
	checkBallReset(breakout);
	
	if ( breakout->bat1.score > MAX_SCORE || breakout->bat2.score > MAX_SCORE) {
		breakout->bat1.score = 0;
		breakout->bat2.score = 0;
	}
	
	if ( hasCollided(breakout->bat1.node, breakout->ball1.node) ) {
		breakout->ball1.directiony = -1;
	} else if ( hasCollided(breakout->bat2.node, breakout->ball1.node) ) {
		breakout->ball1.directiony = 1;
	}
	
	if ( hasCollided(breakout->bat1.node, breakout->ball2.node) ) {
		breakout->ball2.directiony = -1;
	} else if ( hasCollided(breakout->bat2.node, breakout->ball2.node) ) {
		breakout->ball2.directiony = 1;
	}
	
	if ( hasCollided(breakout->ball1.node, breakout->ball2.node) ) {
		breakout->ball1.directiony *= -1;
		breakout->ball2.directiony *= -1;
	}
	
	if ( (breakout->ball1.node.x + BALL_WIDTH) >= SCREEN_WIDTH)  {
		breakout->ball1.directionx = -1;
	} else if ( breakout->ball1.node.x <= 0 ) {
		breakout->ball1.directionx = 1;
	}
	
	if ( (breakout->ball2.node.x + BALL_WIDTH) >= SCREEN_WIDTH)  {
		breakout->ball2.directionx = -1;
	} else if ( breakout->ball2.node.x <= 0 ) {
		breakout->ball2.directionx = 1;
	}
	
	int row, column;
	for ( row=0; row < NUM_BLOCK_ROWS; row++ ) {
		for ( column=0; column < NUM_BLOCK_COLUMNS; column++ ) {
			if ( hasCollided(breakout->ball1.node, breakout->blocks[row][column]) && (breakout->blocks[row][column].w !=0) ) {
				breakout->blocks[row][column].w = 0;
				breakout->blocks[row][column].h = 0;
				breakout->ball1.directiony = 1;
			}
			if ( hasCollided(breakout->ball2.node, breakout->blocks[row][column]) && (breakout->blocks[row][column].w !=0) ) {
				breakout->blocks[row][column].w = 0;
				breakout->blocks[row][column].h = 0;
				breakout->ball2.directiony = -1;
			}
		}
	}
	
	breakout->ball1.node.x  += (breakout->ball1.directionx * BALL_SPEED);
	breakout->ball1.node.y += (breakout->ball1.directiony * BALL_SPEED);
	
	breakout->ball2.node.x  += (breakout->ball2.directionx * BALL_SPEED);
	breakout->ball2.node.y += (breakout->ball2.directiony * BALL_SPEED);
}

void checkBallReset(Game breakout){
	
	if (breakout->ball1.node.y <= 0 || breakout->ball2.node.y <= 0) {
		breakout->bat2.lives--;
	}
	if ( (breakout->ball1.node.y + BALL_HEIGHT) >= SCREEN_HEIGHT || (breakout->ball2.node.y + BAT_HEIGHT) >= SCREEN_HEIGHT) {
		breakout->bat1.lives--;
	}
	
	if ( (breakout->ball1.node.y + BALL_HEIGHT) >= SCREEN_HEIGHT || breakout->ball1.node.y <= 0) {
		breakout->ball1.node.w = BALL_WIDTH;
		breakout->ball1.node.h = BALL_HEIGHT;
		breakout->ball1.node.x = SCREEN_WIDTH / 2;
		breakout->ball1.node.y = (BLOCK_SPACER + SCREEN_HEIGHT) / 2;
		
	} else if ( (breakout->ball2.node.y + BALL_HEIGHT) >= SCREEN_HEIGHT || breakout->ball2.node.y <= 0) {
		breakout->ball2.node.w = BALL_WIDTH;
		breakout->ball2.node.h = BALL_HEIGHT;
		breakout->ball2.node.x = SCREEN_WIDTH / 2;
		breakout->ball2.node.y = BLOCK_SPACER / 2;
	}
}

void resetBalls(Game breakout) {
	breakout->ball1.node.w = BALL_WIDTH;
	breakout->ball1.node.h = BALL_HEIGHT;
	breakout->ball1.node.x = SCREEN_WIDTH / 2;
	breakout->ball1.node.y = (BLOCK_SPACER + SCREEN_HEIGHT) / 2; 
	breakout->ball1.directionx = 1;
	breakout->ball1.directiony = 1;
	
	breakout->ball2.node.w = BALL_WIDTH;
	breakout->ball2.node.h = BALL_HEIGHT;
	breakout->ball2.node.x = SCREEN_WIDTH / 2;
	breakout->ball2.node.y = BLOCK_SPACER / 2; 
	breakout->ball2.directionx = -1;
	breakout->ball2.directiony = -1;
}

void loadBlocks(Game breakout) {
	int row, column;
	for ( row=0; row < NUM_BLOCK_ROWS; row++ ) {
		for ( column=0; column < NUM_BLOCK_COLUMNS; column++ ) {
			breakout->blocks[row][column].w = BLOCK_WIDTH;
			breakout->blocks[row][column].h = BLOCK_HEIGHT;
			breakout->blocks[row][column].x = BLOCK_SPACE_WIDTH * column;
			breakout->blocks[row][column].y = BLOCK_SPACER + BLOCK_SPACE_HEIGHT * row;
		}
	}
}

bool hasCollided (SDL_Rect block1ToCheck, SDL_Rect  block2ToCheck) {
	bool collision = true;
	
	int block1Top, block1Bottom;
	int block1Left, block1Right;
	
	int block2Top, block2Bottom;
	int block2Left, block2Right;
	
	block1Top = block1ToCheck.y;
	block1Bottom = block1ToCheck.y + block1ToCheck.h;
	block1Left = block1ToCheck.x;
	block1Right = block1ToCheck.x + block1ToCheck.w;
	
	block2Top = block2ToCheck.y;
	block2Bottom = block2ToCheck.y + block2ToCheck.h;
	block2Left = block2ToCheck.x;
	block2Right = block2ToCheck.x + block2ToCheck.w;
	
	if( block1Bottom <= block2Top ) {
		collision = false;
	} else if( block1Top >= block2Bottom ) {
		collision = false;
	} else if( block1Right <= block2Left) {
		collision = false;
	} else if( block1Left >= block2Right ) {
		collision = false;
	}
	
	return collision;
}

int batAI(Game breakout, bat batToControl) {
	int batDirection = 0;
	
	if ( breakout->ball2.node.y < breakout->ball1.node.y && breakout->ball2.node.y >= 0 && breakout->bat2.moveRatio >= AI_MOVE_RATIO ) {
		if ( breakout->ball2.node.x > batToControl.node.x && batToControl.node.x < SCREEN_RIGHT ) {
			batDirection = BAT_SPEED;
		} else if ( breakout->ball2.node.x < batToControl.node.x && batToControl.node.x > SCREEN_LEFT ) {
			batDirection -= BAT_SPEED;
		}
		breakout->bat2.moveRatio = 0;
	} else if ( breakout->bat2.moveRatio >= AI_MOVE_RATIO) {
		if ( breakout->ball1.node.x > batToControl.node.x && batToControl.node.x < SCREEN_RIGHT ) {
			batDirection = BAT_SPEED;
		} else if ( breakout->ball1.node.x < batToControl.node.x && batToControl.node.x > SCREEN_LEFT ) {
			batDirection -= BAT_SPEED;
		}
		breakout->bat2.moveRatio = 0;
	} else {
		breakout->bat2.moveRatio++;
	}
	
	return batDirection;
}