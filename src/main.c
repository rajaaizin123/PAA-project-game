#define RAYGUI_IMPLEMENTATION

#include "raylib.h"
#include "raygui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/resource_dir.h" // utility header for SearchAndSetResourceDir

#undef RAYGUI_IMPLEMENTATION // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

#define MAKS_KURIR 10
#define OFFSET_X 50
#define OFFSET_Y 70

typedef enum
{
	MENU_STATE,
	GAME_STATE
} GameState;

void resizeImage();

Vector2 RandomizePosisiKurir(Image map) {
	int percobaan = 0, max_percobaan = 100;

    while (percobaan < max_percobaan) {
        int rx = GetRandomValue(0, map.width - 1);
        int ry = GetRandomValue(0, map.height - 1);

        Color pixel = GetImageColor(map, rx, ry);
        // Cek apakah posisi ini sudah dipakai
        if (pixel.r == 83 && pixel.g == 119 && pixel.b == 94) {
            return (Vector2){rx + OFFSET_X, ry + OFFSET_Y};
        }

		percobaan++;
    }

	return (Vector2){-1, -1};
}

int main()
{
	// Window Inilisiation
	const int screenWidth = 1100;
	const int screenHeight = 700;
	InitWindow(screenWidth, screenHeight, "Smart Kurir");

	// cek game sudah dimulai apa belum
	bool startgame = false;
	Vector2 KurirPos[MAKS_KURIR];
	int posisi_terisi = 0;

	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// set Frame FPS
	SetTargetFPS(60);
	// Start from the main menu
	GameState currentScreen = MENU_STATE;

	// “Start” button
	// Rectangle startButton = {screenWidth / 2 - 100, screenHeight / 2 - 30, 200, 60};
	bool button_start = false;
	bool button_stop = false;
	bool button_random = false;


	// dirselector (handle upload png)
	GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
	char namaFileGambar[512] = {0};
	Texture map_texture = {0};
	Image map;

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

			//DrawTexture(map_texture, GetScreenWidth() / 2 - map.width / 2, GetScreenHeight() / 2 - map.height / 2+20 , WHITE);
			DrawTexture(map_texture, OFFSET_X, OFFSET_Y, WHITE);

			// DrawText(namaFileGambar, 208, GetScreenHeight() - 20, 10, GRAY);

			if (fileDialogState.windowActive)
				GuiLock();

			if (GuiButton((Rectangle){20, 20, 140, 30}, GuiIconText(ICON_FILE_OPEN, "Pilih Map")))
				fileDialogState.windowActive = true;
			// Tombol Start Game

			// deklarasi button
			button_start = GuiButton((Rectangle){170, 20, 140, 30}, "Start Game");
			button_stop = GuiButton((Rectangle){320, 20, 140, 30}, "Stop Game");
			button_random = GuiButton((Rectangle){470, 20, 140, 30}, "Random Mize");

			if (button_start)
			{
				startgame = true;
			}
			if (button_stop) // Geser ke kanan
			{
				startgame = false; // Mengubah nilai ke false untuk menghentikan game
			}

			static Vector2 titik_aspal = {-1, -1};
			if (button_random) // Tambahan tombol di bawah
			{
				titik_aspal = RandomizePosisiKurir(map);
			}

			if (titik_aspal.x != -1 && titik_aspal.y != -1) {
				DrawRectangle(titik_aspal.x, titik_aspal.y, 10, 10, WHITE);
			}

			GuiUnlock();
			// GUI: Dialog Window
			//--------------------------------------------------------------------------------
			GuiWindowFileDialog(&fileDialogState);
			//--------------------------------------------------------------------------------

			//----------------------------------------------------------------------------------

			// draw our texture to the screen
			// DrawTexture(wabbit, 50, 50, WHITE);
			// GuiButton((Rectangle){ 50, 50, 150, 40 }, "Open File");

			// proses...  cek warna pada tiap posisi pixel
			int mapPosX = GetScreenWidth() / 2 - map_texture.width / 2;
			int mapPosY = GetScreenHeight() / 2 - map_texture.height / 2 + 20;
			int pixelX = 290 - mapPosX;
			int pixelY = 400 - mapPosY;

			// DrawRectangle(250, 400, 10, 10, WHITE);
			// Color pixelColor = GetImageColor(map, 250, 400);
			// printf("Kode merah %d\n", pixelColor.r);
			// printf("Kode hijau %d\n", pixelColor.g);
			// printf("Kode biru %d\n", pixelColor.b);

			
		}

		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(map_texture);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
