#ifndef PK2MAP_H
#define PK2MAP_H
struct PK2Map {
	char tileset[13];
	char background[13];
	char music[13];

	char map_name[40];
	char author[40];

	int level_nr;
	int weather;

	int time;

	int scrolling;

	int sprites_amount;
	int player_sprite;

	int map_x;
	int map_y;

	int icon_id;

	char sprites_files[100][13];

	int background_layer[256][224];
	int foreground_layer[256][224];
	int sprites_layer[256][224];
};
#endif
