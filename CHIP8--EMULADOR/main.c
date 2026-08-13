#pragma warning(disable : 6031)
#pragma warning(disable: 6319)
#pragma warning(disable: 4013)

#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "main.h"




uint8_t memory[4096]; // 4kb
uint16_t pc = 0x200; // inicio da rom
uint8_t V[16]; // registrators of general purpose (V0 => VF)
uint16_t I; // indice memory registry

bool debug = false;
// screen (64 x 32 pixels)
uint8_t gfx[64 * 32];
bool draw_flag = false;

// caracteres de 0 a F
const uint8_t fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
};

void reset_cursor() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	COORD pos = { 0,0 };
	SetConsoleCursorPosition(hConsole, pos);
}
void draw_graphics() {
	reset_cursor();

	printf("+----------------------------------------------------------------+\n");
	for (int y = 0; y < 32; y++) {
		printf("|");
		for (int x = 0; x < 64; x++) {
			if (gfx[x + (y * 64)] != 0) {
				printf("\xDB"); // on
			}
			else {
				printf(" "); // off
			}
		}
		printf("|\n");
	}
	printf("+----------------------------------------------------------------+\n");
	fflush(stdout);
}
void chip8_cycle() { // simula 1 pulso da cpu
	uint16_t opcode = (memory[pc] << 8 | memory[pc + 1]); // junta 2 bytes de memória em um opcode de 16 bits
	pc += 2;
	
	//printf("opcode read on pc (0x%03X): 0x%04X\n", pc - 2, opcode);

	switch (opcode & 0xF000) {
    
		

	case 0x0000: {
		if ((opcode & 0x00FF) == 0xE0) {
			// opcode 00E00: cleans the screen
			memset(gfx, 0, sizeof(gfx));
			draw_flag = true;
		}
		break;
	}
	case 0x1000: {
		// Opcode 1NNN: JMP to execution direct to addresss NNN
		uint16_t nnn = opcode & 0x0FFF;
		pc = nnn;
		if (debug != false) {
			printf("instruction: JMP to address 0x%03X\n", nnn);
		}
		break;
	}

	case 0x6000: {
		// Opcode 6XNN: defines the registrator VX with the value NN
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t nn = opcode & 0x00FF;
		V[x] = nn;
		if (debug != false) {
			printf("instruction: V[%X] = %d (0x%02X)\n", x, nn, nn);
		}
		break;
	}
	
	case 0x7000: {
		// Opcode 7XNN: sums NN to the registrator V[X]
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t nn = opcode & 0x00FF;
		V[x] += nn;
		if (debug != false) {
			printf("instruction: V[%X] += %d (new value: %d)\n", x, nn, V[x]);
		}
		
		break;
	}

	case 0xA000: {
		// Opcode ANNN: defines the registrator I with the address NNN
		uint16_t nnn = opcode & 0x0FFF;
		I = nnn;
		if (debug != false) {
			printf("instruction: I = 0x%03X\n", nnn);
		}
		
		break;
	}

	case 0xD000: {
		// opcode DYXN: draws sprites on the screen on v[x] and v[y]
		uint8_t x = (opcode & 0x0F00) >> 8;
		uint8_t y = (opcode & 0x00F0) >> 4;
		uint8_t height = opcode & 0x000F;

		uint8_t x_pos = V[x] % 64;
		uint8_t y_pos = V[y] % 32;

		V[0xF] = 0; // colision register

		for (int row = 0; row < height; row++) {
			uint8_t sprite_byte = memory[I + row];

			for (int col = 0; col < 8; col++) {
				//verifys bit for bit if the sprite is present
				if ((sprite_byte & (0x80 >> col)) != 0) {
					int target_x = (x_pos + col) % 64;
					int target_y = (y_pos + row) % 32;
					int index = target_x + (target_y * 64);

					if (gfx[index] == 1) {
						V[0xF] = 1; // colision detected
					}

					gfx[index] ^= 1; // alternates pixel (XOR)
				}
			}
		}
		draw_flag = true;
		break;
	}

	default:
		if (debug != false) {
			printf("Opcode not yet implemented\n");
		}
		
		break;
	}

	fflush(stdout);
}
bool load_rom(const char* filename) {
	FILE* file = fopen(filename, "rb"); // le em binario
	if (!file) {
		printf("error: file not found %s\n", filename);
		return false;
	}

	// ve o tamanho
	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	// ve se a rom nao é grande demais
	if (size > (4096 - 512)){
		printf("error: rom too big!\n");
		fclose(file);
		return false;
	}

	// le a rom para a ram diretamente
	fread(&memory[0x200], size, 1, file);
	fclose(file);

	printf("rom '%s' loaded sucessfully (%ld bytes)!\n", filename, size);
	return true;
}


int main() {
	memset(memory, 0, sizeof(memory)); //zerar memoria
	memset(V, 0, sizeof(V));
	memset(gfx, 0, sizeof(gfx));
	I = 0;

	printf("allocated ram: %zu bytes.\n", sizeof(memory));
	printf("initialized pc in: 0x%03X\n", pc);

	for (int i = 0; i < 80; i++) { // primeiros 81 endereços carregados para fonte
		memory[i] = fontset[i];
	}
	printf("Font loaded! first byte of ram: 0x%02X\n", memory[0]);
	char rom_name[100];
	printf("bote o nome da rom ai meu consagrado: ");

	if (scanf("%s", rom_name) != 1) {
		printf("fatal error on reading the archive name");
		return 1;
	}

	if (load_rom(rom_name)) {
		//printf("initial byte on 0x200: 0x%02X\n", memory[0x200]);
		printf("running the first 100 cycles\n");
		for (int i = 0; i < 100; i++) {
			chip8_cycle();

			if (draw_flag) {
				draw_graphics();
				draw_flag = false;
			}
			if (i == 99) {
				i = 0;
			}
		}
		fflush(stdout);
		printf("press ENTER to exit...\n");
		(void)getchar();
		(void)getchar();

		return 0;
	}

	return 0;
}