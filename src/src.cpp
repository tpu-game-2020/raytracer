// src.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#define STB_IMAGE_WRITE_IMPLEMENTATION

// メモリリークチェック
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
// メモリリークチェック
// 
#include "stb_image.h"
#include "stb_image_write.h"


#define WIDTH 1920
#define HEIGHT 1080

typedef struct {
	unsigned char r, g, b;
}cVector3;

int main()
{
	// メモリリークチェック
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// メモリリークチェック

	cVector3 *image = (cVector3*)malloc(sizeof(cVector3) * WIDTH * HEIGHT);

	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			cVector3* p = &image[y * WIDTH + x];
			p->r = rand() & 0xff;
			p->g = rand() & 0xff;
			p->b = rand() & 0xff;
		}
	}

	// save
	int w = WIDTH;
	int h = HEIGHT;
	int comp = STBI_rgb; // RGB
	int stride_in_bytes = 3 * w;
	int result = stbi_write_png("result.png", w, h, comp, image, stride_in_bytes);


	free(image);
}

