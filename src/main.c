/*
Raylib example file.
This is an example main file for a simple raylib project.
Use this as a starting point or replace it with your code.

by Jeffery Myers is marked with CC0 1.0. To view a copy of this license, visit https://creativecommons.org/publicdomain/zero/1.0/

*/

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
int main ()
{
	// Window Inilisiation
	const int screenWidth = 800;
	const int screenHeight = 600;
	InitWindow(screenWidth, GetScreenHeight, "Game Start");

	// set Frame FPS
	SetTargetFPS(60);
	// Start from the main menu
	GameState currentScreen = MENU_STATE;

	// “Start” button
	Rectangle startButton = {screenWidth / 2 - 100, screenHeight / 2 - 30, 200, 60};

	// Looping Game
	while (!WindowShouldClose())
	{
		Vector2 mousepoint = GetMousePosition();
		// check keyboard input
		if (currentScreen == MENU_STATE)
		{
			if (GuiButton((Rectangle){screenWidth / 2 - 50, screenHeight / 2 - 20, 100, 40}, "START"))
			{
				currentScreen = GAME_STATE;
			}
		}

		BeginDrawing();
		ClearBackground(BLACK);
		if (currentScreen == MENU_STATE)
		{
			GuiButton((Rectangle){screenWidth / 2 - 50, screenHeight / 2 - 20, 100, 40}, "START");
		}
		else if (currentScreen == GAME_STATE)
		{
			DrawText("Game is Running!", screenWidth / 2 - 80, screenHeight / 2, 20, BLUE);
		}

		EndDrawing();
	}

	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	InitWindow(1100, 700, "Smart Kurir");

	GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
	char namaFileGambar[512] = { 0 };
	Texture map_texture = { 0 };
	Image map;

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");
	
	bool tombol = false;
	SetTargetFPS(60);

	// game loop
	while (!WindowShouldClose()){

	    if (fileDialogState.SelectFilePressed){
            // Load image file (if supported extension)
	            if (IsFileExtension(fileDialogState.fileNameText, ".png")){
			// gabung direktori dan nama file
	                strcpy(namaFileGambar, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
	
			// Load image
			map = LoadImage(namaFileGambar);
			if (map.data == NULL){
				printf("Gagal load gambar.\n");
			}else{
				UnloadTexture(map_texture);
				map_texture = LoadTextureFromImage(map);
			}
	            }
	
	            fileDialogState.SelectFilePressed = false;
        }

	  // drawing
	  BeginDrawing();
	 
	  // Setup the back buffer for drawing (clear color and depth buffers)
	  ClearBackground(BLACK);

          DrawTexture(map_texture, GetScreenWidth()/2 - map_texture.width/2, GetScreenHeight()/2 - map_texture.height/2 - 5, WHITE);
	  DrawRectangleLines(GetScreenWidth()/2 - map_texture.width/2, GetScreenHeight()/2 - map_texture.height/2 - 5, map_texture.width, map_texture.height, BLACK);

	  //DrawText(namaFileGambar, 208, GetScreenHeight() - 20, 10, GRAY);

          //tombol = GuiButton((Rectangle){ 200, 300, 100, 50 }, "Pilih Map");
		
	   if (fileDialogState.windowActive) GuiLock();

	   if (GuiButton((Rectangle){ 20, 20, 140, 30 }, GuiIconText(ICON_FILE_OPEN, "Pilih Map"))) fileDialogState.windowActive = true;

	   GuiUnlock();
		// GUI: Dialog Window
		//--------------------------------------------------------------------------------
		GuiWindowFileDialog(&fileDialogState);
		//--------------------------------------------------------------------------------

		//----------------------------------------------------------------------------------


		// draw our texture to the screen
		//DrawTexture(wabbit, 50, 50, WHITE);
		//GuiButton((Rectangle){ 50, 50, 150, 40 }, "Open File");

		// proses...  cek warna pada tiap posisi pixel
		//Color pixelColor = GetImageColor(map, 60, 60);
		// printf("Kode merah %d\n", pixelColor.r);
		// printf("Kode hijau %d\n", pixelColor.g);
		// printf("Kode biru %d\n", pixelColor.b);

		//DrawTexture(mapTexture, 50, 50, WHITE);

		// end the frame and get ready for the next one  (display frame, poll input, etc...)
		EndDrawing();
	}

	// cleanup
	// unload our texture so it can be cleaned up
	UnloadTexture(map_texture);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}