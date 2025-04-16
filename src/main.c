#define RAYGUI_IMPLEMENTATION

#include "raylib.h"
#include "raygui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "raymath.h"

#include "../include/resource_dir.h" // utility header for SearchAndSetResourceDir

#undef RAYGUI_IMPLEMENTATION // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

#undef RAYGUI_IMPLEMENTATION
#define MAKS_KURIR 10
#define OFFSET_X 50
#define OFFSET_Y 70


// menambahkan fungsi unttuk membandingkan dua warna dengan toleransi
 bool isColorMatch(Color color1, Color color2, int tolerance){
	int diffR = abs(color1.r - color2.r);
	int diffG = abs(color1.g - color2.g);
 	int diffB = abs(color1.b - color2.b);

 	return diffR <= tolerance && diffG <= tolerance && diffB <=tolerance;
 }
typedef enum
{
	MENU_STATE,
	GAME_STATE
} GameState;

// typedef struct Vector2
// {
// 	float x;
// 	float y;
// } Vector2;

void resizeImage();
Vector2 RandomizePosisi(Image map){
// Vector2 RandomizePosisi(Image map, Color targetcolor, int tolerance) {
	int percobaan = 0, max_percobaan = 100;

    while (percobaan < max_percobaan) {
        int rx = GetRandomValue(0, map.width - 1);
        int ry = GetRandomValue(0, map.height - 1);

        Color pixel = GetImageColor(map, rx, ry);
		// if (pixel.r >= 70 && pixel.r <= 180 && pixel.g >= 70 && pixel.g <= 180 && pixel.b >= 70 && pixel.b <= 180){
		if (pixel.r == 83 && pixel.g == 119 && pixel.b == 94){
			return (Vector2){rx, ry};
		}

		percobaan++;
    }

	// for(int y = 0; y < map.height; y++){
	// 	for(int x = 0; x < map.width; x++){
	// 		Color pixel = GetImageColor(map, x,y);
	// 		if(isColorMatch(pixel, targetcolor, tolerance)){
	// 			return(Vector2){x,y};
	// 		}
	// 	}
	// }
	return (Vector2){-1, -1};
}

Vector2 PosisiValid(Image map, Image ukuran, Vector2 posisi_awal){
	// ambil warna
	Color pixel_patokan_kanan;
	int iterasi_pixel = ukuran.width;
	int pixel_outline_x = 0;
	for (int i = 1; i <= ukuran.width; i++){
		// titik ini dari sudut pandang gambar
		pixel_patokan_kanan = GetImageColor(map, posisi_awal.x + iterasi_pixel, posisi_awal.y);
		// if (pixel_patokan_kanan.r >= 90 && pixel_patokan_kanan.r <= 150 && pixel_patokan_kanan.g >= 90 && pixel_patokan_kanan.g <= 150 && pixel_patokan_kanan.b >= 90 && pixel_patokan_kanan.b <= 150){
		if (pixel_patokan_kanan.r == 83 && pixel_patokan_kanan.g == 119 && pixel_patokan_kanan.b == 94)
		{
			break;
		}

		iterasi_pixel--;
		pixel_outline_x = i;
	}

	// cek bawah
	Color pixel_patokan_bawah;
	iterasi_pixel = ukuran.height;
	int pixel_outline_y = 0;
	for (int i = 1; i <= ukuran.height; i++){
		pixel_patokan_bawah = GetImageColor(map, posisi_awal.x, posisi_awal.y + iterasi_pixel);
		// if (pixel_patokan_bawah.r >= 90 && pixel_patokan_bawah.r <= 150 && pixel_patokan_bawah.g >= 90 && pixel_patokan_bawah.g <= 150 && pixel_patokan_bawah.b >= 90 && pixel_patokan_bawah.b <= 150)
		// {
		if (pixel_patokan_bawah.r == 83 && pixel_patokan_bawah.g == 119 && pixel_patokan_bawah.b == 94)
		{
			break;
		}
		iterasi_pixel--;
		pixel_outline_y = i;
	}
	return (Vector2){pixel_outline_x, pixel_outline_y};
}

// Fungsi untuk menghitung arah pergerakan kurir ke arah tujuan
// Vector2 MoveKurir(Vector2 kurir_pos, Vector2 target_pos, float speed)
// {
// 	float angle = atan2(target_pos.y - kurir_pos.y, target_pos.x - kurir_pos.x); 
// 	kurir_pos.x += cos(angle) * speed;											
// 	kurir_pos.y += sin(angle) * speed;										
// 	return kurir_pos;
// }
// Vector2 MoveKurir(Vector2 current, Vector2 target, float speed)
// {
// 	Vector2 direction = {target.x - current.x, target.y - current.y};
// 	float length = sqrtf(direction.x * direction.x + direction.y * direction.y);

// 	if (length > 0)
// 	{
// 		direction.x /= length;
// 		direction.y /= length;
// 	}

// 	Vector2 result = {
// 		current.x + direction.x * speed,
// 		current.y + direction.y * speed};

// 	return result;
// }

// // Fungsi untuk menggambar kurir yang bergerak sepanjang jalur
// void DrawKurirAlongPath(Vector2 kurir_pos, Vector2 target_pos, Texture2D kurir_texture, float speed)
// {
// 	kurir_pos = MoveKurir(kurir_pos, target_pos, speed);		 
// 	DrawTexture(kurir_texture, kurir_pos.x, kurir_pos.y, WHITE); 
// }
int main()
{
	// Window Inilisiation
	const int screenWidth = 1100;
	const int screenHeight = 700;
	InitWindow(screenWidth, screenHeight, "Smart Kurir");

	// deklarasi warna target
	Color targetcolor = (Color){0, 255, 0, 255}; // Misalnya warna hijau

	// cek game sudah dimulai apa belum
	bool startgame = false;

	
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// set Frame FPS
	SetTargetFPS(60);
	// Start from the main menu
	GameState currentScreen = MENU_STATE;

	// “Start” button
	// Rectangle startButton = {screenWidth / 2 - 100, screenHeight / 2 - 30, 200, 60};
	// Tombol Start
	// Rectangle startButton = {50, 50, 100, 40};
	bool button_start = false;
	bool button_stop = false;
	bool button_random = false;

	// dirselector (handle upload png)
	GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
	char namaFileGambar[512] = {0};
	Texture map_texture = {0};
	Image map, kurir;

	Vector2 kurir_rorr = {-1, -1};
	Vector2 source = {-1, -1};
	Vector2 destination = {-1, -1};
	Vector2 outline = {-1, -1};

	// load gambar kurir
	kurir = LoadImage("resources/kurir_baru.png");
	Texture2D kurir_texture = LoadTextureFromImage(kurir);

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");  

	// game loop
	while (!WindowShouldClose())
	{
		//  proses drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		if (currentScreen == MENU_STATE)
		{
			if (GuiButton((Rectangle){screenWidth / 2 - 50, screenHeight / 2 - 20, 100, 40}, "PLAY THE GAME"))
			{
				currentScreen = GAME_STATE;
			}
		}
		else if (currentScreen == GAME_STATE)
		{
			if (fileDialogState.SelectFilePressed)
			{
				// Load image file (if supported extension)
				if (IsFileExtension(fileDialogState.fileNameText, ".png"))
				{
					// gabung direktori dan nama file
					strcpy(namaFileGambar, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));

					// loading message
					DrawText("Loading image...", screenWidth / 2 - 80, screenHeight / 2, 20, BLUE);
					EndDrawing();

					// Load image
					map = LoadImage(namaFileGambar);
					if (map.data == NULL)
					{
						printf("Gagal load gambar.\n");
					}
					else
					{
						UnloadTexture(map_texture);
						map_texture = LoadTextureFromImage(map);
					}
				}

				fileDialogState.SelectFilePressed = false;
			}

			DrawTexture(map_texture, OFFSET_X, OFFSET_Y, WHITE);

			if (fileDialogState.windowActive)
				GuiLock();

			if (GuiButton((Rectangle){20, 20, 140, 30}, GuiIconText(ICON_FILE_OPEN, "Pilih Map")))
				fileDialogState.windowActive = true;
			// Tombol Start Game

			// deklarasi button
			button_start = GuiButton((Rectangle){170, 20, 140, 30}, "Start Game");
			button_stop = GuiButton((Rectangle){320, 20, 140, 30}, "Stop Game");
			button_random = GuiButton((Rectangle){470, 20, 140, 30}, "Random Mize");

			// // Cek kalau mouse klik tombol
			// if (CheckCollisionPointRec(GetMousePosition(), button_start))
			// {
			// 	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
			// 	{
			// 		startgame = true;
			// 		kurir_rorr = (Vector2){100, 100};
			// 		source = (Vector2){400, 300};
			// 	}
			// }

			if (button_start && !startgame)
			{
				// Tombol Start ditekan, mulai permainan
				// startgame = true;

				// // Tentukan posisi awal kurir (misalnya bisa ditentukan secara acak atau tetap)
				// kurir_rorr = (Vector2){100, 100};					 // Posisi awal kurir
				// source = RandomizePosisi(map, targetcolor, 10);		 // Posisi sumber
				// destination = RandomizePosisi(map, targetcolor, 10); // Posisi tujuan

				// // Tambahkan logika untuk menghindari overlap antara source dan destination
				// while (destination.x == source.x && destination.y == source.y)
				// {
				// 	destination = RandomizePosisi(map, targetcolor, 10);
				// }
			}

			if (button_stop)
			{
				startgame = false; // Mengubah nilai ke false untuk menghentikan game
			}

			if (button_random)
			{
				kurir_rorr = RandomizePosisi(map);
				source = RandomizePosisi(map);

				if (source.x == kurir_rorr.x && source.y == kurir_rorr.y)
				{
					source = RandomizePosisi(map);
				}

				destination = RandomizePosisi(map);

				if ((destination.x == source.x && destination.y == source.x) || (destination.x == kurir_rorr.x && destination.y == kurir_rorr.y))
				{
					destination = RandomizePosisi(map);
				}
			}
			// if (button_random)
			// {
			// 	kurir_rorr = RandomizePosisi(map, targetcolor, 10);
			// 	source = RandomizePosisi(map, targetcolor, 10);

			// 	if (source.x == kurir_rorr.x && source.y == kurir_rorr.y)
			// 	{
			// 		source = RandomizePosisi(map, targetcolor, 10);
			// 	}

			// 	destination = RandomizePosisi(map, targetcolor, 10);

			// 	if ((destination.x == source.x && destination.y == source.y) ||
			// 		(destination.x == kurir_rorr.x && destination.y == kurir_rorr.y))
			// 	{
			// 		destination = RandomizePosisi(map, targetcolor, 10);
			// 	}
			// }

			if (kurir_rorr.x != -1 && kurir_rorr.y != -1)
			{
				outline = PosisiValid(map, kurir, kurir_rorr);
				DrawTexture(kurir_texture, kurir_rorr.x - outline.x + OFFSET_X, kurir_rorr.y - outline.y + OFFSET_Y, WHITE);

				outline = PosisiValid(map, kurir, source);
				DrawRectangle(source.x + OFFSET_X - outline.x, source.y + OFFSET_Y - outline.y, 20, 20, YELLOW);

				outline = PosisiValid(map, kurir, destination);
				DrawRectangle(destination.x + OFFSET_X - outline.x, destination.y + OFFSET_Y - outline.y, 20, 20, RED);
			

				// // ambil warna
				// Color pixel_patokan_kanan;
				// int iterasi_pixel = kurir.width;
				// int pixel_outline_x = 0;
				// for (int i = 1; i <= kurir.width; i++){
				// 	// titik ini dari sudut pandang gambar
				// 	pixel_patokan_kanan = GetImageColor(map, titik_aspal.x + iterasi_pixel, titik_aspal.y);
				// 	if (pixel_patokan_kanan.r == 83 && pixel_patokan_kanan.g == 119 && pixel_patokan_kanan.b == 94){
				// 		break;
				// 	}

				// 	iterasi_pixel--;
				// 	pixel_outline_x = i;
				// }

				// // cek bawah
				// Color pixel_patokan_bawah;
				// iterasi_pixel = kurir.height;
				// int pixel_outline_y = 0;
				// for (int i = 1; i <= kurir.height; i++){
				// 	pixel_patokan_bawah = GetImageColor(map, titik_aspal.x, titik_aspal.y + iterasi_pixel);
				// 	if (pixel_patokan_bawah.r == 83 && pixel_patokan_bawah.g == 119 && pixel_patokan_bawah.b == 94){
				// 		break;
				// 	}

				// 	iterasi_pixel--;
				// 	pixel_outline_y = i;
				// }

				// cek posisi dan warnanya
				// Color pixelColor = GetImageColor(map, titik_aspal.x - OFFSET_X, titik_aspal.y - OFFSET_Y);
				// printf("Posisi anda x: %d  y: %d\n", titik_aspal.x, titik_aspal.y);
				// printf("Kode merah %d\n", pixel_patokan_kanan.r);
				// printf("Kode hijau %d\n", pixel_patokan_kanan.g);
				// printf("Kode biru %d\n", pixel_patokan_kanan.b);
				// //printf("pixel outline: %d\n", pixel_outline);
				// DrawTexture(kurir_texture, titik_aspal.x + OFFSET_X - pixel_outline_x , titik_aspal.y + OFFSET_Y - pixel_outline_y, WHITE);

				// cek kanan
				// if ((pixel_patokan_kanan.r != 83 && pixel_patokan_kanan.g != 119 && pixel_patokan_kanan.b != 94) &&// cek kanan
				// 	(pixel_patokan_kiri.r  != 83 && pixel_patokan_kiri.g  != 119 && pixel_patokan_kiri.b  != 94) &&
				// 	(pixel_patokan_bawah.r != 83 && pixel_patokan_bawah.g != 119 && pixel_patokan_bawah.b != 94) && // cek bawah
				// 	(pixel_patokan_atas.r  != 83 && pixel_patokan_atas.g  != 119 && pixel_patokan_atas.b  != 94)// cek atas

				// 	){
				// 	DrawTexture(kurir_texture, titik_aspal.x, titik_aspal.y, WHITE);
				// }
			}

			GuiUnlock();
			// GUI: Dialog Window
			//--------------------------------------------------------------------------------
			GuiWindowFileDialog(&fileDialogState);
		}

		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(map_texture);
	UnloadTexture(kurir_texture);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
