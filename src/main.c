#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include "LevelViewer.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

void init() {
	if (SDL_INIT_VIDEO < 0) {
		printf("Couldn't init SDL: %s\n", SDL_GetError());
		exit(1);
	}

	window = SDL_CreateWindow("PK2LevelViewer in C!", 
						   SDL_WINDOWPOS_CENTERED,
						   SDL_WINDOWPOS_CENTERED,

						   SCREEN_WIDTH,
						   SCREEN_HEIGHT,

						   0);
	

	if (!window) {
		printf("Failed to open the window: %s", SDL_GetError());
		exit(1);
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

	if (!renderer) {
		printf("Failed to init renderer: %s", SDL_GetError());
		exit(1);
	} 
}

int main(int argc, char* argv[]) {
	init();

	Level_Viewer_init(SCREEN_WIDTH, SCREEN_HEIGHT);

	if (argc == 3) {
		if (Level_Viewer_load_map(argv[1], argv[2])) {
			Level_Viewer_load_gfx(renderer);
		}	
	} else {
		SDL_Log("Usage: PK2LevelViewer game_path map_name_with_episode");
	}


	SDL_Event event;
	bool run = true;

	while (run) {
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					run = false;
					break;

				case SDL_KEYDOWN:
				case SDL_KEYUP:
					Level_Viewer_handle_input(event);
					break;
			}
		}

		Level_Viewer_draw(renderer);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	return 0;
}
