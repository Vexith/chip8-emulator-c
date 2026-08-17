
#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define V (chip8->V) // registradores gerais V0 a VF (16 bytes)
#define PC (chip8->pc) // PC (program counter) actual instruction pointer
#define I (chip8->I) // registrador de indice de memoria
#define RAM (chip8->memory) // memory of 4kb (4096 bytes)
#define STACK (chip8->stack) // pilha de chamadas de subrotinas (16 níveis)
#define SP (chip8->sp) // stack pointer
#define GFX (chip8->gfx) // video buffer 64 x 32 pixels (2048 bytes)
#define DRAW (chip8->draw_flag) // sinal de atualização de tela


static const uint8_t fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_init(Chip8* chip8) {
    // zerar memorys
    memset(RAM, 0, sizeof(RAM));
    memset(V, 0, sizeof(V));
    memset(STACK, 0, sizeof(STACK));
    memset(GFX, 0, sizeof(GFX));
    PC = 0x200;
    I = 0;
    SP = 0;
    DRAW = false;
    // primeiros 81 endereços da fonte carregados pra memória
    for (int i = 0; i < 80; i++) {
        RAM[i] = fontset[i];
    }
}

bool load_rom(Chip8* chip8, const char* filename) {
    FILE* file = fopen(filename, "rb"); // le em binario
    if (!file) {
        printf("error: file not found %s\n", filename);
        return false;
    }
    
    // ve o tamanho
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    // ve se a rom nao é grande demais (tipo tua mae)
    if (size > (4096 - 512)) {
        printf("error: rom too big!\n");
        fclose(file);
        return false;
    }

    // le a rom para a ram diretamente
    fread(&RAM[0x200], size, 1, file);
    fclose(file);

    //printf("rom '%s' loaded sucessfully (%ld bytes)!\n", filename, size);
    return true;
}
void chip8_cycle(Chip8* chip8) {
    uint16_t opcode = (RAM[PC] << 8) | RAM[PC + 1]; // junta 2 bytes consecutivos da memória em um opcode de 16 bits (Big-Endian)
    PC += 2; // avança o program counter pra próxima instrução

    // extrai os parametros comuns dos nibbles intermediários
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    // decodifica o opcode pelo nibble mais significativo
    switch (opcode & 0xF000) {
    default:
        break;
    case 0x0000:
        if ((opcode & 0x00FF) == 0xE0) { // 00E0: limpa a tela
            memset(GFX, 0, sizeof(GFX));
            DRAW = true;
        }
        else if ((opcode & 0x00FF) == 0xEE) { // 00EE: retorna de uma subrotina
            SP--;
            PC = STACK[SP];
        }
        break;
    case 0x1000: // desvia a execução pro endereço NNN
        PC = opcode & 0x0FFF;
        break;
    
    case 0x2000: // chama a subrotina no endereço NNN
        STACK[SP] = PC;
        SP++;
        PC = opcode & 0x0FFF;
        break;

    case 0x3000: // pula pra proxima inst se v[x] = nn
        if (V[x] == (opcode & 0x00FF)) PC += 2;
        break;

    case 0x4000: // pula pra proxima inst se v[x] != nn
        if (V[x] != (opcode & 0x00FF)) PC += 2;
        break;

    case 0x6000: // define o valor de v[x] = n 
        V[x] = opcode & 0x00FF;
        break;
    case 0x7000: // adiciona nn ao registrador v[x]
        V[x] += (opcode & 0x00FF);
        break;

    case 0x8000: // operações aritméticas entre v[x] e v[y]
        switch (opcode & 0x000F) {
        case 0x0: V[x] = V[y]; break; // =
        case 0x1: V[x] |= V[y]; break; // OR
        case 0x2: V[x] &= V[y]; break; // AND
        case 0x3: V[x] ^= V[y]; break; // XOR

        case 0x4: {
            uint16_t sum = V[x] + V[y];
            V[0xF] = (sum > 255) ? 1 : 0;
            V[x] = sum & 0xFF;
            break;
        }
        
        case 0x5:
            V[0xF] = V[x] >= V[y] ? 1 : 0;
            V[x] -= V[y];
            break;
        case 0x6: // shift right (8XY6)
            V[0xF] = V[x] & 0x1;
            V[x] >>= 1;
            break;
        case 0x7: // auto explicativo
            V[0xF] = V[y] >= V[x] ? 1 : 0;
            V[x] = V[y] - V[x];
            break;
        case 0xE: // shift left (8XYE)
            V[0xF] = (V[x] & 0x80) >> 7;
            V[x] <<= 1;
            break;
        }
        break;

    case 0xA000: // auto explicativo
        I = opcode & 0x0FFF;
        break;

    case 0xD000: { // desenha o sprite na posição (x, y) com largura 8 e altura N
        uint8_t height = opcode & 0x000F;
        uint8_t x_pos = V[x] % 64;
        uint8_t y_pos = V[y] % 32;

        V[0xF] = 0; // reseta registrador de colisão

        for (int row = 0; row < height; row++) {
            if (y_pos + row >= 32) break; // corta o sprite no eixo y

            uint8_t sprite = RAM[I + row]; // le o byte na linha da memória

            for (int col = 0; col < 8; col++) {
                if (x_pos + row >= 64) break; // corta na borda direita

                if ((sprite & (0x80 >> col)) != 0) { // verifica se o bit do pixel atual no byte do sprite está ativo
                    int tx = (x_pos + col) % 64;
                    int ty = (y_pos + row) % 32;
                    int idx = tx + (ty * 64);
                    
                    if (GFX[idx] == 1) V[0xF] = 1; // se pixel já existia no buffer, registra colisão em v[f]

                    GFX[idx] ^= 1; // inverte o estado do pixel na tela (XOR)
                }
            }
            DRAW = true; // notifica o sistema de renderização sobre a alteração
        }
    }
    }

    
}