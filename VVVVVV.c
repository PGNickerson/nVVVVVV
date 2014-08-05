#include <nspireio.h>
#include <stdio.h>
#include <os.h>
#include <libndls.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "spritesandtypes.h"
#include "map1.h"

#define M_PI 3.14159265358979323846

#define itofix(x) ((x) << 8)
#define fixtoi(x) ((x) >> 8)
#define fixmul(x, y) ((x)*  (y) >> 8)
#define fixdiv(x, y) (((x) << 8) / (y))

#define fixsin(x) fixcos((x) - 64)

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

#define COLOR_BPP		   16
#define COLOR_STORAGE_SIZE 2
#define BMPWIDTH		   17;
#define BMPHEIGHT		   12;

#define shift_y 8
#define shift_x 8
#define vy_limit 256
#define vx_limit 256
#define maxtrail 20

#define TILE_WIDTH 8
#define TILE_HEIGHT 8

typedef int Fixed;

struct Rect {
	int x;
	int y;
	int h;
	int w;
};

struct Point {
	int x;
	int y;
};

Fixed fixcos(Fixed angle) {
	static Fixed cosLUT[] = { 256, 255, 255, 255, 254, 254, 253, 252, 251, 249, 248, 246, 244, 243, 241, 238, 236, 234, 231, 228, 225, 222, 219, 216, 212, 209, 205, 201, 197, 193, 189, 185, 181, 176, 171, 167, 162, 157, 152, 147, 142, 136, 131, 126, 120, 115, 109, 103, 97, 92, 86, 80, 74, 68, 62, 56, 49, 43, 37, 31, 25, 18, 12, 6, 0, -6, -12, -18, -25, -31, -37, -43, -49, -56, -62, -68, -74, -80, -86, -92, -97, -103, -109, -115, -120, -126, -131, -136, -142, -147, -152, -157, -162, -167, -171, -176, -181, -185, -189, -193, -197, -201, -205, -209, -212, -216, -219, -222, -225, -228, -231, -234, -236, -238, -241, -243, -244, -246, -248, -249, -251, -252, -253, -254, -254, -255, -255, -255, -256, -255, -255, -255, -254, -254, -253, -252, -251, -249, -248, -246, -244, -243, -241, -238, -236, -234, -231, -228, -225, -222, -219, -216, -212, -209, -205, -201, -197, -193, -189, -185, -181, -176, -171, -167, -162, -157, -152, -147, -142, -136, -131, -126, -120, -115, -109, -103, -97, -92, -86, -80, -74, -68, -62, -56, -49, -43, -37, -31, -25, -18, -12, -6, 0, 6, 12, 18, 25, 31, 37, 43, 49, 56, 62, 68, 74, 80, 86, 92, 97, 103, 109, 115, 120, 126, 131, 136, 142, 147, 152, 157, 162, 167, 171, 176, 181, 185, 189, 193, 197, 201, 205, 209, 212, 216, 219, 222, 225, 228, 231, 234, 236, 238, 241, 243, 244, 246, 248, 249, 251, 252, 253, 254, 254, 255, 255, 255 };
	return cosLUT[angle & 0xff];
}

inline void setPixel565(int x, int y, uint16_t c) {
	if(x>=0 && x<320 && y>=0 && y<240) {
		if(is_cx) {
			*((unsigned short*)(SCREEN_BASE_ADDRESS + (x << 1) + (y << 9) + (y << 7))) = c;
		}else{
			unsigned char* p = (unsigned char*)(SCREEN_BASE_ADDRESS  + ((x >> 1) + (y << 7) + (y << 5)));
			char black_and_white = ((c>>11)+((c&0x7E0)>>5)+(c&0x1F))>>3;
			*p = (x & 1) ? ((*p & 0xF0) | black_and_white) : ((*p & 0x0F) | (black_and_white << 4));
		}
	}
}

inline void setPixelRGB(int x, int y, int r, int g, int b) {
	if(x>=0 && x<320 && y>=0 && y<240) {
		if(is_cx) {
			*((unsigned short*)(SCREEN_BASE_ADDRESS + (x << 1) + (y << 9) + (y << 7))) = (((r) >> 3) << 11) | (((g) >> 2) << 5) | ((b) >> 3);
		}else{
			unsigned char* p = (unsigned char*)(SCREEN_BASE_ADDRESS  + ((x >> 1) + (y << 7) + (y << 5)));
			char black_and_white = (r>>6) + (g>>5) + (b>>6);
			*p = (x & 1) ? ((*p & 0xF0) | black_and_white) : ((*p & 0x0F) | (black_and_white << 4));
		}
	}
}

void rotate(int x, int y, Fixed ca, Fixed sa, struct Rect* out) {
	out->x = fixtoi(fixmul(itofix(x), ca)+fixmul(itofix(y), sa));
	out->y = fixtoi(fixmul(itofix(x), -sa)+fixmul(itofix(y), ca));
}

void custom_rotosprite(unsigned short* source, struct Rect sr, Fixed angle) {
	struct Rect upleft, upright, downleft, downright;
	struct Rect fr;
	Fixed dX = fixcos(angle), dY = fixsin(angle);
	rotate(-source[0] / 2, -source[1] / 2, dX, dY, &upleft);
	rotate(source[0] / 2, -source[1] / 2, dX, dY, &upright);
	rotate(-source[0] / 2, source[1] / 2, dX, dY, &downleft);
	rotate(source[0] / 2, source[1] / 2, dX, dY, &downright);
	fr.x = min(min(min(upleft.x, upright.x), downleft.x), downright.x)+sr.x;
	fr.y = min(min(min(upleft.y, upright.y), downleft.y), downright.y)+sr.y;
	fr.w = max(max(max(upleft.x, upright.x), downleft.x), downright.x)+sr.x;
	fr.h = max(max(max(upleft.y, upright.y), downleft.y), downright.y)+sr.y;
	struct Rect cp;
	struct Rect lsp;
	struct Rect cdrp;
	lsp.x = fixmul(itofix(fr.x-sr.x), dX)+fixmul(itofix(fr.y-sr.y), -dY);
	lsp.y = fixmul(itofix(fr.x-sr.x), dY)+fixmul(itofix(fr.y-sr.y), dX);
	for(cp.y = fr.y; cp.y < fr.h; cp.y++) {
		cdrp.x = lsp.x;
		cdrp.y = lsp.y;
		for(cp.x = fr.x; cp.x < fr.w; cp.x++) {
			if(cp.x>=0 && cp.x<320 && cp.y>=0 && cp.y<240) {
				if(abs(fixtoi(cdrp.x))<source[0]/2 && abs(fixtoi(cdrp.y))<source[1]/2) {
					unsigned short currentPixel=source[fixtoi(cdrp.x)+source[0]/2+(fixtoi(cdrp.y)+source[1]/2)*source[0]+3];
					if(currentPixel!=source[2]) {
						setPixel565(cp.x, cp.y, currentPixel);
					}
				}
			}
			cdrp.x += dX;
			cdrp.y += dY;
		}
		lsp.x -= dY;
		lsp.y += dX;
	}
}

void putsprite(int x, int y, struct Rect src_rect, unsigned short* ptr)
{
	int i, j;
	for(j=0; j<src_rect.h; j++)
	{
		for(i=0; i<src_rect.w; i++)
		{
			if(ptr[ptr[0]*(src_rect.y+j)+(src_rect.x+i)+3] != ptr[2])
			{
				setPixel565(x+i, y+j, ptr[ptr[0]*(src_rect.y+j)+(src_rect.x+i)+3]);
			}
		}
	}
}

void FillScreen565(unsigned short c) {
	int x; int y;
	for(y=0;y<240;y++) {
		for(x=0;x<320;x++) {
			setPixel565(x,y,c);
		}
	}
}

void clearplayer(struct Point playerpoint)
{
	int x, y;
	for(y=playerpoint.y;y<playerpoint.y+21;y++)
	{
		for(x=playerpoint.x;x<playerpoint.x+10;x++)
		{
			setPixel565(x,y,0);
		}
	}
}

/*
void DrawCenteredNum(int x, int y, int n, unsigned short* font) {
	int stringwidth=log10(n)+1;
	int fontw=font[0];
	int fonth=font[1];
	x=x+stringwidth*(fontw-1)/2;
	y=y-fonth/2;
	int p,i,j;
	unsigned short c;
	while(n) {
		p=(n%10)*fontw*fonth+2;
		for(j=0;j<fonth;j++) {
			for(i=0;i<fontw;i++) {
				c=font[++p];
				if(c!=font[2]) {
					setPixel565(x+i,y+j,c);
				}
			}
		}
		x=x-fontw;
		n=n/10;
	}
}

void EraseCenteredNum(int x, int y, int n, unsigned short* font) {
	int stringwidth=log10(n)+1;
	int fontw=font[0];
	int fonth=font[1];
	x=x+stringwidth*(fontw-1)/2;
	y=y-fonth/2;
	int p,i,j;
	unsigned short c;
	while(n) {
		p=(n%10)*fontw*fonth+2;
		for(j=0;j<fonth;j++) {
			for(i=0;i<fontw;i++) {
				c=font[++p];
				if(c!=font[2]) {
					setPixel565(x+i,y+j,0); //seule ligne qui change par rapport à Draw
				}
			}
		}
		x=x-fontw;
		n=n/10;
	}
}
*/

//BEGIN

unsigned short is_in_air = 1;
unsigned short is_in_flip = 0;
struct Point player_point = {16,16};
signed short gravity = 1;
struct Point checkpoint = {16,16};

void draw_tile(unsigned short *tileset, int tile_num, int x, int y)
{
    struct Rect src_rect;
	struct Point screen_pos;
    src_rect.x = tile_num % 30 * 10 + 8;
    src_rect.y = tile_num / 30 * 10;
    src_rect.w = TILE_WIDTH;
    src_rect.h = TILE_HEIGHT;
    screen_pos.x = x * TILE_WIDTH;
    screen_pos.y = y * TILE_HEIGHT;
    putsprite(screen_pos.x, screen_pos.y, src_rect, tileset);
}

void draw_tile_map(void)
{
    int i, j;
    for(i = 0; i < MAP1_HEIGHT; ++i)
        for(j = 0; j < MAP1_WIDTH; ++j)
            draw_tile(image_VVVVVV_MapSprites, map1_data[i][j], j, i);
}

void flip_player()
{
	gravity = -1 * gravity;
	is_in_air = 1;
}

int can_move_x()
{
	int i;
	for(i = 0; i < 21; ++i)
	{
		if(map1_data[(player_point.y + i) / 8][player_point.x / 8] != 20)
		{
			return 0;
		}
		if(map1_data[(player_point.y + i) / 8][(player_point.x + 9) / 8] != 20)
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
		if(map1_data[player_point.y / 8][(player_point.x + i) / 8] != 20)
		{
			is_in_air = 0;
			return 0;
		}
		if(map1_data[(player_point.y + 20) / 8][(player_point.x + i) / 8] != 20)
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
			if(map1_data[(player_point.y + 20) / 8][(player_point.x + i) / 8] <= 3)
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
		if((map1_data[(player_point.y + i) / 8][player_point.x / 8] == 4) && !(is_in_flip))
		{
			return 1;
		}
		if((map1_data[(player_point.y + i) / 8][(player_point.x + 9) / 8] == 4) && !(is_in_flip))
		{
			return 1;
		}
		if(i < 10)
		{
			if((map1_data[player_point.y / 8][(player_point.x + i) / 8] == 4) && !(is_in_flip))
			{
				return 1;
			}
			if((map1_data[(player_point.y + 20) / 8][(player_point.x + i) / 8] == 4) && !(is_in_flip))
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
	int keep_playing = 1;
	struct Rect player_sprite = {25,76,21,10};
	struct Rect inverted_player_sprite = {59,76,21,10};
	FillScreen565(0);
	draw_tile_map();
	while (keep_playing)
	{
		prev_x = player_point.x;
		prev_y = player_point.y;
		if(K_7)
		{
			player_point.x--;
			if(!can_move_x())
			{
				player_point.x = prev_x;
			}
		}
		if(K_9)
		{
			player_point.x++;
			if(!can_move_x())
			{
				player_point.x = prev_x;
			}
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
		if(!can_move_y())
		{
			player_point.y = prev_y;
		}
		if(gravity == 1)
		{
			putsprite(player_point.x,player_point.y,player_sprite,image_VVVVVV);
		}
		if(gravity == -1)
		{
			putsprite(player_point.x,player_point.y,inverted_player_sprite,image_VVVVVV);
		}
		sleep(10);
		clearplayer(player_point);
	}
}