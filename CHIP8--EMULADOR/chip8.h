#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t memory[4096];
	uint8_t V[16];
	uint16_t I;
	uint16_t pc;
	uint16_t stack[16];
	uint8_t sp;
	uint8_t gfx[64 * 32];
	bool draw_flag;
} Chip8;

void chip8_init(Chip8* chip8);
bool chip8_load_rom(Chip8 *chip8, const char *filename);
void chip8_cycle(Chip8* chip8);

void display_init(void);
void display_draw(const Chip8* chip8);

#endif