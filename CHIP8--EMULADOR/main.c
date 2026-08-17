#pragma warning(disable : 6031)
#pragma warning(disable: 6319)
#pragma warning(disable: 4013)

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "main.h"
#include "chip8.h"



int main() {
	Chip8 chip8;
	chip8_init(&chip8);
	display_init();

	char rom_name[100];
	printf("type the name of the rom here: ");
	if ((scanf("%99s", rom_name) != 1)) goto cleanup;
	if (!load_rom(&chip8, rom_name)) goto cleanup;

	chip8.draw_flag = true;
	bool running = true;
	while (running) {
		for (int i = 0; i < 3; i++) {
			chip8_cycle(&chip8);
		}
		

		if (chip8.draw_flag) {
			display_draw(&chip8);
			chip8.draw_flag = false;
			
		}
		Sleep(16);
	}

	if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
		running = false;
	}
	
cleanup:
	printf("press ENTER to exit...\n");

	fflush(stdout);

	(void)getchar();
	(void)getchar();
	return 0;
}