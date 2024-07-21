#ifndef LEVEL_VIEWER_H
#define LEVEL_VIEWER_H

#include <SDL2/SDL_render.h>
#include <SDL2/SDL_events.h>

#include <stdbool.h>

void Level_Viewer_init(int camera_width, int camera_height);
void Level_Viewer_close(void);

void Level_Viewer_draw(SDL_Renderer* renderer);
void Level_Viewer_handle_input(SDL_Event event);

void Level_Viewer_load_gfx(SDL_Renderer* renderer);
/*
*	Loads a map.
*
*	const char* game_path	- Path to the game's content folder
*	const char* map_name	- Name of the map to load with episode (i.e. rooster island 1/level001.map)
*	
*	Returns true on success, false if it failed.
*/
bool Level_Viewer_load_map(const char* game_path, const char* map_name);

#endif
