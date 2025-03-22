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

#include "../include/resource_dir.h"	// utility header for SearchAndSetResourceDir

#undef RAYGUI_IMPLEMENTATION            // Avoid including raygui implementation again

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "gui_window_file_dialog.h"

int main ()
{
	// Tell the window to use vsync and work on high DPI displays
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);

<<<<<<< HEAD
=======
	// Create the window and OpenGL context
>>>>>>> 87c171c (update main.c)
	InitWindow(1100, 700, "Smart Kurir");

	GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
	char namaFileGambar[512] = { 0 };
	Texture map_texture = { 0 };
<<<<<<< HEAD

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");
	
=======
	Image map;

	// Utility function from resource_dir.h to find the resources folder and set it as the current working directory so we can load from it
	SearchAndSetResourceDir("resources");

>>>>>>> 87c171c (update main.c)
	bool tombol = false;
	SetTargetFPS(60);

	// game loop
<<<<<<< HEAD
	while (!WindowShouldClose()){
=======
	while (!WindowShouldClose())		
	{
>>>>>>> 87c171c (update main.c)

	    if (fileDialogState.SelectFilePressed){
            // Load image file (if supported extension)
<<<<<<< HEAD
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
=======
            if (IsFileExtension(fileDialogState.fileNameText, ".png"))
            {
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
>>>>>>> 87c171c (update main.c)
        }

	  // drawing
	  BeginDrawing();

	  // Setup the back buffer for drawing (clear color and depth buffers)
	  ClearBackground(BLACK);

<<<<<<< HEAD
          DrawTexture(map_texture, GetScreenWidth()/2 - map_texture.width/2, GetScreenHeight()/2 - map_texture.height/2 - 5, WHITE);
	  DrawRectangleLines(GetScreenWidth()/2 - map_texture.width/2, GetScreenHeight()/2 - map_texture.height/2 - 5, map_texture.width, map_texture.height, BLACK);

	  //DrawText(namaFileGambar, 208, GetScreenHeight() - 20, 10, GRAY);
=======
		DrawTexture(map_texture, GetScreenWidth()/2 - map_texture.width/2, GetScreenHeight()/2 - map_texture.height/2 - 5, WHITE);
		DrawRectangleLines(GetScreenWidth()/2 - map_texture.width/2, GetScreenHeight()/2 - map_texture.height/2 - 5, map_texture.width, map_texture.height, BLACK);

		//DrawText(namaFileGambar, 208, GetScreenHeight() - 20, 10, GRAY);
>>>>>>> 87c171c (update main.c)

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

<<<<<<< HEAD
		// proses...  cek warna pada tiap posisi pixel
		//Color pixelColor = GetImageColor(map, 60, 60);
		// printf("Kode merah %d\n", pixelColor.r);
		// printf("Kode hijau %d\n", pixelColor.g);
		// printf("Kode biru %d\n", pixelColor.b);
=======
		// proses... cek warna pada tiap posisi pixel 
		// Color pixelColor = GetImageColor(map, 60, 60);
		//  printf("Kode merah %d\n", pixelColor.r);
		//  printf("Kode hijau %d\n", pixelColor.g);
		//  printf("Kode biru %d\n", pixelColor.b);
>>>>>>> 87c171c (update main.c)

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
