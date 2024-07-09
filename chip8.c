#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
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
    config->scale_factor = 20; // 1280x640
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
void update_screen(sdl_t sdl)
{
    SDL_RenderPresent(sdl.renderer);
}

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
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                // Escape key. exit window
                chip8->state = QUIT;
                return;
            
            case SDLK_SPACE:
                if(chip8->state == RUNNING) //Pause
                    chip8->state = PAUSED;
                else 
                    {
                        chip8->state = RUNNING; //Resume
                        puts("==== PAUSED ====");
                    }
                return;
            default:
                break;
            }
            break;

        case SDL_KEYUP:
            break;

        default:
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
    memcpy(&chip8->ram[0], font, sizeof(font));
    // Load ROM
    FILE *rom = fopen(rom_name, "rb");
    if(!rom){
        printf("Could not open ROM FILE: %s\n", rom_name);
        return false;
    }
    
    // Get ROM SIZE
    fseek(rom, 0, SEEK_END);
    const size_t rom_size = ftell(rom); //Moment C
    const size_t max_size = sizeof chip8->ram - entry_point; 
    rewind(rom);

    if(rom_size > max_size)
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
    printf("Adress: 0x%04X, Opcode: 0x%04X Desc: ", chip8->PC-2, chip8->inst.opcode);
    switch((chip8->inst.opcode >> 12) & 0x0F) //First 4 bits of the opcode
    {
        case 0x0: // The instruction begins with 0
            if(chip8->inst.opcode == 0x00E0) // Clear screen
            {
                printf("Clear screen\n");
            }
            else if (chip8->inst.opcode == 0x00EE) // Return
            {
                printf("Return from subroutine to adress 0x%04X\n", *(chip8->stack_ptr - 1));
            }
            break;
        case 0x02:          // Calls subroutine at NNN
            *chip8->stack_ptr++ = chip8->PC; // Push return adress
            chip8->PC = chip8->inst.NNN;   // Change program counter
            break;
        case 0x06:
            //0x6XNN; V[X] <= NN
            printf("Set register V%X to NN(0x%02X)\n", chip8->inst.X, chip8->inst.NN);
            break;
        case 0x0A
            //0xANNN; I (index register) <= NNN
            printf("Index register 0x%04X <- 0x%04X\n", chip8->I, chip8->inst.NNN);
            break;
            
        default:
            printf("Unimplemented or invalid opcode\n");
            break;
    }
}
#endif

void emulate_instruction(chip8_t *chip8)
{
    //Get next opcode rom RAM
    chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | (chip8->ram[chip8->PC+1]); //little endian -> big endian
    chip8->PC += 2; // Increment program counter for next opcode

    //Fill out current instruction format
    chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
    chip8->inst.NN = chip8->inst.opcode & 0x00FF;
    chip8->inst.N = chip8->inst.opcode & 0x000F;
    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;

#ifdef DEBUG
    print_debug_info(chip8);
#endif

    //Emulate opcode
    switch((chip8->inst.opcode >> 12) & 0x0F) //First 4 bits of the opcode
    {
        case 0x0: // The instruction begins with 0
            if(chip8->inst.opcode == 0x00E0) // Clear screen
            {
                memset(&chip8->display[0], 0, 64*32); //Sets all the bits to 0 
            }
            else if (chip8->inst.opcode == 0x00EE) // Return
            {
                // Returns from a subroutine
                // Set program counter to return adress from the stack
                chip8->PC = *(--chip8->stack_ptr);
            }
            else
            {
                //printf("Unimplemeneted\n");
                int ttttt = 0;
                if(ttttt == 5) ttttt = 2;
            }
            break;
        case 0x01:
            //goto NNN
            chip8->PC = chip8->inst.NNN;
            break;
        
        case 0x02:          // Calls subroutine at NNN
            *chip8->stack_ptr++ = chip8->PC; // Push return adress
            chip8->PC = chip8->inst.NNN;   // Change program counter
            break;
        
        case 0x0A:
            //0xANNN; I (index register) <= NNN
            chip8->I = chip8->inst.NNN;
            break;
        
        
        case 0x06:
            //0x6XNN; V[X] <= NN
            chip8->V[chip8->inst.X] = chip8->inst.NN; 
            break;
        default:
            //puts("Unimplemented or invalid opcode");
            break;
    }
}

int main(int argc, char **argv)
{
    //Usage message for agcs
    if (argc < 2)
    {
        printf("Usage: %s <rom_name>\n", argv[0]);
        return -1;
    }
    (void)argc;
    (void)argv;
    // puts("TEST");
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
        

        if (chip8.state == PAUSED) continue;

        // Get_time();
        // Emulate CHIP8 instructions
        emulate_instruction(&chip8);
        // Get_time() elapsed since last get_time

        // Delay for aprox 60Hz
        SDL_Delay(16 /*- actual time elapsed*/);
        // Update window with changes on every iteration
        update_screen(sdl);
    }
    // Final cleanup
    final_cleanup(sdl);

    exit(EXIT_SUCCESS);
    return 0;
}