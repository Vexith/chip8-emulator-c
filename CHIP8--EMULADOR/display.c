#include "chip8.h"
#include <stdio.h>
#include <windows.h>

#define GFX (chip8->gfx)

static HANDLE hConsole;
void display_init(void) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	// oculta o cursor
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = FALSE;
	SetConsoleCursorInfo(hConsole, &cursorInfo);

	
}

static void reset_cursor(void) {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { 0,0 };
	SetConsoleCursorPosition(hConsole, pos);
}

void display_draw(const Chip8* chip8) {
	char buffer[2300];

	int ptr = 0;

	reset_cursor();


	ptr += sprintf(&buffer[ptr], "+----------------------------------------------------------------+");

	for (int y = 0; y < 32; y++) {

		buffer[ptr++] = '|';

		for (int x = 0; x < 64; x++) {

			buffer[ptr++] = GFX[x + (y * 64)] ? '#' : ' ';

		}

		buffer[ptr++] = ' | ';

		buffer[ptr++] = '\n';

	}

	ptr += sprintf(&buffer[ptr], "+----------------------------------------------------------------+");

	buffer[ptr] = '\0';

	printf("%s", buffer);
	fflush(stdout);


}