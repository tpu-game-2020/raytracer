// src.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//
#include <iostream>

/// ■メモリリークチェック
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
// メモリリークチェック


/// ■ インクルード
// 標準ライブラリ
#include <math.h>

// stb(画像読み書きライブラリ)
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"


/// ■ 大事な定数
#define PI 3.141592653589793826433f

#define WIDTH 1920
#define HEIGHT 1080

/// ■ ベクトルライブラリ
typedef struct {
	float x, y, z;
}Vector3;

Vector3 add(const Vector3 v0, const Vector3 v1)
{
	Vector3 ret = {v0.x + v1.x, v0.y + v1.y, v0.z + v1.z};
	return ret;
}
Vector3 sub(const Vector3 v0, const Vector3 v1)
{
	Vector3 ret = { v0.x - v1.x, v0.y - v1.y, v0.z - v1.z };
	return ret;
}
Vector3 scale(const Vector3 v, float f)
{
	Vector3 ret = {v.x * f, v.y * f, v.z * f};
	return ret;
}
float dot(const Vector3 v0, const Vector3 v1) 
{
	return v0.x * v1.x + v0.y * v1.y + v0.z * v1.z;
}
Vector3 cross(const Vector3 v0, const Vector3 v1)
{
	Vector3 ret = {
		v0.y * v1.z - v0.z * v1.y,
		v0.z * v1.x - v0.x * v1.z,
		v0.x * v1.y - v0.y * v1.x
	};
	return ret;
}
Vector3 normalize(const Vector3 v)
{
	float len = sqrtf(dot(v, v));
	Vector3 ret = {v.x / len, v.y / len, v.z / len};
	return ret;
}

Vector3 get_ray_dir(float u, float v,
	const Vector3 camera_up, const Vector3 camera_dir,
	float fov, float aspect)
{
	float proj_x = u * 2.0f - 1.0f; // [0,1]->[-1,+1]
	float proj_y = -v * 2.0f + 1.0f; // [0,1]->[+1,-1] 画像とクリップ座標系はy軸反転

	Vector3 dx = normalize(cross(camera_dir, camera_up));
	Vector3 dy = normalize(cross(dx, camera_dir));

	float sy = tanf(0.5f * fov);
	float sx = sy / aspect;
	return normalize(add(normalize(camera_dir),
		add(scale(dx, proj_x * sx)
			, scale(dy, proj_y * sy))));
}

/// ■ オブジェクト
typedef struct {
	Vector3 albedo;        // 拡散反射係数
	float reflection;      // 反射係数
	float transmission;    // 透過係数
	float refraction_index;// 屈折率
} Material;

typedef struct {
	Vector3 center;
	float radisu;
	Material material;
} Sphere;

// 交差判定
float is_collide(const Sphere *p, const Vector3 pos, const Vector3 dir)
{
	Vector3 oc = sub(pos, p->center);
	float a = dot(dir, dir);
	float b = dot(oc, dir);
	float c = dot(oc, oc) - p->radisu * p->radisu;
	float discriminant = b * b - a * c;

	if (discriminant < 0.0f) return -1.0f; //衝突しませんでした

	// 手前の交点を返す
	float t = (-b - sqrtf(discriminant)) / a;
	if (0.0f < t) return t; // 正でなければだめ

	return (-b + sqrtf(discriminant)) / a;// 奥の交点を返す
}

/// ■ レイトレコア
Vector3 RayTracing(const Vector3 pos, const Vector3 dir, int depth = 0, float index = 1.0f)
{
	Vector3 col = { 1.00f,0.97f, 1.00f };// 何も衝突しないときは空な色

	// 何度も反射する場合は途中で打ち切り
	if (10 < ++depth) return col;

	// オブジェクト定義
	Sphere obj[] = {
		//  x        y        z          r       R      G      B     反射  透過  屈折率
		{{ 0.0f,    0.5f,    1.0f},     0.5f, {{1.00f, 0.00f, 0.00f}, 0.02f,0.0f} },// 空中の球(赤)
		{{ 2.0f,    0.5f,   -1.0f},     0.5f, {{0.02f, 0.80f, 0.10f}, 0.02f,0.93f, 2.4f} },// 空中の球(屈折)
		{{-2.0f,    0.5f,   -1.0f},     0.5f, {{0.00f, 0.00f, 0.00f}, 0.95f,0.0f} },// 空中の球(反射)
		{{ 0.0f, -100000.0f, 0.0f}, 100000.f, {{0.76f, 0.64f, 0.44f}, 0.0f, 0.0f} },// 地面
		{{1000.0f, 10000.0f,500.0f},  1000.f, {{1000.f,990.0f,980.0f},0.0f, 0.0f} },// 太陽
	};

	float min_t = 100000000000.0f;// 十分遠い場所
	int min_idx = -1;
	for (int i = 0; i < sizeof(obj)/sizeof(obj[0]); i++) 
	{
		// 一番近いオブジェクトを検索
		float t = is_collide(&obj[i], pos, dir);
		if (0.0001f < t && t < min_t) {// 何度も衝突するのを防ぐために少し前の値を採用
			min_t = t;
			min_idx = i;
		}
	}

	// 衝突しなかったら、空な色を返す
	if (min_idx < 0) return col;

	// 色計算
	const Sphere* p = &obj[min_idx];
	const Material* m = &p->material;
	Vector3 new_pos = add(pos, scale(dir, min_t));// 少し前に出して再判定されるのを防ぐ
	Vector3 normal = normalize(sub(new_pos, p->center));

	// 発光
	float emmisive = 1.0f - m->reflection - m->transmission;
	col = { 0.0f, 0.0f, 0.0f };// 発光がなければ黒
	if (0.0f < emmisive) {
		col = scale(p->material.albedo, emmisive);
	}

	// 反射
	if (0.0f < m->reflection) {
		Vector3 reflect = add(dir, scale(normal, -2.0f * dot(normal, dir)));
		col = add(col, scale(RayTracing(new_pos, reflect, depth, index), m->reflection));
	}
	// 屈折
	if (0.0f < m->transmission) {
		float new_index = (0.0f < dot(dir, normal)) ? 1.0f : m->refraction_index; // 出ていくか入っていくか?
		Vector3 vert = scale(normal, dot(dir, normal));// 垂直成分
		Vector3 pall = sub(dir, vert);// 平行成分
		Vector3 reflact = normalize(add(vert, scale(pall, index / new_index)));
		new_pos = add(new_pos, scale(reflact, 0.01f));
		col = add(col, scale(RayTracing(new_pos, reflact, depth, new_index), m->transmission));
	}

	return col;
}


/// ■ エントリーポイント
int main()
{
	// メモリリークチェック
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// メモリイメージの作成
	Vector3* image = (Vector3*)malloc(sizeof(Vector3) * WIDTH * HEIGHT);
	if (image == NULL) return -1;// メモリ確保の失敗

	// カメラ・スクリーンの設定
	float fov = 0.6f * 0.5f * PI;
	float aspect = (float)HEIGHT / (float)WIDTH;
	Vector3 camera_pos = {0.0f, 1.0f, 3.0f}; // カメラの位置
	Vector3 camera_up  = {0.0f, 1.0f, 0.0f};   // カメラの上方向
	Vector3 camera_dir = {0.0f,-0.2f, -1.0f};  // 視線方向
	
	// レイトレーシングの計算
	for (int y = 0; y < HEIGHT; y++) {
		for (int x = 0; x < WIDTH; x++) {
			float u = ((float)x + 0.5f) / (float)WIDTH;
			float v = ((float)y + 0.5f) / (float)HEIGHT;
			
			Vector3 dir = get_ray_dir(u, v, camera_up, camera_dir, fov, aspect);
			image[y * WIDTH + x] = RayTracing(camera_pos, dir);
		}
	}

	// 8bitのバッファに移す(1.0以上は白)
	unsigned char* file_image = (unsigned char*)malloc(3 * WIDTH * HEIGHT);
	if (file_image == NULL) { free(image); return -1; }// メモリを確保できなければ終了
	for (int y = 0; y < HEIGHT; y++) {
		unsigned char* p = &file_image[3 * y * WIDTH];
		const Vector3* s = &image[y * WIDTH];
		for (int x = 0; x < WIDTH; x++) {
			p[0] = (1.0f < s->x) ? 255 : (unsigned char)(255.9999f * s->x);
			p[1] = (1.0f < s->y) ? 255 : (unsigned char)(255.9999f * s->y);
			p[2] = (1.0f < s->z) ? 255 : (unsigned char)(255.9999f * s->z);
			p += 3;
			s++;
		}
	}

	// PNG ファイルとして保存
	int stride_in_bytes = 3 * WIDTH;
	int result = stbi_write_png("result.png", WIDTH, HEIGHT, STBI_rgb, file_image, stride_in_bytes);

	// 使用メモリの解放
	free(file_image);
	free(image);
}

