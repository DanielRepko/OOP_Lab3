#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <iostream>

#include "SDL.h"

#ifdef __NDS__
#define WINDOW_WIDTH    320
#define WINDOW_HEIGHT   240
#else
#define WINDOW_WIDTH    640
#define WINDOW_HEIGHT   480
#endif

#define NUM_SPRITES     4

SDL_Texture *alienSprite;
SDL_Texture *cannonSprite;
SDL_Texture *cannonBallSprite;
SDL_Texture *fireBallSprite;

SDL_Rect positions[NUM_SPRITES];
int sprite_w[4], sprite_h[4];

SDL_Joystick *joy = NULL;

//added an 
int LoadSprite(char *file, SDL_Texture *&spriteToPopulate, SDL_Renderer *renderer, int widthIndex, int heightIndex)
{
	SDL_Surface *temp;

	// Load the sprite image
	temp = SDL_LoadBMP(file);
	if (temp == NULL) {
		fprintf(stderr, "Couldn't load %s: %s", file, SDL_GetError());
		return (-1);
	}
	sprite_w[widthIndex] = temp->w;
	sprite_h[heightIndex] = temp->h;

	// Set transparent pixel as the pixel at (0,0)
	if (temp->format->palette) {
		SDL_SetColorKey(temp, SDL_TRUE, *(Uint8 *)temp->pixels);
	}
	else {
		switch (temp->format->BitsPerPixel) {
		case 15:
			SDL_SetColorKey(temp, SDL_TRUE, (*(Uint16 *)temp->pixels) & 0x00007FFF);
			break;
		case 16:
			SDL_SetColorKey(temp, SDL_TRUE, *(Uint16 *)temp->pixels);
			break;
		case 24:
			SDL_SetColorKey(temp, SDL_TRUE, (*(Uint32 *)temp->pixels) & 0x00FFFFFF);
			break;
		case 32:
			SDL_SetColorKey(temp, SDL_TRUE, *(Uint32 *)temp->pixels);
			break;
		}
	}

	// Create textures from the image
	spriteToPopulate = SDL_CreateTextureFromSurface(renderer, temp);
	if (!spriteToPopulate) {
		fprintf(stderr, "Couldn't create texture: %s\n", SDL_GetError());
		SDL_FreeSurface(temp);
		return (-1);
	}
	SDL_FreeSurface(temp);

	return (0);
}

void MoveSprites(SDL_Window * window, SDL_Renderer * renderer)
{
	int i;
	int window_w = WINDOW_WIDTH;
	int window_h = WINDOW_HEIGHT;
	SDL_Rect *position, *velocity;

	// Draw a gray background
	SDL_SetRenderDrawColor(renderer, 0xA0, 0xA0, 0xA0, 0xFF);
	SDL_RenderClear(renderer);

	// Move the sprite, bounce at the wall, and draw
	for (i = 0; i < NUM_SPRITES; ++i) {
		position = &positions[i];

		//Perform sprite-specific actions / Blit the sprite onto the screen
		switch (i)
		{
		case 0:
			//keep the sprite from moving past the top of the screen
			if ((positions[i].y + sprite_h[i]) > WINDOW_HEIGHT)
			{
				positions[i].y = WINDOW_HEIGHT - sprite_h[i];
			}
			//keep the sprite from moving past the bottom of the screen
			if (positions[i].y < 0)
			{
				positions[i].y = 0;
			}

			SDL_RenderCopy(renderer, alienSprite, NULL, position);
			break;
		case 1:
			//keep the sprite from moving past the top of the screen
			if ((positions[i].y + sprite_h[i]) > WINDOW_HEIGHT)
			{
				positions[i].y = WINDOW_HEIGHT - sprite_h[i];
			}
			//keep the sprite from moving past the bottom of the screen
			if (positions[i].y < 0)
			{
				positions[i].y = 0;
			}

			SDL_RenderCopy(renderer, cannonSprite, NULL, position);
			break;
		case 2:
			//render and add velocity to the cannonball
			positions[2].x -= 5;
			SDL_RenderCopy(renderer, cannonBallSprite, NULL, position);
			break;
		case 3:
			//render and add velocity to the fireball
			positions[3].x += 5;
			SDL_RenderCopy(renderer, fireBallSprite, NULL, position);
			break;
		}
		
	}
	// Update the screen!
	SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[])
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	int i, done;
	SDL_Event event;

	window = SDL_CreateWindow("Basic SDL Sprites",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN);
	if (!window) {
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);
	if (!renderer) {
		return false;
	}

	//load the sprites
	if (LoadSprite("alien.bmp", alienSprite, renderer, 0, 0) < 0) {
		return false;
	}
	if (LoadSprite("cannon.bmp", cannonSprite, renderer, 1, 1) < 0) {
		return false;
	}
	if (LoadSprite("cannonball.bmp", cannonBallSprite, renderer, 2, 2) < 0) {
		return false;
	}
	if (LoadSprite("fireball.bmp", fireBallSprite, renderer, 3, 3) < 0) {
		return false;
	}

	//set the joystick
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	if (SDL_NumJoysticks() > 0)
	{
		joy = SDL_JoystickOpen(0);
	}

	// Initialize the sprite positions
	positions[0].x = 0;
	positions[0].y = WINDOW_HEIGHT / 2 - (sprite_h[0] / 2);
	positions[0].w = sprite_w[0];
	positions[0].h = sprite_h[0];

	positions[1].x = WINDOW_WIDTH - sprite_w[1];
	positions[1].y = WINDOW_HEIGHT / 2 - (sprite_h[1] / 2);
	positions[1].w = sprite_w[1];
	positions[1].h = sprite_h[1];

	

	// Main render loop
	done = 0;
	while (!done) {
		// Check for events
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN) {
				done = 1;
			}

			//move the alien with the mouse
			if (event.type == SDL_MOUSEMOTION)
			{
				positions[0].y = event.motion.y - (sprite_h[0] / 2);
			}			

			//shoot a fireball from the alien after mouse click
			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == 1)
			{
				positions[3].w = sprite_w[3];
				positions[3].h = sprite_h[3];

				positions[3].x = positions[0].x + sprite_w[3];
				positions[3].y = positions[0].y + (sprite_h[3] / 3);
			}

			//shoot the cannonball when the A button is pressed
			if (event.jbutton.which == 0 && event.jbutton.button == 0 && event.jbutton.state == SDL_PRESSED)
			{
				positions[2].w = sprite_w[2];
				positions[2].h = sprite_h[2];

				positions[2].x = positions[1].x + sprite_w[2];
				positions[2].y = positions[1].y + (sprite_h[2] / 3);
			}
			
		}

		//move the cannon
		if (joy)
		{
			positions[1].y += SDL_JoystickGetAxis(joy, 1) / 6000;
		}

		MoveSprites(window, renderer);
		SDL_Delay(3);
	}

	SDL_Quit();

	return true;
}
