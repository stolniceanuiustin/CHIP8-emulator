#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include "SDL.h"

typedef struct{
    SDL_Window *window;
    SDL_Renderer *renderer;
}sdl_t;

typedef struct{
    int window_height;
    int window_width;
    int fg_color; //foregroud 0xRRGGBBAA
    int bg_color; //background 0xRRGGBBAA
    uint32_t scale_factor; 
}config_t;

typedef enum{
    QUIT,
    RUNNING,
    PAUSE,
}emulator_state_t;

typedef struct{
    emulator_state_t state;
}chip8_t;



//Initializare
bool init_sdl(sdl_t* sdl, config_t config)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0)
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
                                    
    if(!sdl->window)
    {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    //RENDERER
    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if(!sdl->renderer)
    {
        printf("renderer could not be created! %s\n", SDL_GetError());
        return false;
    }
    return true;
}

//Iniitial emulator config from passed arguments 
bool set_config_from_args(config_t *config, int argc, char **argv)
{

    //set default
    config->window_width = 64; //CHIP8 X RESOLUTION
    config->window_height = 32; //Y
    config->fg_color = 0xFFFFFFFF;
    config->bg_color = 0xFFFFFFFF;
    config->scale_factor = 20; //1280x640
    //override default from args
    for(int i=1; i < argc; i++)
    {
        (void)argv[i];
    }

    return true;
}



//clear sdl window to background color
void clear_screen(sdl_t sdl, const config_t config)
{
    uint8_t r = (config.bg_color >> 24) & 0xFF;
    uint8_t g = (config.bg_color >> 16) & 0xFF;
    uint8_t b = (config.bg_color >> 8) & 0xFF;
    uint8_t a = (config.bg_color >> 0) & 0xFF;

    SDL_SetRenderDrawColor(sdl.renderer, r, g, b, a);
    SDL_RenderClear(sdl.renderer);
}

//update window with any changes
void update_screen(sdl_t sdl)
{
    SDL_RenderPresent(sdl.renderer);
}

void handle_input(chip8_t* chip8)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                chip8->state = QUIT;
                return;
            
            case SDL_KEYDOWN:
                break;
            
            case SDL_KEYUP:
               break;
            
            default:
                break;
        }
    }
}

bool init_chip8(chip8_t* chip8)
{
    chip8->state = RUNNING;
    return true;
}

//Cleanup
void final_cleanup(sdl_t sdl)
{
    SDL_DestroyRenderer(sdl.renderer);
    SDL_DestroyWindow(sdl.window);
    SDL_Quit();
    return;
}


int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    //puts("TEST");
    //Initialise emulator config
    config_t config = {0};
    if(!set_config_from_args(&config, argc, argv))
        return -1;
    
    //Initialize SDL
    sdl_t sdl = {0};
    if(!init_sdl(&sdl, config)) 
        return -1;

    //Init chip8 machine
    chip8_t chip8 = {0};
    if(!init_chip8(&chip8))
        return -1;
    
    clear_screen(sdl, config);

    //Main emulator loop
    while(chip8.state != QUIT){
        //Handle user input
        handle_input(&chip8);
        //if (chip8.state == PAUSED) continue

        //Get_time();
        //Emulate CHIP8 instructions 
        //Get_time() elapsed since last get_time

        //Delay for aprox 60Hz
        SDL_Delay(16 /*- actual time elapsed*/);
        //Update window with changes on every iteration
        update_screen(sdl);
    } 
    //Final cleanup
    final_cleanup(sdl);

    exit(EXIT_SUCCESS);
    return 0;
}