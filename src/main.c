#define RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raymath.h>

#include "../include/resource_dir.h" // utility header for SearchAndSetResourceDir
#include "gui_window_file_dialog.h"

#undef RAYGUI_IMPLEMENTATION
#define MAKS_KURIR 25
#define OFFSET_X 50
#define OFFSET_Y 70
#define GRID 25 // ini sekaligus lebar aspal (defaultnya)

#define OFFSET_KURIR 12

typedef enum{
	MENU_STATE,
	GAME_STATE
} GameState;

void resizeImage();

bool isWarnaAspal(Color pixel){
	if ( (pixel.r >= 90 && pixel.r <= 150) && 
		 (pixel.g >= 90 && pixel.g <= 150) && 
		 (pixel.b >= 90 && pixel.b <= 150)){

			return true;
	}

	return false;
}

unsigned char** LoadMapKeArray(Image map) {
	unsigned char ** jalanMap = (unsigned char**)malloc(map.height * sizeof(unsigned char**));

    for (int y = 0; y < map.height; y++) {
        jalanMap[y] = (unsigned char*)malloc(map.width * sizeof(unsigned char));

        for (int x = 0; x < map.width; x++) {
            Color pixel = GetImageColor(map, x, y);
            jalanMap[y][x] = isWarnaAspal(pixel) ? '1' : '0';
        }
    }

	return jalanMap;
}

//Vector2 RandomizePosisi(Image map){
Vector2 RandomizePosisi(Image map) {
	int percobaan = 0, max_percobaan = 100;

	while (percobaan < max_percobaan){
		int rx = GetRandomValue(0, map.width - 1);
		int ry = GetRandomValue(0, map.height - 1);

		Color pixel = GetImageColor(map, rx, ry);
		if (isWarnaAspal(pixel)){
			return (Vector2){rx, ry};
		}

	 	percobaan++;
	}

	return (Vector2){-1, -1};
}

Vector2 PosisiValid(Image map, Image ukuran, Vector2 posisi_awal){
	// ambil warna
	Color pixel_patokan_kanan;
	int iterasi_pixel = ukuran.width;
	int pixel_outline_x = 0;
	for (int i = 1; i <= ukuran.width; i++)
	{
		// titik ini dari sudut pandang gambar
		pixel_patokan_kanan = GetImageColor(map, posisi_awal.x + iterasi_pixel, posisi_awal.y);
		if (isWarnaAspal(pixel_patokan_kanan)){
			break;
		}

		iterasi_pixel--;
		pixel_outline_x = i;
	}

	// cek bawah
	Color pixel_patokan_bawah;
	iterasi_pixel = ukuran.height;
	int pixel_outline_y = 0;
	for (int i = 1; i <= ukuran.height; i++)
	{
		pixel_patokan_bawah = GetImageColor(map, posisi_awal.x, posisi_awal.y + iterasi_pixel);
		if (isWarnaAspal(pixel_patokan_bawah)){
			break;
		}
		iterasi_pixel--;
		pixel_outline_y = i;
	}
	return (Vector2){pixel_outline_x, pixel_outline_y};
}

//Fungsi untuk menghitung arah pergerakan kurir ke arah tujuan
// Vector2 MoveKurir(Vector2 kurir_pos, Vector2 target_pos, float speed)
// {
// 	float angle = atan2(target_pos.y - kurir_pos.y, target_pos.x - kurir_pos.x);
// 	kurir_pos.x += cos(angle) * speed;
// 	kurir_pos.y += sin(angle) * speed;
// 	return kurir_pos;
// }

void DrawGrid__(int screen_width, int screen_height){
	for (int i = 0; i < 44; i++){
		DrawLine(0, i * GRID, screen_width, i * GRID, BLUE);

		DrawLine(i * GRID, 0, i * GRID, screen_height, BLUE);
	}
}

Vector2 posisiPojok(Vector2 current, unsigned char** dataJalan, int tinggi_map, int lebar_map) {
    int cx = (int)current.x;
    int cy = (int)current.y;

    // cek ke atas
    for (int y = cy; y >= 0; y--) {
        if (dataJalan[y][cx] == '0') {
            return (Vector2){current.x, y};
        }
    }

    // cek ke bawah
    for (int y = cy; y < tinggi_map; y++) {
        if (dataJalan[y][cx] == '0') {
            return (Vector2){current.x, y};
        }
    }

    // cek ke kiri
    for (int x = cx; x >= 0; x--) {
        if (dataJalan[cy][x] == '0') {
            return (Vector2){x, current.y};
        }
    }

    // cek ke kanan
    for (int x = cx; x < lebar_map; x++) {
        if (dataJalan[cy][x] == '0') {
            return (Vector2){x, current.y};
        }
    }

    return (Vector2){-1, -1};
}

#define SIZE_MEMO 5
typedef struct {
	int x[SIZE_MEMO];
	int y[SIZE_MEMO];
	int head;
} PosMemo;

void InitMemo(PosMemo *pos_memo){
	for (int i = 0; i < SIZE_MEMO; i++){
		pos_memo->x[i] = -1;
		pos_memo->y[i] = -1;
	}
	pos_memo->head = 0;
}

bool sudahDikunjungi(PosMemo *pos_memo, int x, int y) {
    for (int i = 0; i < SIZE_MEMO; i++) {
        if (pos_memo->x[i] == x && pos_memo->y[i] == y) {
            return true;
        }
    }
    return false;
}

void AddToMemo(PosMemo *pos_memo, int x, int y) {
    pos_memo->x[pos_memo->head] = x;
    pos_memo->y[pos_memo->head] = y;
    pos_memo->head = (pos_memo->head + 1) % SIZE_MEMO;
}

//* versi 0.1
Vector2 GerakKurir1(Vector2 target, Vector2 current, unsigned char** dataJalan, int *memo_arah, int speed, PosMemo *pos_memo){
	int x = (int)current.x;
    int y = (int)current.y;
    int tx = (int)target.x;
    int ty = (int)target.y;

	int x_memo = (int)current.x;
	int y_memo = (int)current.y;

	// kode arah ---> 1:atas, 2:bawah, 3: kanan, 4: kiri, 5:miring-kanan-atas, 6:miring-kiri-atas, 
	//                7:miring-kanan-bawah, 8:miring-kiri-bawah
	if (tx > x) *memo_arah = 3;
	else if (tx < x) *memo_arah = 4;

	if (ty > y) *memo_arah = 2;
	else if (ty < y) *memo_arah = 1;

	// titik sebelumnya
	

	// pergerakan ke = atas - bawah - kanan - kiri
	// kasus jalan diagonal = atas - bawah - kanan - kiri

	// if (sudahDikunjungi(pos_memo, x, y)){
	// 	if (*memo_arah == 1){
	// 		return (Vector2){x, y - 1};
	// 	}else if (*memo_arah == 2){
	// 		return (Vector2){x, y + 1};
	// 	}else if (*memo_arah == 3){
	// 		return (Vector2){x + 1, y};
	// 	}else if (*memo_arah == 4){
	// 		return (Vector2){x - 1, y};
	// 	}
	// }

	if (dataJalan[y - 1][x] == '1' && *memo_arah != 2){
		// ------------  Kasus diagonal --------------- //
		// tujuan ada di kanan 
		// -- dengan jalan miring ke kanan
		if (dataJalan[y][x + 1] == '0' && dataJalan[y - 1][x + 1] == '1' && *memo_arah == 3){
			x += 1;
			y -= 1;
			*memo_arah = 5;
		}
		// -- dengan jalan miring ke kiri
		else if (dataJalan[y][x + 1] == '0' && dataJalan[y - 1][x] == '0' && dataJalan[y - 1][x - 1] == '1' && *memo_arah == 3){
			x -= 1;
			y -= 1;
			*memo_arah = 6;
		}

		// tujuan ada di kiri
		// -- dengan jalan miring ke kanan
		else if (dataJalan[y][x - 1] == '0' && dataJalan[y - 1][x] == '0' && dataJalan[y - 1][x + 1] == '1' && *memo_arah == 4){
			x += 1;
			y -= 1;
			*memo_arah = 5;
		}
		// -- dengan jalan miring ke kiri
		else if (dataJalan[y][x - 1] == '0' && dataJalan[y - 1][x - 1] == '1' && *memo_arah == 4){
			x -= 1;
			y -= 1;
			*memo_arah = 6;
		}


		// ----------- Kasus tidak diagonal -------------- //
		else {
			x = (int)current.x;
			y -= 1;
			*memo_arah = 1;
		}
	}
	else if (dataJalan[y + 1][x] == '1' && *memo_arah != 1){
		// ------------  Kasus diagonal --------------- //
		// tujuan ada di kanan 
		// -- dengan jalan miring ke kanan
		if (dataJalan[y][x + 1] == '0' && dataJalan[y + 1][x + 1] == '1' && *memo_arah == 3){
			x += 1;
			y += 1;
			*memo_arah = 7;
		}
		// -- dengan jalan miring ke kiri
		else if (dataJalan[y][x + 1] == '0' && dataJalan[y + 1][x] == '0' && dataJalan[y + 1][x - 1] == '1' && *memo_arah == 3){
			x -= 1;
			y += 1;
			*memo_arah = 8;
		}

		// tujuan ada di kiri
		// -- dengan jalan miring ke kanan
		else if (dataJalan[y + 1][x] == '0' && dataJalan[y][x - 1] == '0' && dataJalan[y + 1][x + 1] == '1' && *memo_arah == 4){
			x += 1;
			y += 1;
			*memo_arah = 7;
		}
		// -- dengan jalan miring ke kiri
		else if (dataJalan[y][x - 1] == '0' && dataJalan[y + 1][x - 1] == '1' && *memo_arah == 4){
			x -= 1;
			y += 1;
			*memo_arah = 8;
		}

		// ----------- Kasus tidak diagonal -------------- //
		else {
			x = (int)current.x;
			y += 1;
			*memo_arah = 2;
		}
	}
	else if (dataJalan[y][x + 1] == '1'){
		// hanya jalan ke kanan jika sebelumnya belum pernah jalan ke kiri
		if (*memo_arah != 4 || dataJalan[y][x - 1] == '0'){
			x += 1;
			y = (int)current.y;
			*memo_arah = 3;
		}
	}
	else if (dataJalan[y][x - 1] == '1'){
		if (*memo_arah != 3 || dataJalan[y][x + 1] == '0'){
			x -= 1;
			y = (int)current.y;
			*memo_arah = 4;
		}
	}

	
	AddToMemo(pos_memo, (int)current.x, (int)current.y);

	printf("Aspal atas: %c\n", dataJalan[y - 1][x]);
	printf("Aspal bawah: %c\n", dataJalan[y + 1][x]);
	printf("Aspal kanan: %c\n", dataJalan[y][x + 1]);
	printf("Aspal kiri: %c\n\n", dataJalan[y][x - 1]);

	printf("source --> x: %d, y: %d\n", x, y);
	printf("Arah: %d\n", *memo_arah);
	// printf("target --> x: %.2f, y: %.2f\n", target.x, target.y);
	// printf("dir x:%.2f\n", direction.x);
	// printf("dir y:%.2f\n", direction.y);
	
	return (Vector2){x, y};
	
}


// versi 0.2
Vector2 GerakKurir2(Vector2 target, Vector2 current, unsigned char** dataJalan, int *memo_arah, int speed, PosMemo *pos_memo) {
    int x = (int)current.x;
    int y = (int)current.y;
    int tx = (int)target.x;
    int ty = (int)target.y;

    int step_x = 0, step_y = 0;

    if (tx > x) {
		step_x = 1;
		*memo_arah = 1;
	}
    else if (tx < x) {
		step_x = -1;
		*memo_arah = 2;
	}

    if (ty > y) {
		step_y = 1;
		*memo_arah = 3;
	}

    else if (ty < y) {
		step_y = -1;
		*memo_arah = 4;
	}

    // Prioritas gerak diagonal (miring)
	printf("arah: %d\n", *memo_arah);
	printf("x: %d, y: %d\n", x, y);
	if (sudahDikunjungi(pos_memo, x, y)){
		if (*memo_arah == 1 && dataJalan[y][x + 1] == '1'){
			return (Vector2){x + 1, y};
		}else if (ty > y && dataJalan[y][x + 1] == '0'){
			return (Vector2){x, y + 1};
		}else if (ty < y && dataJalan[y][x + 1] == '0'){
			return (Vector2){x, y - 1};
		}else {
			return (Vector2){x, y + 1};
		}
		
		if (*memo_arah == 2 && dataJalan[y][x - 1] == '1'){
			return (Vector2){x - 1, y};
		}else if (ty > y && dataJalan[y][x - 1] == '0'){
			return (Vector2){x, y + 1};
		}else if (ty < y && dataJalan[y][x - 1] == '0'){
			return (Vector2){x, y - 1};
		}else {
			return (Vector2){x, y + 1};
		}
		
		if (*memo_arah == 3 && dataJalan[y + 1][x] == '1'){
			return (Vector2){x, y + 1};
		}else if (tx > x && dataJalan[y + 1][x] == '0'){
			return (Vector2){x + 1, y};
		}else if (tx < x && dataJalan[y + 1][x] == '0'){
			return (Vector2){x - 1, y};
		}

		if (*memo_arah == 4 && dataJalan[y - 1][x] == '1'){
			return (Vector2){x, y - 1};
		}else if (tx > x && dataJalan[y - 1][x] == '0'){
			return (Vector2){x + 1, y};
		}else if (tx < x && dataJalan[y - 1][x] == '0'){
			return (Vector2){x - 1, y};
		}
	}

	if (dataJalan[y + step_y][x + step_x] == '1') {
        x += step_x;
        y += step_y;
    }
    else if (dataJalan[y + step_y][x] == '1') {
        y += step_y;
    }
    else if (dataJalan[y][x + step_x] == '1') { 
        x += step_x;
	}

    return (Vector2){x, y}; 
}

int LawanArah(int arah) {
    switch (arah) {
        case 1: return 2;  // kanan ←→ kiri
        case 2: return 1;
        case 3: return 4;  // bawah ←→ atas
        case 4: return 3;
        case 5: return 8;  // kanan bawah ←→ kiri atas
        case 6: return 7;  // kiri bawah ←→ kanan atas
        case 7: return 6;
        case 8: return 5;
        default: return 0;
    }
}

// versi 0.3
Vector2 GerakKurir(Vector2 target, Vector2 current, unsigned char** dataJalan, int *memo_arah, int speed, PosMemo *pos_memo, int *rotasi) {
    int x = (int)current.x;
    int y = (int)current.y;
    int tx = (int)target.x;
    int ty = (int)target.y;

    AddToMemo(pos_memo, x, y);

    Vector2 bestMove = current;
    float bestScore = 1e9;

    Vector2 arah[8] = {
        {1, 0},   // kanan
        {-1, 0},  // kiri
        {0, 1},   // bawah
        {0, -1},  // atas
        {1, 1},   // kanan bawah
        {-1, 1},  // kiri bawah
        {1, -1},  // kanan atas
        {-1, -1}  // kiri atas
    };

    for (int i = 0; i < 8; i++) {
        int dx = (int)arah[i].x;
        int dy = (int)arah[i].y;
        int nx = x + dx;
        int ny = y + dy;

        if (dataJalan[ny][nx] == '1' && !sudahDikunjungi(pos_memo, nx, ny)) {
            Vector2 next = {nx, ny};
            float dist = Vector2Distance(target, next);

			// if ((i + 1) == LawanArah(*memo_arah)) {
            //     dist += 1.0f;
            // }

			printf("dist: %.2f\n", dist);
			printf("best score: %.2f\n", bestScore);

            if (dist < bestScore) {
                bestScore = dist;
                bestMove = next;
                *memo_arah = i + 1;
				//float ang = atan2f(next.y - current.y, next.x - current.x) * 57.2957795f; // RAD2DEG
                //*rotasi = (int)ang;
            }
        }
    }

	printf("x: %.2f, y: %.2f\n", bestMove.x, bestMove.y);
    return bestMove;
}


Vector2 MoveKurir(Vector2 current, Vector2 target, float speed){
   	Vector2 direction = {target.x - current.x, target.y - current.y};
   	float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
	
   	if (length > 0){
   		direction.x /= length;
   		direction.y /= length;
   	}

	printf("dir x: %.2f\n", direction.x);
	printf("dir y: %.2f\n", direction.y);

   	Vector2 result = {
   		current.x + direction.x * speed,
   		current.y + direction.y * speed};

   	return (Vector2)result;
  }

/***

Vector2 MoveKurir(Vector2 current, Vector2 target, float speed){
 	int direction_x = target.x - current.x;
 	int direction_y = target.y - current.y;

 	float length = sqrtf(direction_x * direction_x + direction_y * direction_y);

 	// kalo target.x > current.x dan target.y == current.y maka (tujuan = kuadran I, current = kuadran II)
 	// if (direction_x == 0){
 	// }

	Vector2 result = {
		current.x + 1 * speed,
		current.y + 0,
	};

	return (Vector2)result;
}

***/

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

	// button
	bool button_start = false;
	bool button_stop = false;
	bool button_random = false;

	// dirselector (handle upload png)
	GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
	char namaFileGambar[512] = {0};
	Texture map_texture = {0};
	Image map, kurir;

	// load data aspal ke array
	unsigned char** dataJalan;

	// load grid map
	Vector2 kurir_rorr = {-1, -1};
	Vector2 source = {-1, -1};
	Vector2 destination = {-1, -1};
	Vector2 outline = {-1, -1};
	int memo_arah = 0;
	Vector2 memo_pos = {-1, -1};
	PosMemo pos_memo;
	InitMemo(&pos_memo);

	// hal hal yang berhubungan dengan entitas kurir
	kurir = LoadImage("resources/kurir_new2.png");
	Texture2D kurir_texture = LoadTextureFromImage(kurir);
	Rectangle sourceRecKurir = {0.f, 0.f, (float)kurir.width, (float)kurir.height};
	//Rectangle destRecKurir = {kurir_rorr.x, kurir_rorr.y, (float)kurir.width, (float)kurir.height};
	Vector2 origin = { (float)kurir.width / 2.0f, (float)kurir.height / 2.0f};
	int rotation = 0;


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
						dataJalan = LoadMapKeArray(map);
					}

					EndDrawing();
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
			// if (CheckCollisionPointRec(GetMousePosition(), button_start)){
			//  	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
			//  	{
			//  		startgame = true;
			// 		kurir_rorr = (Vector2){100, 100};
			// 		source = (Vector2){400, 300};
			// 	}
			// }

			if (button_start && !startgame)
			{
				//Tombol Start ditekan, mulai permainan
				startgame = true;

				// Tentukan posisi awal kurir (misalnya bisa ditentukan secara acak atau tetap)
				// kurir_rorr = (Vector2){100, 100};					 // Posisi awal kurir
				// source = RandomizePosisi(map, targetcolor, 10);		 // Posisi sumber
				// destination = RandomizePosisi(map, targetcolor, 10); // Posisi tujuan


				// Tambahkan logika untuk menghindari overlap antara source dan destination
				// while (destination.x == source.x && destination.y == source.y)
				// {
				// 	destination = RandomizePosisi(map, targetcolor, 10);
				// }

			}			

			if (button_stop)
			{
				startgame = false; // Mengubah nilai ke false untuk menghentikan game
			}

			// if (button_random){

			// 	kurir_rorr = RandomizePosisi(map, targetcolor + OFFSET_KURIR, 10);
			// 	source = RandomizePosisi(map, targetcolor + OFFSET_KURIR, 10);

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

			if (startgame) {
				int x = (int)kurir_rorr.x;
				int y = (int)kurir_rorr.y;

				int x_sc = (int)source.x;
				int y_sc = (int)source.y;

				int distance_x = x_sc - x;
				int distance_y = y_sc - y;

				memo_pos.x = x;
				memo_pos.y = y;
				kurir_rorr = GerakKurir(source, kurir_rorr, dataJalan, &memo_arah, 1.0f, &pos_memo, &rotation);
				//DrawPixel(kurir_rorr.x, kurir_rorr.y, RED);
			
				//if (x >= 0 && x < map.width && y >= 0 && y < map.height && dataJalan[y][x] == '1') {
					//kurir_rorr = MoveKurir(kurir_rorr, source, 1.0f);
					//kurir_rorr.x++;
				//}else{


					// if ((dataJalan[y][x + 25] == '1' || dataJalan[y][x - 25] == '0') && (arah_sebelumnya.x != -1  && distance_x > 0)){
					// 	kurir_rorr.x++;
					// 	arah_sebelumnya.x = 1;
					// }else if ((dataJalan[y - 25][x] == '1' || dataJalan[y + 25][x] == '0') && (arah_sebelumnya.y != 1 && distance_y < 0)){
					// 	kurir_rorr.y--;
					// 	arah_sebelumnya.y = -1;
					// }else if ((dataJalan[y + 25][x] == '1' || dataJalan[y - 25][x] == '0') && (arah_sebelumnya.y != -1 && distance_y > 0)) {  // cek bawah
					// 	kurir_rorr.y++;
					// 	arah_sebelumnya.y = 1;
					// }else if ((dataJalan[y][x - 25] ==  '1' || dataJalan[y][x + 25] == '0') && (arah_sebelumnya.x != 1 && distance_x < 0)){
					// 	kurir_rorr.x--;
					// 	arah_sebelumnya.x = -1;
					// }
				//}
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

			if (kurir_rorr.x != -1 && kurir_rorr.y != -1)
			{
				outline = PosisiValid(map, kurir, kurir_rorr);
				//DrawTexture(kurir_texture, kurir_rorr.x - outline.x + OFFSET_X, kurir_rorr.y - outline.y + OFFSET_Y, WHITE);

				// Rotasi
				Rectangle destRecKurir = {kurir_rorr.x - outline.x + OFFSET_X, kurir_rorr.y - outline.y + OFFSET_Y, (float)kurir.width, (float)kurir.height};
				DrawTexturePro(kurir_texture, sourceRecKurir, destRecKurir, origin, rotation, WHITE);

				outline = PosisiValid(map, kurir, source);
				DrawRectangle(source.x + OFFSET_X - outline.x, source.y + OFFSET_Y - outline.y, 20, 20, YELLOW);

				outline = PosisiValid(map, kurir, destination);
				DrawRectangle(destination.x + OFFSET_X - outline.x, destination.y + OFFSET_Y - outline.y, 20, 20, RED);
				
			}
			
			//DrawGrid__(screenWidth, screenHeight);

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

	for (int y = 0; y < map.height; y++) {
		free(dataJalan[y]);
	}
	free(dataJalan);	

	// destroy the window and cleanup the OpenGL context
	CloseWindow();
	return 0;
}
