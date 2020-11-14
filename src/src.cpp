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
	float r, g, b;
}Vector3;


void RayTrace(float *dest, int width, int height)
{

}

int main()
{
	// メモリリークチェック
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// メモリリークチェック

	// メモリイメージの作成
	Vector3* image = (Vector3*)malloc(sizeof(Vector3) * WIDTH * HEIGHT);

	for (int y = 0; y < HEIGHT; y++) {
		Vector3* p = &image[y * WIDTH];
		for (int x = 0; x < WIDTH; x++) {
			p->r = (float)rand() / (float)RAND_MAX;
			p->g = (float)rand() / (float)RAND_MAX;
			p->b = (float)rand() / (float)RAND_MAX;
			p++;
		}
	}


	// 8bitのバッファに移す
	unsigned char* file_image = (unsigned char*)malloc(3 * WIDTH * HEIGHT);
	for (int y = 0; y < HEIGHT; y++) {
		unsigned char* p = &file_image[3 * y * WIDTH];
		const Vector3* s = &image[y * WIDTH];
		for (int x = 0; x < WIDTH; x++) {
			p[0] = (1.0f < s->r) ? 255 : (unsigned char)(255.9999f * s->r);
			p[1] = (1.0f < s->g) ? 255 : (unsigned char)(255.9999f * s->g);
			p[2] = (1.0f < s->b) ? 255 : (unsigned char)(255.9999f * s->b);
			p += 3;
			s++;
		}
	}


	// PNG ファイルとして保存
	int w = WIDTH;
	int h = HEIGHT;
	int comp = STBI_rgb; // RGB
	int stride_in_bytes = 3 * WIDTH;
	int result = stbi_write_png("result.png", w, h, comp, file_image, stride_in_bytes);


	free(file_image);
	free(image);
}

