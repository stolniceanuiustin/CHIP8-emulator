#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include "SDL.h"
#include "chip8.h"

// Initializare
bool init_sdl(sdl_t *sdl, config_t config)
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
    {
        SDL_Log("SDL Subsystem not initialised! %s\n", SDL_GetError());
        return false;
    }
    sdl->window = SDL_CreateWindow("CHIP8 Emulator",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   config.window_width * config.scale_factor,
                                   config.window_height * config.scale_factor,
                                   0);

    if (!sdl->window)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    // RENDERER
    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if (!sdl->renderer)
    {
        printf("renderer could not be created! %s\n", SDL_GetError());
        return false;
    }
    return true;
}

// Iniitial emulator config from passed arguments
bool set_config_from_args(config_t *config, int argc, char **argv)
{

    // set default
    config->window_width = 64;  // CHIP8 X RESOLUTION
    config->window_height = 32; // Y
    config->fg_color = 0xFFFFFFFF;
    config->bg_color = 0x000000FF;
    config->scale_factor = 20;             // 1280x640
    config->instructions_per_second = 600; // standard speed
    config->current_extension = 0;         // CHIP8
    // override default from args
    for (int i = 1; i < argc; i++)
    {
        (void)argv[i];
    }

    return true;
}

// clear sdl window to background color
void clear_screen(sdl_t sdl, const config_t config)
{
    uint8_t r = (config.bg_color >> 24) & 0xFF;
    uint8_t g = (config.bg_color >> 16) & 0xFF;
    uint8_t b = (config.bg_color >> 8) & 0xFF;
    uint8_t a = (config.bg_color >> 0) & 0xFF;

    SDL_SetRenderDrawColor(sdl.renderer, r, g, b, a);
    SDL_RenderClear(sdl.renderer);
}

// update window with any changes
void update_screen(sdl_t sdl, config_t config, chip8_t chip8)
{
    SDL_Rect rect = {.x = 0,
                     .y = 0,
                     .w = config.scale_factor,
                     .h = config.scale_factor};
    // Get color
    uint8_t fg_r = (config.fg_color >> 24) & 0xFF;
    uint8_t fg_g = (config.fg_color >> 16) & 0xFF;
    uint8_t fg_b = (config.fg_color >> 8) & 0xFF;
    uint8_t fg_a = (config.fg_color >> 0) & 0xFF;

    uint8_t bg_r = (config.bg_color >> 24) & 0xFF;
    uint8_t bg_g = (config.bg_color >> 16) & 0xFF;
    uint8_t bg_b = (config.bg_color >> 8) & 0xFF;
    uint8_t bg_a = (config.bg_color >> 0) & 0xFF;

    // Unidimensional index = y * window_width + x
    for (uint32_t i = 0; i < sizeof chip8.display; i++)
    {
        // 1D i value => 2D X/Y
        // X = i % window_width
        // Y = i / window_width
        rect.x = (i % config.window_width) * config.scale_factor;
        rect.y = (i / config.window_width) * config.scale_factor;
        if (chip8.display[i]) // fg color
        {
            SDL_SetRenderDrawColor(sdl.renderer, fg_r, fg_g, fg_b, fg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);
        }

        else
        { // background color
            SDL_SetRenderDrawColor(sdl.renderer, bg_r, bg_g, bg_b, bg_a);
            SDL_RenderFillRect(sdl.renderer, &rect);
        }
    }
    SDL_RenderPresent(sdl.renderer);
}

// CHIP8 keypad to querty
/*
1 2 3 C    1 2 3 4
4 5 6 D    Q W E R
7 8 9 E    A S D F
A 0 B F    Z X C V
*/
void handle_input(chip8_t *chip8)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            chip8->state = QUIT;
            return;
        case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        // Escape key; Exit window & End program
                        chip8->state = QUIT;
                        break;
                        
                    case SDLK_SPACE:
                        // Space bar
                        if (chip8->state == RUNNING) {
                            chip8->state = PAUSED;  // Pause
                            puts("==== PAUSED ====");
                        } else {
                            chip8->state = RUNNING; // Resume
                        }
                        break;

                    // Map qwerty keys to CHIP8 keypad
                    case SDLK_1: chip8->keypad[0x1] = true; break;
                    case SDLK_2: chip8->keypad[0x2] = true; break;
                    case SDLK_3: chip8->keypad[0x3] = true; break;
                    case SDLK_4: chip8->keypad[0xC] = true; break;

                    case SDLK_q: chip8->keypad[0x4] = true; break;
                    case SDLK_w: chip8->keypad[0x5] = true; break;
                    case SDLK_e: chip8->keypad[0x6] = true; break;
                    case SDLK_r: chip8->keypad[0xD] = true; break;

                    case SDLK_a: chip8->keypad[0x7] = true; break;
                    case SDLK_s: chip8->keypad[0x8] = true; break;
                    case SDLK_d: chip8->keypad[0x9] = true; break;
                    case SDLK_f: chip8->keypad[0xE] = true; break;

                    case SDLK_z: chip8->keypad[0xA] = true; break;
                    case SDLK_x: chip8->keypad[0x0] = true; break;
                    case SDLK_c: chip8->keypad[0xB] = true; break;
                    case SDLK_v: chip8->keypad[0xF] = true; break;

                    default: break;
                        
                }
                break; 

       case SDL_KEYUP:
                switch (event.key.keysym.sym) {
                    // Map qwerty keys to CHIP8 keypad
                    case SDLK_1: chip8->keypad[0x1] = false; break;
                    case SDLK_2: chip8->keypad[0x2] = false; break;
                    case SDLK_3: chip8->keypad[0x3] = false; break;
                    case SDLK_4: chip8->keypad[0xC] = false; break;

                    case SDLK_q: chip8->keypad[0x4] = false; break;
                    case SDLK_w: chip8->keypad[0x5] = false; break;
                    case SDLK_e: chip8->keypad[0x6] = false; break;
                    case SDLK_r: chip8->keypad[0xD] = false; break;

                    case SDLK_a: chip8->keypad[0x7] = false; break;
                    case SDLK_s: chip8->keypad[0x8] = false; break;
                    case SDLK_d: chip8->keypad[0x9] = false; break;
                    case SDLK_f: chip8->keypad[0xE] = false; break;

                    case SDLK_z: chip8->keypad[0xA] = false; break;
                    case SDLK_x: chip8->keypad[0x0] = false; break;
                    case SDLK_c: chip8->keypad[0xB] = false; break;
                    case SDLK_v: chip8->keypad[0xF] = false; break;

                    default: break;
                }
                break;

        }
    }
}

bool init_chip8(chip8_t *chip8, char rom_name[])
{
    const uint32_t entry_point = 0x200; // Where programs start
    const uint8_t font[] = {
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
    // Load font
    memcpy(&chip8->ram[0], font, sizeof(font)); // FONT STARTS AT 0x0
    // Load ROM
    FILE *rom = fopen(rom_name, "rb");
    if (!rom)
    {
        printf("Could not open ROM FILE: %s\n", rom_name);
        return false;
    }

    // Get ROM SIZE
    fseek(rom, 0, SEEK_END);
    const size_t rom_size = ftell(rom); // Moment C
    const size_t max_size = sizeof chip8->ram - entry_point;
    rewind(rom);

    if (rom_size > max_size)
    {
        printf("Rom file %s is bigger than RAM !?. Max size: %zu, Rom size: %zu", rom_name, max_size, rom_size);
        return false;
    }

    if (fread(&chip8->ram[entry_point], rom_size, 1, rom) != 1)
    {
        printf("Could not read rom file into CHIP8 RAM\n");
        return false;
    };
    fclose(rom);
    // Set chip8 machine defaults
    chip8->state = RUNNING;  // Default state machine
    chip8->PC = entry_point; // Where programs start being loaded in RAM
    chip8->rom_name = rom_name;
    chip8->stack_ptr = &chip8->stack[0];

    return true;
}

// Cleanup
void final_cleanup(sdl_t sdl)
{
    SDL_DestroyRenderer(sdl.renderer);
    SDL_DestroyWindow(sdl.window);
    SDL_Quit();
    return;
}

#ifdef DEBUG

void print_debug_info(chip8_t *chip8)
{
    printf("Adress: 0x%04X, Opcode: 0x%04X Desc: ", chip8->PC - 2, chip8->inst.opcode);
    switch ((chip8->inst.opcode >> 12) & 0x0F) // First 4 bits of the opcode
    {

    case 0x0:                             // The instruction begins with 0
        if (chip8->inst.opcode == 0x00E0) // Clear screen
        {
            printf("Clear screen\n");
        }
        else if (chip8->inst.opcode == 0x00EE) // Return
        {
            printf("Return from subroutine to adress 0x%04X\n", *(chip8->stack_ptr - 1));
        }
        break;
    case 0x01:
        // goto NNN
        printf("Jump to adress %04X\n", chip8->inst.NNN);
        // chip8->PC = chip8->inst.NNN;
        break;
    case 0x02: // Calls subroutine at NNN
        printf("Calls subroutine at NNN(0x%04X)", chip8->inst.NNN);
        break;
    case 0x03:
        // 3XNN -> skips the next instruction if VX = NN
        printf("If V%X == NN (0x%02X == 0x%02X), skip next instruction\n", chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.NN);
        break;
    case 0x04:
        // 4XNN -> if(Vx != NN) skip the next instruiction
        printf("If V%X != NN (0x%02X == 0x%02X), skip next instruction\n", chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.NN);
        break;

    case 0x05:
        // 5XNN -> if VX == VY skip the next insturction
        printf("If V%X == V%X (0x%02X == 0x%02X), skip next instruction\n", chip8->inst.X, chip8->inst.Y, chip8->V[chip8->inst.X], chip8->V[chip8->inst.Y]);
        break;
    case 0x06:
        // 0x6XNN; V[X] <= NN
        printf("Set register V%X to NN(0x%02X)\n", chip8->inst.X, chip8->inst.NN);
        break;
    case 0x07:
        // 0x6XNN; V[X] <= NN
        printf("Set register V%X to V%X + NN(0x%02X), result: %02X\n", chip8->inst.X, chip8->inst.X, chip8->inst.NN, chip8->V[chip8->inst.X] + chip8->inst.NN);
        break;
    case 0x08: // Operatii aritmetice
        uint8_t X = chip8->inst.X;
        uint8_t Y = chip8->inst.Y;
        uint8_t carry = 0;
        switch (chip8->inst.N)
        {
        case 0:
            printf("V%X = V%X (V%X = 0x%02X )\n", X, Y, X, chip8->V[Y]);
            break;

        case 1:
            printf("V%X |= V%X (V%X = 0x%02X )\n", X, Y, X, chip8->V[X] | chip8->V[Y]);
            break;

        case 2:
            printf("V%X &= V%X (V%X = 0x%02X )\n", X, Y, X, chip8->V[X] & chip8->V[Y]);
            break;

        case 3:
            printf("V%X ^= V%X (V%X = 0x%02X )\n", X, Y, X, chip8->V[X] ^ chip8->V[Y]);
            break;

        case 4:
            uint16_t overflow_check = (uint16_t)(chip8->V[X] + chip8->V[Y]);
            if (overflow_check > 255)
                carry = 1;
            else
                carry = 0;
            printf("V%X += V%X (V%X = 0x%02X ), VF = %01X \n", X, Y, X, chip8->V[X] + chip8->V[Y], carry);
            break;

        case 5:
            // chip8->V[X] = chip8->V[X] - chip8->V[Y];
            if (chip8->V[X] >= chip8->V[Y])
                carry = 1;
            else
                carry = 0;
            printf("V%X -= V%X (V%X = 0x%02X, V%X = 0x%02X), result = 0x%02X, VF = %01X \n", X, Y, X, chip8->V[X], Y, chip8->V[Y], chip8->V[X] - chip8->V[Y], carry);
            break;

        case 6:
            carry = chip8->V[X] & 1;
            printf("V%X >>= 1 (V%X = 0x%02X ), VF = %01X \n", X, X, chip8->V[X] >> 1, carry);
            break;

        case 7:
            // chip8->V[X] = chip8->V[Y] - chip8->V[X];
            if (chip8->V[Y] >= chip8->V[X])
                carry = 1;
            else
                carry = 0;
            printf("V%X = V%X - V%X (V%X = 0x%02X ), VF = %01X \n", X, Y, X, X, chip8->V[Y] - chip8->V[X], carry);
            break;

        case 0x0E:
            carry = (chip8->V[X] & (1 << 7));
            printf("V%X << = 1 (V%X = 0x%02X ), VF = %01X \n", X, X, chip8->V[X] << 1, carry);
            // chip8->V[X] <<=1;
            // chip8->V[0x0F] = carry;
            break;
        }
        break;

    case 0x0E:                      // input handling
        if (chip8->inst.NN == 0x9E) // if(key() == VX) skip the next instruction
        {
            printf("If key at V%X(0x%02X) is pressed, skip the next instruction\n", chip8->inst.X, chip8->V[chip8->inst.X]);
            break;
        }
        else if (chip8->inst.NN == 0xA1)
        {
            printf("If key at V%X(0x%02X) is not pressed, skip the next instruction\n", chip8->inst.X, chip8->V[chip8->inst.X]);
            break;
        }
        break;

        break;
    case 0x09:
        printf("If V%X != V%X (0x%02X == 0x%02X), skip next instruction\n", chip8->inst.X, chip8->inst.Y, chip8->V[chip8->inst.X], chip8->V[chip8->inst.Y]);
        break;
    case 0x0A:
        // 0xANNN; I (index register) <= NNN
        printf("Index register 0x%04X <- 0x%04X\n", chip8->I, chip8->inst.NNN);
        break;
    case 0x0B:
        // 0xBNNN: jump to adress NNN + V[0]
        printf("Jump to adress NNN + V[0] (0x%04X)\n", chip8->inst.NNN + chip8->V[0]);
        break;
    case 0x0C:
        printf("Set V%X = rand() %% 256 & %02X (NN)", chip8->inst.X, chip8->inst.NN);
        break;
    case 0x0D:
        // 0xDXYN: draw N height sprite at coords V[X], V[Y]
        // Read from memory location I.
        // VF is set to 1 if any screen pixels are flipped from set to unset when the sprite is drawn, and to 0 if that does not happen
        printf("Draw  N(%u) height sprite at coords V%X(0x%02X), V%X(0x%02X) from I (0x%04X). VF = 1/0\n",
               chip8->inst.N, chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.Y, chip8->V[chip8->inst.Y], chip8->I);
        break;
    case 0x0F:
        switch (chip8->inst.NN)
        {
        case 0x0A:
            // 0x0FX0A : VX = get_key(); wait until a keypress then store it in VX
            printf("V%X = get_key(); wait until a keypress and store it in V%X\n", chip8->inst.X, chip8->inst.X);
            break;
        case 0x1E:
            // 0xFX1E: I += VX
            printf("I(%04X) += V%X: I = %04X\n", chip8->I, chip8->inst.X, chip8->I + chip8->V[chip8->inst.X]);
            break;
        case 0x07:
            // 0xFX07: VX = delay timer
            printf("V%X = %02X (delay timer)\n", chip8->inst.X, chip8->delay_timer);
            break;
        case 0x15:
            // 0xFX1: delay_timer = VX
            printf("%02X (delay timer) = V%X(%02X)\n", chip8->delay_timer, chip8->inst.X, chip8->V[chip8->inst.X]);
            break;
        case 0x18:
            // 0xF18 sound timer = VX
            printf("%02X (sound timer) = V%X(%02X)\n", chip8->sound_timer, chip8->inst.X, chip8->V[chip8->inst.X]);
            break;

        case 0x29:
            // 0xFX29 // i = sprite_adr[VX];
            printf("Set I to sprite location at V%X(0x%02X - 0-F)\n", chip8->inst.X, chip8->V[chip8->inst.X]);
            break;
        case 0x33:
            // stores the BCD representaiton of VX, I = hundrend's digit, I+1 = ten's digit, i+2 = one's digit
            printf("Stored BCD representation of VX at I for drawing i suppose\n");
            break;
        case 0x55:
            // 0xFX55: registry dump from 0 to VX at I
            printf("Register dump V0 - V%X at memory from I(0x%04X)\n", chip8->inst.X, chip8->I);
            break;
        case 0x65:
            // 0xF65: registry load V0 - VX from I
            printf("Register load V0 - V%X from memory I(0x%04X)\n", chip8->inst.X, chip8->I);
            break;
        default:
            printf("Unimplemented or invalid opcode\n");
            break;
        }

        break;
    default:
        printf("Unimplemented or invalid opcode\n");
        break;
    }
}
#endif

void emulate_instruction(chip8_t *chip8, config_t config)
{
    // Get next opcode from RAM
    chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | (chip8->ram[chip8->PC + 1]); // little endian -> big endian
    chip8->PC += 2;
    chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
    chip8->inst.NN = chip8->inst.opcode & 0x00FF;
    chip8->inst.N = chip8->inst.opcode & 0x000F;
    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;

#ifdef DEBUG
    print_debug_info(chip8);
#endif

    // Emulate opcode
    switch ((chip8->inst.opcode >> 12) & 0x0F) // First 4 bits of the opcode
    {
    case 0x00:
        if (chip8->inst.opcode == 0x00E0) // Clear screen
        {
            memset(&chip8->display[0], 0, 64 * 32);
            chip8->draw = true;
        }
        else if (chip8->inst.opcode == 0x00EE) // Return from subroutine
        {
            // Returns from a subroutine
            chip8->PC = *(--chip8->stack_ptr);
        }
        else
        {
            // printf("Unimplemeneted\n");
            int ttttt = 0;
            if (ttttt == 5)
                ttttt = 2;
        }
        break;

    case 0x01:
        // goto NNN
        chip8->PC = chip8->inst.NNN;
        break;
    case 0x02:                           // Calls subroutine at NNN
        *chip8->stack_ptr++ = chip8->PC; // Push return adress
        chip8->PC = chip8->inst.NNN;     // Change program counter
        break;
    case 0x03:
        // 3XNN -> skips the next instruction if VX = NN
        if (chip8->V[chip8->inst.X] == chip8->inst.NN)
            chip8->PC += 2;
        break;

    case 0x04:
        // 4XNN -> if(Vx != NN) skip the next instruiction
        if (chip8->V[chip8->inst.X] != chip8->inst.NN)
            chip8->PC += 2;
        break;

    case 0x05:
        // 5XNN -> if VX == VY skip the next insturction
        if (chip8->inst.N != 0)
            break;

        if (chip8->V[chip8->inst.X] == chip8->V[chip8->inst.Y])
            chip8->PC += 2;
        break;

    case 0x06:
        // 0x6XNN; V[X] <= NN
        chip8->V[chip8->inst.X] = chip8->inst.NN;
        break;

    case 0x07:
        // 0x7XNN. V[X] += NN
        chip8->V[chip8->inst.X] += chip8->inst.NN;
        break;

    case 0x08: // Operatii aritmetice
        uint8_t X = chip8->inst.X;
        uint8_t Y = chip8->inst.Y;
        uint16_t carry = 0;
        switch (chip8->inst.N)
        {
        case 0:
            chip8->V[X] = chip8->V[Y];
            break;

        case 1:
            chip8->V[X] |= chip8->V[Y];
            if (config.current_extension == 0)
                chip8->V[0xF] = 0;
            break;

        case 2:
            chip8->V[X] &= chip8->V[Y];
            if (config.current_extension == 0)
                chip8->V[0xF] = 0;
            break;

        case 3:
            chip8->V[X] ^= chip8->V[Y];
            if (config.current_extension == 0)
                chip8->V[0xF] = 0;
            break;

        case 4:
            chip8->V[X] = chip8->V[X] + chip8->V[Y];
            carry = ((uint16_t)(chip8->V[chip8->inst.X] + chip8->V[chip8->inst.Y]) > 255);
            chip8->V[X] += chip8->V[Y];
            chip8->V[0xF] = carry;
            break;

        case 5:
            if (chip8->V[X] >= chip8->V[Y])
                chip8->V[0xF] = 1;
            else
                chip8->V[0xF] = 0;
            chip8->V[X] = chip8->V[X] - chip8->V[Y];
            break;
        case 6:
            if (config.current_extension == 0)
            {
                carry = chip8->V[chip8->inst.Y] & 1;                    // Use VY
                chip8->V[chip8->inst.X] = chip8->V[chip8->inst.Y] >> 1; // Set VX = VY result
            }
            else
            {
                carry = chip8->V[chip8->inst.X] & 1; // Use VX
                chip8->V[chip8->inst.X] >>= 1;       // Use VX
            }

            chip8->V[0xF] = carry;
            break;
        case 7:
            chip8->V[X] = chip8->V[Y] - chip8->V[X];
            carry = (chip8->V[chip8->inst.X] <= chip8->V[chip8->inst.Y]);
            chip8->V[0xF] = carry;
            break;

        case 0x0E:
            if (config.current_extension == 0)
            {
                carry = (chip8->V[Y] & 0x80) >> 7;
                chip8->V[X] = chip8->V[Y] << 1;
            }
            else
            {
                carry = (chip8->V[X] & 0x80) >> 7;
                chip8->V[X] <<= 1;
            }
            chip8->V[0xF] = carry;
            break;

        default:
            break;
        }
        break;

    case 0x09:
        // 9XY0 if(Vx != Vy) skip next instruction
        if (chip8->V[chip8->inst.X] != chip8->V[chip8->inst.Y])
            chip8->PC += 2;
        break;
    case 0x0A:
        // 0xANNN; I (index register) <= NNN
        chip8->I = chip8->inst.NNN;
        break;

    case 0x0B:
        // 0xBNNN: jump to adress NNN + V[0]
        chip8->PC = chip8->inst.NNN + chip8->V[0];
        break;

    case 0x0C:
        // 0xCXNN = VX = rand() % 256 & NN
        chip8->V[chip8->inst.X] = (rand() % 256) & chip8->inst.NN;
        break;
    case 0x0D: {
            // 0xDXYN: Draw N-height sprite at coords X,Y; Read from memory location I;
            //   Screen pixels are XOR'd with sprite bits, 
            //   VF (Carry flag) is set if any screen pixels are set off; This is useful
            //   for collision detection or other reasons.
            uint8_t X_coord = chip8->V[chip8->inst.X] % config.window_width;
            uint8_t Y_coord = chip8->V[chip8->inst.Y] % config.window_height;
            const uint8_t orig_X = X_coord; // Original X value

            chip8->V[0xF] = 0;  // Initialize carry flag to 0

            // Loop over all N rows of the sprite
            for (uint8_t i = 0; i < chip8->inst.N; i++) {
                // Get next byte/row of sprite data
                const uint8_t sprite_data = chip8->ram[chip8->I + i];
                X_coord = orig_X;   // Reset X for next row to draw

                for (int8_t j = 7; j >= 0; j--) {
                    // If sprite pixel/bit is on and display pixel is on, set carry flag
                    bool *pixel = &chip8->display[Y_coord * config.window_width + X_coord]; 
                    const bool sprite_bit = (sprite_data & (1 << j));

                    if (sprite_bit && *pixel) {
                        chip8->V[0xF] = 1;  
                    }

                    // XOR display pixel with sprite pixel/bit to set it on or off
                    *pixel ^= sprite_bit;

                    // Stop drawing this row if hit right edge of screen
                    if (++X_coord >= config.window_width) break;
                }

                // Stop drawing entire sprite if hit bottom edge of screen
                if (++Y_coord >= config.window_height) break;
            }
            chip8->draw = true;
            break;
        }
    case 0x0E: // input handling
        if (chip8->inst.NN == 0x9E)
        {
            // 0xEX9E: Skip next instruction if key in VX is pressed
            if (chip8->keypad[chip8->V[chip8->inst.X]])
                chip8->PC += 2;
        }
        else if (chip8->inst.NN == 0xA1)
        {
            // 0xEX9E: Skip next instruction if key in VX is not pressed
            if (!chip8->keypad[chip8->V[chip8->inst.X]])
                chip8->PC += 2;
        }
        break;

    case 0x0F:
        switch (chip8->inst.NN)
        {
        case 0x0A:
            // 0x0FX0A : VX = get_key(); wait until a keypress then store it in VX
            static uint8_t key = 0xFF;
            static bool key_pressed = false;
            for (uint8_t i = 0; key == 0xFF && i < 16; i++)
            {
                if (chip8->keypad[i])
                {
                    key_pressed = true;
                    key = i;
                    break;
                }
            }
            if (!key_pressed)
                chip8->PC -= 2; // waits until a keypress
            else
            {
                if (chip8->keypad[key])
                    chip8->PC -= 2;
                else
                {
                    chip8->V[chip8->inst.X] = key; // VX = key
                    key = 0xFF;                    // reset to key not foundd
                    key_pressed = false;
                }
            }
            break;

        case 0x1E:
            // 0xFX1E: I += VX
            chip8->I += chip8->V[chip8->inst.X];
            break;

        case 0x07:
            // 0xFX07: VX = delay timer
            chip8->V[chip8->inst.X] = chip8->delay_timer;
            break;

        case 0x15:
            // 0xFX15: delay timer = VX
            chip8->delay_timer = chip8->V[chip8->inst.X];
            break;

        case 0x18:
            // 0xFX18: sound timer = VX
            chip8->sound_timer = chip8->V[chip8->inst.X];
            break;

        case 0x29:
            // 0xFX29: Set register I to sprite location in memory for character in VX (0x0-0xF)
            chip8->I = chip8->V[chip8->inst.X] * 5;
            break;

        case 0x33:
            // stores the BCD representaiton of VX, I = hundrend's digit, I+1 = ten's digit, i+2 = one's digit
            uint8_t bcd = chip8->V[chip8->inst.X];
            chip8->ram[chip8->I + 2] = bcd % 10;
            bcd /= 10;
            chip8->ram[chip8->I + 1] = bcd % 10;
            bcd /= 10;
            chip8->ram[chip8->I] = bcd;
            break;

        case 0x55:
            // 0xFX55: registry dump from 0 to X, starting from adress I. I is left unmodified
            //  SCHIP increments I, chip8 doesnt increment I
            for (uint8_t i = 0; i <= chip8->inst.X; i++)
            {
                if (config.current_extension == 0)
                    chip8->ram[chip8->I++] = chip8->V[i];
                else
                    chip8->ram[chip8->I + i] = chip8->V[i];
            }
            break;
        case 0x65:
            // 0xFX65: registry load from 0 to X, starting from adress I. I is left unmodified
            for (uint8_t i = 0; i <= chip8->inst.X; i++)
            {
                if (config.current_extension == 0)
                    chip8->V[i] = chip8->ram[chip8->I++]; // Increment I each time
                else
                    chip8->V[i] = chip8->ram[chip8->I + i];
            }
            break;
        default:
            break;
        }
        break;
    default:
        // puts("Unimplemented or invalid opcode");
        break;
    }
}

void update_timers(chip8_t *chip8)
{
    if (chip8->delay_timer > 0)
        chip8->delay_timer--;
    if (chip8->sound_timer > 0)
        chip8->sound_timer--;
    // to play osund;
}

int main(int argc, char **argv)
{
    srand(time(NULL));
    // Usage message for agcs
    if (argc < 2)
    {
        printf("Usage: %s <rom_name>\n", argv[0]);
        return -1;
    }
    (void)argc;
    (void)argv;
    // Initialise emulator config
    config_t config = {0};
    if (!set_config_from_args(&config, argc, argv))
        return -1;

    // Initialize SDL
    sdl_t sdl = {0};
    if (!init_sdl(&sdl, config))
        return -1;

    // Init chip8 machine
    chip8_t chip8 = {0};
    char *rom_name = argv[1];
    if (!init_chip8(&chip8, rom_name))
    {
        printf("initializaton failed\n");
        return -1;
    }
    clear_screen(sdl, config);

    // Main emulator loop
    while (chip8.state != QUIT)
    {
        // Handle user input
        handle_input(&chip8);
        if (chip8.state == PAUSED)
            continue;


        uint32_t start_time = SDL_GetTicks();

        // Emulate CHIP8 instructions for this frame (60 hz)
        for (uint32_t i = 0; i < config.instructions_per_second / 60; i++)
            emulate_instruction(&chip8, config);


        uint32_t end_time = SDL_GetTicks();

        // Delay for aprox 60Hz
        double time_elapsed = (double)(end_time - start_time) / 1000;

        if (16.67f > time_elapsed)
        {
            SDL_Delay(16.67f - time_elapsed); 
        }
        else{
            SDL_Delay(0);  
        }
        // Update window with changes on every iteration
        if(chip8.draw)
        {
            update_screen(sdl, config, chip8);
            chip8.draw = false;
        }
        update_timers(&chip8);
    }
    // Final cleanup
    final_cleanup(sdl);

    exit(EXIT_SUCCESS);
    return 0;
}