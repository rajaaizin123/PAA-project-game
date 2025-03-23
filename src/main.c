#define RAYGUI_IMPLEMENTATION

#include "raylib.h"
#include "raygui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/resource_dir.h"	// utility header for SearchAndSetResourceDir

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

typedef enum
{
	MENU_STATE,
	GAME_STATE
} GameState;

void resizeImage();

int main ()
{
	// Window Inilisiation
	const int screenWidth = 1100;
	const int screenHeight = 700;
	InitWindow(screenWidth, screenHeight, "Smart Kurir");

	// cek game sudah dimulai apa belum
	bool startgame = false;
	Vector2 KurirPos = {screenWidth / 2, screenHeight / 2}; // posisi awal

	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// set Frame FPS
	SetTargetFPS(60);
	// Start from the main menu
	GameState currentScreen = MENU_STATE;

	// “Start” button
	//Rectangle startButton = {screenWidth / 2 - 100, screenHeight / 2 - 30, 200, 60};

	// dirselector (handle upload png)
	GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
	char namaFileGambar[512] = { 0 };
	Texture map_texture = { 0 };
	Image map;

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	// game loop
	while (!WindowShouldClose())
	{
		BeginDrawing();
		ClearBackground(BLACK);

		if (currentScreen == MENU_STATE)
		{
			// Tombol Play the Game
			if (GuiButton((Rectangle){screenWidth / 2 - 50, screenHeight / 2 - 20, 100, 40}, "PLAY THE GAME"))
			{
				currentScreen = GAME_STATE;
				startgame = false; // Reset status game jika kembali ke menu
			}
		}
		else if (currentScreen == GAME_STATE)
		{
			if (fileDialogState.windowActive)
				GuiLock();

			// Tombol Pilih Map
			if (GuiButton((Rectangle){20, 20, 140, 30}, GuiIconText(ICON_FILE_OPEN, "Pilih Map")))
				fileDialogState.windowActive = true;

			// Tombol Start Game (di sebelah kanan tombol Pilih Map)
			if (!startgame)
			{
				if (GuiButton((Rectangle){180, 20, 140, 30}, "Start Game"))
				{
					startgame = true;
				}
			}

			GuiUnlock();

			// Tampilkan gambar map jika ada
			if (map_texture.id > 0)
			{
				DrawTexture(map_texture, GetScreenWidth() / 2 - map_texture.width / 2, GetScreenHeight() / 2 - map_texture.height / 2 + 10, WHITE);
			}

			// Menggambar karakter kurir jika game sudah dimulai
			if (startgame)
			{
				// Kontrol pergerakan kurir
				if (IsKeyDown(KEY_RIGHT) && KurirPos.x < screenWidth - 20)
					KurirPos.x += 5;
				if (IsKeyDown(KEY_LEFT) && KurirPos.x > 20)
					KurirPos.x -= 5;
				if (IsKeyDown(KEY_UP) && KurirPos.y > 20)
					KurirPos.y -= 5;
				if (IsKeyDown(KEY_DOWN) && KurirPos.y < screenHeight - 20)
					KurirPos.y += 5;

				// Menggambar kurir (segitiga)
				Vector2 v1 = {KurirPos.x, KurirPos.y - 20};
				Vector2 v2 = {KurirPos.x - 20, KurirPos.y + 20};
				Vector2 v3 = {KurirPos.x + 20, KurirPos.y + 20};

				DrawTriangle(v1, v2, v3, BLUE);
			}

			// GUI Window File Dialog
			GuiWindowFileDialog(&fileDialogState);
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