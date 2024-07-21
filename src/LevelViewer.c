#include "LevelViewer.h"
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_log.h>
#include <SDL2/SDL_surface.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#include "PK2Map.h"
#include "util.h"

#define CLEAR_LAYER(layer)\
	for (int i = 0; i < 256; ++i) {\
		for (int j = 0; j < 224; ++j) {\
			layer[i][j] = 255;\
		}\
	}

#define READ_PK2_INT(target, fp) \
	{\
		char tmp[8]; \
		fread(tmp, sizeof(tmp), 1, fp); \
		target = strtol(tmp, NULL, 10); \
	}

struct Camera {
	int x, y;
	int width, height;
};

static struct Camera camera;
static struct PK2Map map;

static SDL_Texture* background_image = NULL;
static SDL_Texture* tileset_image = NULL;

static char game_folder[4096];

static void read_layer(int layer[][224], FILE* fp);
static void draw_layer(SDL_Renderer* renderer, int layer[][224]);
static void lookup_file(char* buffer, const char* file);

void Level_Viewer_init(int camera_width, int camera_height) {
	camera.x = 0;
	camera.y = 0;
	camera.width = camera_width;
	camera.height = camera_height;

	memset(map.tileset, 0, sizeof map.tileset);
	memset(map.background, 0, sizeof map.background);

	CLEAR_LAYER(map.background_layer)
	CLEAR_LAYER(map.foreground_layer)
	CLEAR_LAYER(map.sprites_layer)
}

void Level_Viewer_draw(SDL_Renderer* renderer) {
	SDL_SetRenderDrawColor(renderer, 28, 28, 31, 255);
	SDL_RenderClear(renderer);

	if (background_image) {
		for (int rx = 0; rx < camera.width / 640; ++rx) {
			for (int ry = 0; ry < (camera.height / 480) + 1; ++ry) {
				SDL_Rect target = {
					.x = rx * 640,
					.y = ry * 480,
					.w = 640,
					.h = 480
				};

				SDL_RenderCopy(renderer, background_image, NULL, &target);
			}
		}
	}

	if (tileset_image) {
		draw_layer(renderer, map.background_layer);
		draw_layer(renderer, map.foreground_layer);
	}

	SDL_RenderPresent(renderer);
}

void Level_Viewer_handle_input(SDL_Event event) {
	switch (event.key.keysym.sym) {
		case SDLK_UP:
			camera.y -= 32;

			if (camera.y < 0) camera.y = 0;
		break;

		case SDLK_DOWN:
			camera.y += 32;

			if (camera.y + camera.height > 224 * 32) camera.y = (224 * 32) - camera.height;
		break;

		case SDLK_LEFT:
			camera.x -= 32;

			if (camera.x < 0) camera.x = 0;
		break;

		case SDLK_RIGHT:
			camera.x += 32;

			if (camera.x + camera.width > 256 * 32) camera.x = (256 * 32) - camera.width;
		break;

		case SDLK_SPACE:
			camera.x = ((256 * 32) / 2) - (camera.width / 2);
			camera.y = ((224 * 32) / 2) - (camera.height / 2);
		break;
	}
}

bool Level_Viewer_load_map(const char* game_path, const char* map_name) {
	DIR* dir = NULL;

	if ((dir = opendir(game_path)) == NULL) {
		SDL_Log("Unable to read directory: %s", game_path);

		return false;
	}

	closedir(dir);

	// Read the map file
	FILE* fp = NULL;
	char file_path[4096];

	// NOTE: This is unsafe!
	strcpy(file_path, game_path);
	strcat(file_path, "/episodes/");
	strcat(file_path, map_name);

	strcpy(game_folder, game_path);

	fp = fopen(file_path, "rb");

	if (fp) {
		const unsigned char expected_id[5] = { 0x31, 0x2E, 0x33, 0x00, 0xCD };
		unsigned char read_id[5] = {0};

		fread(read_id, sizeof(unsigned char), 5, fp);

		if (memcmp(expected_id, read_id, 5) == 0) {
			fread(map.tileset, sizeof(map.tileset), 1, fp);
			fread(map.background, sizeof(map.background), 1, fp);
			fread(map.music, sizeof(map.music), 1, fp);

			fread(map.map_name, sizeof(map.map_name), 1, fp);
			fread(map.author, sizeof(map.author), 1, fp);

			READ_PK2_INT(map.level_nr, fp)
			READ_PK2_INT(map.weather, fp)

			int unused;
			READ_PK2_INT(unused, fp)
			READ_PK2_INT(unused, fp)
			READ_PK2_INT(unused, fp)

			READ_PK2_INT(map.time, fp)
			READ_PK2_INT(unused, fp)
			READ_PK2_INT(map.scrolling, fp)
			READ_PK2_INT(map.player_sprite, fp)
			READ_PK2_INT(map.map_x, fp)
			READ_PK2_INT(map.map_y, fp)
			READ_PK2_INT(map.icon_id, fp)
			READ_PK2_INT(map.sprites_amount, fp)

			fread(map.sprites_files, sizeof map.sprites_files[0], map.sprites_amount, fp);

			read_layer(map.background_layer, fp);
			read_layer(map.foreground_layer, fp);
			read_layer(map.sprites_layer, fp);
		} else {
			SDL_Log("File is NOT a PK2 map!");

			return false;
		}

		fclose(fp);
	} else {
		SDL_Log("Unable to open file: %s", file_path);

		return false;
	}

	return true;
}

void Level_Viewer_load_gfx(SDL_Renderer* renderer) {
	char background_path[4096];
	strcpy(background_path, game_folder);
	strcat(background_path, "/gfx/scenery/");

	lookup_file(background_path, map.background);

	SDL_Surface* tmpBackground = SDL_LoadBMP(background_path);

	if (!tmpBackground) {
		SDL_Log("Unable to load image: %s", SDL_GetError());
	} else {
		background_image = SDL_CreateTextureFromSurface(renderer, tmpBackground);

		char tileset_path[4096];
		// NOTE: UNSAFE
		strcpy(tileset_path, game_folder);
		strcat(tileset_path, "/gfx/tiles/");

		// TODO: Should return something and handle file not being found, although SDL_LoadBMP will also throw an error so...
		lookup_file(tileset_path, map.tileset);

		SDL_Surface* tmpTileset = SDL_LoadBMP(tileset_path);

		if (tmpTileset) {
			SDL_Color alphaCol = tmpTileset->format->palette->colors[255];
			SDL_SetColorKey(tmpTileset, SDL_TRUE, SDL_MapRGB(tmpTileset->format, alphaCol.r, alphaCol.g, alphaCol.b));

			SDL_Palette* palette = tmpBackground->format->palette;
			int res = SDL_SetPaletteColors(tmpTileset->format->palette, palette->colors, 0, palette->ncolors);

			if (res < 0) SDL_Log("Unable to set palette! %s: ", SDL_GetError());

			tileset_image = SDL_CreateTextureFromSurface(renderer, tmpTileset);

			SDL_FreeSurface(tmpTileset);
		} else {
			SDL_Log("Unable to load tileset: %s", SDL_GetError());
		}

		SDL_FreeSurface(tmpBackground);
	}
}

void Level_Viewer_close(void) {
	if (background_image) SDL_DestroyTexture(background_image);
	if (tileset_image) SDL_DestroyTexture(tileset_image);
}


static void read_layer(int layer[][224], FILE* fp) {
	int startX, startY;
	int endX, endY;

	READ_PK2_INT(startX, fp)
	READ_PK2_INT(startY, fp)
	READ_PK2_INT(endX, fp)
	READ_PK2_INT(endY, fp)
	
	for (int y = startY; y <= startY + endY; ++y) {
		for (int x = startX; x <= startX + endX; ++x) {
			unsigned char tmp;

			fread(&tmp, sizeof(char), 1, fp);
			layer[x][y] = tmp;
		}
	}

	// NOTE: It's kinda uncessary to do this for every layer, it'd be better to just save this information in the map struct, but whatever.
	camera.x = startX * 32;
	camera.y = startY * 32;

	// NOTE: Again, kinda hacky. But this isn't a serious project, no one's gonna use this.
	if (camera.y + camera.height > 224 * 32) camera.y = (224 * 32) - camera.height;
}


static void draw_layer(SDL_Renderer* renderer, int layer[][224]) {
	for (int x = camera.x / 32; x < (camera.x + camera.width) / 32; ++x) {
		for (int y = camera.y / 32; y < (camera.y + camera.height) / 32; ++y) {
			int tile = layer[x][y];

			if (tile != 255) {
				int tilesetX = (tile % 10) * 32;
				int tilesetY = (tile / 10) * 32;

				SDL_Rect tileSource = {
					.x = tilesetX,
					.y = tilesetY,
					.w = 32,
					.h = 32
				};

				SDL_Rect tileDest = {
					.x = (x * 32) - camera.x,
					.y = (y * 32) - camera.y,
					.w = 32,
					.h = 32
				};

				SDL_RenderCopy(renderer, tileset_image, &tileSource, &tileDest);
			}
		}
	}
}

void lookup_file(char* buffer, const char* file) {
	bool file_found = false;
	int file_exists = access(file, F_OK | R_OK);
	if (file_exists == -1) {
		// Search for file if it doesn't exist
		
		DIR* d = opendir(buffer);

		if (d) {
			struct dirent* dir;

			char* tmp_dir_entry = (char *) malloc(256);
			char* tmp_background = (char *) calloc(256, sizeof(char)); // String containing the lowered background file name
			str_tolower(tmp_background, 256, file);

			while ((dir = readdir(d)) != NULL) {
				if (dir->d_type == DT_REG) {
					memset(tmp_dir_entry, 0, 256);
					str_tolower(tmp_dir_entry, 256, dir->d_name);

					if (strcmp(tmp_dir_entry, tmp_background) == 0) {
						// If the file with the correct case has been found, add it to the folder
						strcat(buffer, dir->d_name);

						file_found = true;

						break;
					}
				}
			}
		
			closedir(d);

			free(tmp_dir_entry);
			free(tmp_background);
		} else {
			SDL_Log("Unable to open directory: %s", file);
		}
	} else {
		SDL_Log("Unable to read file: %s", file);
	} 
}
