#ifndef CHIP8_H
#define CHIP8_H
#include "SDL.h"

typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
} sdl_t;

typedef struct
{
    int window_height;
    int window_width;
    int fg_color; // foregroud 0xRRGGBBAA
    int bg_color; // background 0xRRGGBBAA
    uint32_t scale_factor;
} config_t;

typedef enum
{
    QUIT,
    RUNNING,
    PAUSED,
} emulator_state_t;
// CHIP8 instruction format
typedef struct
{
    uint16_t opcode;
    uint16_t NNN; // 12 bit  Adress / constnat
    uint8_t NN;   // 8bit const
    uint8_t N;    // 4bit const
    uint8_t X;    // 4 bit register identifier(V0-VF)
    uint8_t Y;    // 4 bit register identifier(V0-VF)

    // inst.X, inst.NNN;
} instruction_t;

typedef struct
{
    emulator_state_t state;
    uint8_t ram[0x1000];
    bool display[64 * 32]; // &ram[0XF00]
    uint16_t stack[12];    // call stack
    uint16_t *stack_ptr;   // Self explanatory
    uint8_t V[16];         // Data registers V0-VF. (F = flags)
    uint16_t I;            // Adress register, 12 bits wide (Index register?)
    uint16_t PC;           // Program Counter
    uint8_t delay_timer;   // Decrements at 60hz when >0
    uint8_t sound_timer;   // Decremets at 60hz when >0and will play a tone when >0
    bool keypad[16];       // Hexadecimal keypad 0x0-0xF;
    char *rom_name;        // Currently running ROM
    instruction_t inst;
} chip8_t;

#endif