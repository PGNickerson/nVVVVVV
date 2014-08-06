#include <os.h>

#include "n2DLib/n2DLib.h"
#include "spritesandtypes.h"
#include "map1.h"

#define K_ESC isKeyPressed(KEY_NSPIRE_ESC)
#define K_ENTER isKeyPressed(KEY_NSPIRE_ENTER)
#define K_TAB isKeyPressed(KEY_NSPIRE_TAB)
#define K_A isKeyPressed(KEY_NSPIRE_A)
#define K_7 isKeyPressed(KEY_NSPIRE_7)
#define K_9 isKeyPressed(KEY_NSPIRE_9)
#define K_8 isKeyPressed(KEY_NSPIRE_8)
#define K_5 isKeyPressed(KEY_NSPIRE_5)
#define K_4 isKeyPressed(KEY_NSPIRE_4)
#define K_6 isKeyPressed(KEY_NSPIRE_6)

unsigned short is_in_air = 1;
unsigned short is_in_flip = 0;
Point player_point = {16,16};
signed short gravity = 2;
Point checkpoint = {16,16};
Rect src_rect;

void draw_tile(unsigned short *tileset, int tile_num, int x, int y)
{
    src_rect.x = tile_num % 30 * 10 + 8;
    src_rect.y = tile_num / 30 * 10;
    drawSpritePart(tileset, x, y, &src_rect);
}

void draw_tile_map(void)
{
    int i, j;
    for(i = 0; i < MAP1_HEIGHT; ++i)
        for(j = 0; j < MAP1_WIDTH; ++j)
            draw_tile(image_VVVVVV_MapSprites, map1_data[i][j], j * TILE_HEIGHT, i * TILE_WIDTH);
}

void flip_player()
{
	gravity = -1 * gravity;
	is_in_air = 1;
}

int can_move_x()
{
	int i;
	for(i = 0; i < 20; ++i)
	{
		if(map1_data[(player_point.y + i) / 8][player_point.x / 8] > 20)
		{
			return 0;
		}
		if(map1_data[(player_point.y + i) / 8][(player_point.x + 9) / 8] > 20)
		{
			return 0;
		}
	}
	return 1;
}

int can_move_y()
{
	int i;
	for(i = 0; i < 10; ++i)
	{
		if(map1_data[player_point.y / 8][(player_point.x + i) / 8] > 20)
		{
			is_in_air = 0;
			return 0;
		}
		if(map1_data[(player_point.y + 19) / 8][(player_point.x + i) / 8] > 20)
		{
			is_in_air = 0;
			return 0;
		}
	}
	is_in_air = 1;
	return 1;
}

int detect_spike()
{
	int i;
	for(i = 0; i < 21; ++i)
	{
		if(map1_data[(player_point.y + i) / 8][player_point.x / 8] <= 3)
		{
			return 1;
		}
		if(map1_data[(player_point.y + i) / 8][(player_point.x + 9) / 8] <= 3)
		{
			return 1;
		}
		if(i < 10)
		{
			if(map1_data[player_point.y / 8][(player_point.x + i) / 8] <= 3)
			{
				return 1;
			}
			if(map1_data[(player_point.y + 19) / 8][(player_point.x + i) / 8] <= 3)
			{
				return 1;
			}
		}
	}
	return 0;
}

int detect_flip()
{
	int i;
	for(i = 0; i < 21; ++i)
	{
		if ((i < 17) && (i > 2))
		{
			if((map1_data[(player_point.y + i) / 8][player_point.x / 8] == 4) && !(is_in_flip))
			{
				return 1;
			}
			if((map1_data[(player_point.y + i) / 8][(player_point.x + 9) / 8] == 4) && !(is_in_flip))
			{
				return 1;
			}
		}
		if(i < 10)
		{
			if((map1_data[(player_point.y + 3) / 8][(player_point.x + i) / 8] == 4) && !(is_in_flip))
			{
				return 1;
			}
			if((map1_data[(player_point.y + 16) / 8][(player_point.x + i) / 8] == 4) && !(is_in_flip))
			{
				return 1;
			}
		}
	}
	is_in_flip = 1;
	return 0;
}

int main()
{
	int prev_x;
	int prev_y;
	int future_y;
	int keep_playing = 1;
	Rect player_sprite = {25,76,21,10};
	Rect inverted_player_sprite = {59,76,21,10};
	src_rect.w = TILE_WIDTH;
	src_rect.h = TILE_HEIGHT;
	initBuffering();
	while (keep_playing)
	{
		clearBufferB();
		prev_x = player_point.x;
		prev_y = player_point.y;
		if(K_7)
		{
			player_point.x -= 2;
		}
		if(K_9)
		{
			player_point.x += 2;
		}
		if(K_8)
		{
			if(!is_in_air)
			{
				flip_player();
			}
		}
		if(K_ESC)
		{
			keep_playing = 0;
		}
		player_point.y = player_point.y + gravity;
		if(detect_spike())
		{
			player_point = checkpoint;
		}
		if(detect_flip())
		{
			flip_player();
		}
		else
		{
			is_in_flip = 0;
		}
		future_y = player_point.y;
		player_point.y = prev_y;
		if(!can_move_x())
		{
			player_point.x = prev_x;
		}
		player_point.y = future_y;
		if(!can_move_y())
		{
			player_point.y = prev_y;
		}
		draw_tile_map();
		if(gravity == 2)
		{
			drawSpritePart(image_VVVVVV, player_point.x, player_point.y, &player_sprite);
		}
		if(gravity == -2)
		{
			drawSpritePart(image_VVVVVV, player_point.x, player_point.y, &inverted_player_sprite);
		}
		updateScreen();
		//sleep(10);
	}
	deinitBuffering();
}