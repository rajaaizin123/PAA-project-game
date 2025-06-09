#define RAYGUI_IMPLEMENTATION
#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION

#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raymath.h>
#include <stdint.h>
#include <float.h>

#include "../include/resource_dir.h" // utility header for SearchAndSetResourceDir
#include "gui_window_file_dialog.h"

#undef RAYGUI_IMPLEMENTATION
#define MAKS_KURIR 25
#define OFFSET_X 50
#define OFFSET_Y 70
#define GRID 25 // ini sekaligus lebar aspal (defaultnya)
#define OFFSET_KURIR 12

typedef enum
{
	MENU_STATE,
	GAME_STATE
} GameState;

typedef struct Vector_16
{
	uint16_t x;
	uint16_t y;
	uint16_t step;
} Vector16;

typedef struct
{
	int x[5];
	int y[5];
	int head;
} PosMemo;

typedef struct
{
	int x, y;
} Point;

bool tahapDua = false;
bool posisiSudahDiacak = false;
bool kurirSampai = false;
// Function to check if a pixel is asphalt
bool isWarnaAspal(Color pixel)
{
	return (pixel.r >= 90 && pixel.r <= 150) &&
		   (pixel.g >= 90 && pixel.g <= 150) &&
		   (pixel.b >= 90 && pixel.b <= 150);
}

// Load map into a 2D array
unsigned char **LoadMapKeArray(Image map)
{
	if (map.data == NULL)
	{
		printf("Error: Map data is NULL in LoadMapKeArray\n");
		return NULL;
	}
	unsigned char **jalanMap = (unsigned char **)malloc(map.height * sizeof(unsigned char *));
	if (!jalanMap)
	{
		printf("Error: Failed to allocate memory for jalanMap\n");
		return NULL;
	}
	for (int y = 0; y < map.height; y++)
	{
		jalanMap[y] = (unsigned char *)malloc(map.width * sizeof(unsigned char));
		if (!jalanMap[y])
		{
			printf("Error: Failed to allocate memory for jalanMap[%d]\n", y);
			for (int i = 0; i < y; i++)
				free(jalanMap[i]);
			free(jalanMap);
			return NULL;
		}
		for (int x = 0; x < map.width; x++)
		{
			Color pixel = GetImageColor(map, x, y);
			jalanMap[y][x] = isWarnaAspal(pixel) ? '1' : '0';
		}
	}
	return jalanMap;
}

// Randomize a valid position on the map
Vector2 RandomizePosisi(Image map)
{
	if (map.data == NULL)
	{
		printf("Error: Peta belum dimuat di RandomizePosisi!\n");
		return (Vector2){-1, -1};
	}

	int max_attempts = 100;
	for (int i = 0; i < max_attempts; i++)
	{
		int rx = GetRandomValue(0, map.width - 1);
		int ry = GetRandomValue(0, map.height - 1);
		if (rx >= 0 && ry >= 0 && rx < map.width && ry < map.height && isWarnaAspal(GetImageColor(map, rx, ry)))
		{
			printf("Valid position found: (%d, %d)\n", rx, ry);
			return (Vector2){(float)rx, (float)ry};
		}
	}
	printf("Warning: Tidak dapat menemukan posisi valid setelah %d percobaan\n", max_attempts);
	return (Vector2){-1, -1};
}


void ResetPosisi(Vector2 *kurir, Vector2 *src, Vector2 *dst, int *pathLen, int *step, bool *posisiFlag)
{
	*kurir = (Vector2){-1, -1};
	*src = (Vector2){-1, -1};
	*dst = (Vector2){-1, -1};
	*pathLen = 0;
	*step = 0;
	*posisiFlag = false;
}

// Validate position to ensure it fits within the map
Vector2 PosisiValid(Image map, Image ukuran, Vector2 posisi_awal)
{
	if (map.data == NULL || ukuran.data == NULL)
	{
		printf("Error: Map or ukuran data is NULL in PosisiValid\n");
		return (Vector2){0, 0};
	}
	Color pixel_patokan_kanan;
	int iterasi_pixel = ukuran.width;
	int pixel_outline_x = 0;
	for (int i = 1; i <= ukuran.width; i++)
	{
		int check_x = (int)posisi_awal.x + iterasi_pixel;
		if (check_x >= map.width || check_x < 0)
			break;
		pixel_patokan_kanan = GetImageColor(map, check_x, (int)posisi_awal.y);
		if (isWarnaAspal(pixel_patokan_kanan))
		{
			break;
		}
		iterasi_pixel--;
		pixel_outline_x = i;
	}

	Color pixel_patokan_bawah;
	iterasi_pixel = ukuran.height;
	int pixel_outline_y = 0;
	for (int i = 1; i <= ukuran.height; i++)
	{
		int check_y = (int)posisi_awal.y + iterasi_pixel;
		if (check_y >= map.height || check_y < 0)
			break;
		pixel_patokan_bawah = GetImageColor(map, (int)posisi_awal.x, check_y);
		if (isWarnaAspal(pixel_patokan_bawah))
		{
			break;
		}
		iterasi_pixel--;
		pixel_outline_y = i;
	}
	return (Vector2){(float)pixel_outline_x, (float)pixel_outline_y};
}

// Initialize position memo
void InitMemo(PosMemo *pos_memo)
{
	for (int i = 0; i < 5; i++)
	{
		pos_memo->x[i] = -1;
		pos_memo->y[i] = -1;
	}
	pos_memo->head = 0;
}

// Check if a position has been visited
bool sudahDikunjungi(PosMemo *pos_memo, int x, int y)
{
	for (int i = 0; i < 5; i++)
	{
		if (pos_memo->x[i] == x && pos_memo->y[i] == y)
		{
			return true;
		}
	}
	return false;
}

// Add position to memo
void AddToMemo(PosMemo *pos_memo, int x, int y)
{
	pos_memo->x[pos_memo->head] = x;
	pos_memo->y[pos_memo->head] = y;
	pos_memo->head = (pos_memo->head + 1) % 5;
}

// Fungsi heuristik untuk menghitung jarak Manhattan
int heuristic(int x1, int y1, int x2, int y2)
{
	return abs(x1 - x2) + abs(y1 - y2);
}

// Fungsi A* Pathfinding
int AStar(Point start, Point goal, unsigned char **dataJalan, int width, int height, Point *path, int maxPathLen)
{
	if (dataJalan == NULL)
	{
		printf("Error: dataJalan is NULL in AStar\n");
		return 0;
	}
	if (start.x < 0 || start.y < 0 || goal.x < 0 || goal.y < 0 ||
		start.x >= width || start.y >= height || goal.x >= width || goal.y >= height)
	{
		printf("Error: Invalid start or goal coordinates in AStar\n");
		return 0;
	}

	typedef struct
	{
		int x, y;
		int g, h, f;
		int parentX, parentY;
	} Node;

	// Alokasi dinamis
	Node **nodes = malloc(height * sizeof(Node *));
	bool **closed = malloc(height * sizeof(bool *));
	bool **open = malloc(height * sizeof(bool *));
	for (int y = 0; y < height; y++)
	{
		nodes[y] = malloc(width * sizeof(Node));
		closed[y] = calloc(width, sizeof(bool)); // Inisialisasi ke false
		open[y] = calloc(width, sizeof(bool));
		for (int x = 0; x < width; x++)
		{
			nodes[y][x] = (Node){x, y, INT_MAX, 0, INT_MAX, -1, -1};
		}
	}

	int (*openList)[2] = malloc(width * height * sizeof(int[2]));
	int openCount = 0;

	nodes[start.y][start.x].g = 0;
	nodes[start.y][start.x].h = heuristic(start.x, start.y, goal.x, goal.y);
	nodes[start.y][start.x].f = nodes[start.y][start.x].h;
	openList[openCount][0] = start.x;
	openList[openCount][1] = start.y;
	open[start.y][start.x] = true;
	openCount++;

	int dx[] = {1, -1, 0, 0, 1, -1, 1, -1};
	int dy[] = {0, 0, 1, -1, 1, 1, -1, -1};

	while (openCount > 0)
	{
		int minF = INT_MAX;
		int minIdx = 0;
		for (int i = 0; i < openCount; i++)
		{
			int x = openList[i][0];
			int y = openList[i][1];
			if (nodes[y][x].f < minF)
			{
				minF = nodes[y][x].f;
				minIdx = i;
			}
		}

		int cx = openList[minIdx][0];
		int cy = openList[minIdx][1];

		// Remove dari open list
		open[cy][cx] = false;
		openList[minIdx][0] = openList[openCount - 1][0];
		openList[minIdx][1] = openList[openCount - 1][1];
		openCount--;

		closed[cy][cx] = true;

		if (cx == goal.x && cy == goal.y)
		{
			// Rekonstruksi path
			int pathLen = 0;
			int x = cx, y = cy;
			while (x != -1 && y != -1 && pathLen < maxPathLen)
			{
				path[pathLen].x = x;
				path[pathLen].y = y;
				int px = nodes[y][x].parentX;
				int py = nodes[y][x].parentY;
				x = px;
				y = py;
				pathLen++;
			}

			// Reverse path
			for (int i = 0; i < pathLen / 2; i++)
			{
				Point temp = path[i];
				path[i] = path[pathLen - 1 - i];
				path[pathLen - 1 - i] = temp;
			}

			// Dealokasi
			for (int i = 0; i < height; i++)
			{
				free(nodes[i]);
				free(closed[i]);
				free(open[i]);
			}

			free(nodes);
			free(closed);
			free(open);
			free(openList);

			return pathLen;
		}

		for (int i = 0; i < 8; i++)
		{
			int nx = cx + dx[i];
			int ny = cy + dy[i];

			if (nx < 0 || ny < 0 || nx >= width || ny >= height || closed[ny][nx] || dataJalan[ny][nx] == '0')
				continue;

			int g = nodes[cy][cx].g + ((i < 4) ? 10 : 14);
			if (g < nodes[ny][nx].g)
			{
				nodes[ny][nx].g = g;
				nodes[ny][nx].h = heuristic(nx, ny, goal.x, goal.y);
				nodes[ny][nx].f = g + nodes[ny][nx].h;
				nodes[ny][nx].parentX = cx;
				nodes[ny][nx].parentY = cy;

				if (!open[ny][nx])
				{
					openList[openCount][0] = nx;
					openList[openCount][1] = ny;
					open[ny][nx] = true;
					openCount++;
				}
			}
		}
	}

	// Tidak ditemukan path, bersihkan memori
	for (int i = 0; i < height; i++)
	{
		free(nodes[i]);
		free(closed[i]);
		free(open[i]);
	}
	free(nodes);
	free(closed);
	free(open);
	free(openList);

	printf("AStar: No path found\n");
	return 0;
}

// Fixed naifMove function
Vector2 naifMove(int *memo_arah)
{
	Vector2 arah[8] = {
		{1, 0},	 // kanan
		{-1, 0}, // kiri
		{0, 1},	 // bawah
		{0, -1}, // atas
		{1, 1},	 // kanan bawah
		{-1, 1}, // kiri bawah
		{1, -1}, // kanan atas
		{-1, -1} // kiri atas
	};

	if (*memo_arah >= 1 && *memo_arah <= 8)
	{
		return arah[*memo_arah - 1];
	}
	return (Vector2){0, 0}; // Default if no valid direction
}

// Revised GerakKurir using A* path
Vector2 GerakKurir(Vector2 target, Vector2 current, unsigned char **dataJalan,
				   int width, int height, int *memo_arah, int speed,
				   PosMemo *pos_memo, int *rotasi, Point *path, int *pathLen, int *currentStep)
{
	if (dataJalan == NULL)
	{
		printf("Error: dataJalan is NULL in GerakKurir\n");
		return current;
	}
	int x = (int)current.x;
	int y = (int)current.y;
	int tx = (int)target.x;
	int ty = (int)target.y;

	if (x < 0 || y < 0 || tx < 0 || ty < 0)
	{
		printf("Error: Invalid coordinates in GerakKurir (current: %d,%d, target: %d,%d)\n", x, y, tx, ty);
		return current;
	}

	if (*currentStep >= *pathLen)
	{
		// Recalculate path if needed
		Point start = {x, y};
		Point goal = {tx, ty};
		*pathLen = AStar(start, goal, dataJalan, width, height, path, 1000);
		*currentStep = 0;
		if (*pathLen == 0)
		{
			printf("Error: Failed to recalculate path in GerakKurir\n");
			return current;
		}
	}

	if (*currentStep < *pathLen)
	{
		Vector2 stepTarget = {(float)path[*currentStep].x, (float)path[*currentStep].y};
		Vector2 diff = Vector2Subtract(stepTarget, current);
		float dist = Vector2Length(diff);

		if (dist < speed)
		{
			current = stepTarget;
			(*currentStep)++;
			AddToMemo(pos_memo, (int)current.x, (int)current.y);
		}
		else
		{
			Vector2 dir = Vector2Scale(Vector2Normalize(diff), speed);
			current = Vector2Add(current, dir);
		}

		// Update rotation
		if (Vector2Length(diff) > 0)
		{
			*rotasi = (int)(atan2f(diff.y, diff.x) * 57.2957795f); // RAD2DEG
			// Update memo_arah based on direction
			if (fabs(diff.x) > fabs(diff.y))
			{
				*memo_arah = (diff.x > 0) ? 1 : 2; // Kanan or Kiri
			}
			else
			{
				*memo_arah = (diff.y > 0) ? 3 : 4; // Bawah or Atas
			}
		}
	}

	return current;
}

int main()
{
	const int screenWidth = 1100;
	const int screenHeight = 700;
	InitWindow(screenWidth, screenHeight, "Smart Kurir");
	SetConfigFlags(FLAG_VSYNC_HINT | FLAG_WINDOW_HIGHDPI);
	SetTargetFPS(60);

	GameState currentScreen = MENU_STATE;
	bool startgame = false;
	bool mapLoaded = false;

	GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());
	char namaFileGambar[512] = {0};
	Texture map_texture = {0};
	Image map = {0};
	unsigned char **dataJalan = NULL;

	Vector2 kurir_rorr = {-1, -1};
	Vector2 source = {-1, -1};
	Vector2 destination = {-1, -1};
	Vector2 outline = {-1, -1};
	int memo_arah = 0;
	PosMemo pos_memo;
	InitMemo(&pos_memo);
	int rotation = 0;
	float speed = 1.0f;

	Point path[1000];
	int pathLen = 0;
	int currentStep = 0;

	Image kurir = LoadImage("resources/kurir_new2.png");
	if (kurir.data == NULL)
	{
		printf("Error: Failed to load kurir_new2.png\n");
		CloseWindow();
		return 1;
	}
	Texture2D kurir_texture = LoadTextureFromImage(kurir);
	Rectangle sourceRecKurir = {0.f, 0.f, (float)kurir.width, (float)kurir.height};
	Vector2 origin = {(float)kurir.width / 2.0f, (float)kurir.height / 2.0f};

	SearchAndSetResourceDir("resources");

	while (!WindowShouldClose())
	{
		BeginDrawing();
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
				if (IsFileExtension(fileDialogState.fileNameText, ".png"))
				{
					strcpy(namaFileGambar, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));
					Image newMap = LoadImage(namaFileGambar);
					if (newMap.data != NULL)
					{
						if (dataJalan)
						{
							for (int y = 0; y < map.height; y++)
								free(dataJalan[y]);
							free(dataJalan);
							if (map.data)
								UnloadImage(map);
						}
						map = newMap;
						UnloadTexture(map_texture);
						map_texture = LoadTextureFromImage(map);
						dataJalan = LoadMapKeArray(map);
						if (dataJalan == NULL)
						{
							printf("Error: Failed to load map array\n");
							mapLoaded = false;
						}
						else
						{
							mapLoaded = true;
							printf("Map loaded: %s (%dx%d)\n", namaFileGambar, map.width, map.height);
						}
					}
					else
					{
						printf("Gagal memuat gambar: %s\n", namaFileGambar);
						mapLoaded = false;
					}
				}
				fileDialogState.SelectFilePressed = false;
			}

			DrawTexture(map_texture, OFFSET_X, OFFSET_Y, WHITE);
			
			if (fileDialogState.windowActive)
				GuiLock();

			bool button_file = GuiButton((Rectangle){20, 20, 140, 30}, GuiIconText(ICON_FILE_OPEN, "Pilih Map"));
			if (button_file)
				fileDialogState.windowActive = true;

			bool button_start = GuiButton((Rectangle){170, 20, 140, 30}, "Start Game");
			bool button_stop = GuiButton((Rectangle){320, 20, 140, 30}, "Stop Game");
			bool button_random = GuiButton((Rectangle){470, 20, 140, 30}, "Random Mize");
			bool button_reset = GuiButton((Rectangle){620, 20, 140, 30}, "Reset Posisi");

			if (button_start && !startgame && mapLoaded && dataJalan)
			{
				printf("Start Game button pressed\n");
				startgame = true;

				if (!posisiSudahDiacak)
				{
					kurir_rorr = RandomizePosisi(map);
					source = RandomizePosisi(map);
					destination = RandomizePosisi(map);
				}

				if (kurir_rorr.x == -1 || source.x == -1 || destination.x == -1)
				{
					printf("Error: Posisi tidak valid\n");
					startgame = false;
				}
				else
				{
					Point start = {(int)kurir_rorr.x, (int)kurir_rorr.y};
					Point goal = {(int)source.x, (int)source.y};
					pathLen = AStar(start, goal, dataJalan, map.width, map.height, path, 1000);
					currentStep = 0;

					if (pathLen == 0)
					{
						printf("Error: Tidak ada jalur ditemukan\n");
						startgame = false;
					}
					else
					{
						posisiSudahDiacak = false; 
					}
				}
			}

			if (button_stop)
			{
				printf("Stop Game button pressed\n");
				startgame = false;
			}

			if (button_random && mapLoaded && dataJalan)
			{
				printf("Random Mize button pressed\n");
				kurir_rorr = RandomizePosisi(map);
				source = RandomizePosisi(map);
				destination = RandomizePosisi(map);
				posisiSudahDiacak = true;
				startgame = false;
				pathLen = 0;
				currentStep = 0;
			}

			// Garis Line
			// if (startgame && pathLen > 1)
			// {
			// 	for (int i = 0; i < pathLen - 1; i++)
			// 	{
			// 		Vector2 a = {(float)path[i].x + 0.5f + OFFSET_X, (float)path[i].y + 0.5f + OFFSET_Y};
			// 		Vector2 b = {(float)path[i + 1].x + 0.5f + OFFSET_X, (float)path[i + 1].y + 0.5f + OFFSET_Y};
			// 		DrawLineEx(a, b, 2.0f, tahapDua ? ORANGE : SKYBLUE);
			// 	}
			// }

			if (startgame && mapLoaded && dataJalan)
			{
				if (currentStep < pathLen)
				{
					Vector2 midTarget = {(float)path[currentStep].x + 0.5f, (float)path[currentStep].y + 0.5f};
					kurir_rorr = GerakKurir(
						midTarget,
						kurir_rorr,
						dataJalan,
						map.width, map.height,
						&memo_arah,
						1,
						&pos_memo,
						&rotation,
						path,
						&pathLen,
						&currentStep);
				}
				else if (!tahapDua)
				{
					Point start2 = {(int)kurir_rorr.x, (int)kurir_rorr.y};
					Point goal2 = {(int)destination.x, (int)destination.y};
					pathLen = AStar(start2, goal2, dataJalan, map.width, map.height, path, 1000);
					currentStep = 0;
					tahapDua = true;
					source = (Vector2){-1, -1};
				}
				else
				{
					kurirSampai = true;
					startgame = false;
				}
			}

			if (kurirSampai)
			{
				DrawText("Kurir telah sampai!", 20, 60, 20, RED);
			}

			// Ukuran titik source dan destination menyesuaikan grid kecil
			if (source.x != -1 && source.y != -1)
			{
				outline = PosisiValid(map, kurir, source);
				DrawRectangle((int)(source.x + OFFSET_X - outline.x + 0.5f), (int)(source.y + OFFSET_Y - outline.y + 0.5f), 10, 10, YELLOW);
			}

			if (destination.x != -1 && destination.y != -1)
			{
				outline = PosisiValid(map, kurir, destination);
				DrawRectangle((int)(destination.x + OFFSET_X - outline.x + 0.5f), (int)(destination.y + OFFSET_Y - outline.y + 0.5f), 10, 10, RED);
			}

			if (kurir_rorr.x != -1 && kurir_rorr.y != -1)
			{
				float scale = 1.0f; // bisa diubah ke 0.5 jika kurir terlalu besar
				Rectangle destRecKurir = {
					kurir_rorr.x + OFFSET_X,
					kurir_rorr.y + OFFSET_Y,
					kurir.width * scale,
					kurir.height * scale};
				Vector2 originKurir = {
					(kurir.width * scale) / 2.0f,
					(kurir.height * scale) / 2.0f};
				DrawTexturePro(kurir_texture, sourceRecKurir, destRecKurir, originKurir, (float)rotation, WHITE);

				// outline = PosisiValid(map, kurir, kurir_rorr);
				// Rectangle destRecKurir = {kurir_rorr.x - outline.x + OFFSET_X, kurir_rorr.y - outline.y + OFFSET_Y, (float)kurir.width, (float)kurir.height};
				// DrawTexturePro(kurir_texture, sourceRecKurir, destRecKurir, origin, (float)rotation, WHITE);

				// if (source.x != -1 && source.y != -1)
				// {
				// 	outline = PosisiValid(map, kurir, source);
				// 	DrawRectangle(source.x + OFFSET_X - outline.x, source.y + OFFSET_Y - outline.y, 20, 20, YELLOW);
				// }

				// if (destination.x != -1 && destination.y != -1)
				// {
				// 	outline = PosisiValid(map, kurir, destination);
				// 	DrawRectangle(destination.x + OFFSET_X - outline.x, destination.y + OFFSET_Y - outline.y, 20, 20, RED);
				// }
				// Ukuran titik source dan destination menyesuaikan grid kecil
				// if (source.x != -1 && source.y != -1)
				// {
				// 	outline = PosisiValid(map, kurir, source);
				// 	DrawRectangle((int)(source.x + OFFSET_X - outline.x + 0.5f), (int)(source.y + OFFSET_Y - outline.y + 0.5f), 10, 10, YELLOW);
				// }

				// if (destination.x != -1 && destination.y != -1)
				// {
				// 	outline = PosisiValid(map, kurir, destination);
				// 	DrawRectangle((int)(destination.x + OFFSET_X - outline.x + 0.5f), (int)(destination.y + OFFSET_Y - outline.y + 0.5f), 10, 10, RED);
				// }
			}
			if (button_reset)
			{
				printf("Reset Posisi button pressed\n");
				ResetPosisi(&kurir_rorr, &source, &destination, &pathLen, &currentStep, &posisiSudahDiacak);
				startgame = false;
			}

			GuiUnlock();
			GuiWindowFileDialog(&fileDialogState);
		}

		EndDrawing();
	}

	// Cleanup
	UnloadTexture(map_texture);
	UnloadTexture(kurir_texture);
	UnloadImage(kurir);
	if (dataJalan && map.data)
	{
		for (int y = 0; y < map.height; y++)
			free(dataJalan[y]);
		free(dataJalan);
	}
	if (map.data)
		UnloadImage(map);
	CloseWindow();
	return 0;
}