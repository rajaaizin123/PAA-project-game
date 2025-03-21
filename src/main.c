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

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

	// Create the window and OpenGL context
	InitWindow(1100, 700, "Kurir Wahabi");

	GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
	char namaFileGambar[512] = { 0 };
	Texture texture = { 0 };

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

	//Image map = LoadImage("map.png");
	//Texture mapTexture = LoadTextureFromImage(map);

	bool tombol = false;
	SetTargetFPS(60);

	// game loop
	while (!WindowShouldClose())		// run the loop untill the user presses ESCAPE or presses the Close button on the window
	{

		if (fileDialogState.SelectFilePressed)
        {
            // Load image file (if supported extension)
            if (IsFileExtension(fileDialogState.fileNameText, ".png"))
            {
                strcpy(namaFileGambar, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
                UnloadTexture(texture);
                texture = LoadTexture(namaFileGambar);
            }

            fileDialogState.SelectFilePressed = false;
        }

		// drawing
		BeginDrawing();

		// Setup the back buffer for drawing (clear color and depth buffers)
		ClearBackground(BLACK);

		DrawTexture(texture, GetScreenWidth()/2 - texture.width/2, GetScreenHeight()/2 - texture.height/2 - 5, WHITE);
		DrawRectangleLines(GetScreenWidth()/2 - texture.width/2, GetScreenHeight()/2 - texture.height/2 - 5, texture.width, texture.height, BLACK);

		DrawText(namaFileGambar, 208, GetScreenHeight() - 20, 10, GRAY);

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

		// proses...
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
	UnloadTexture(texture);

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}