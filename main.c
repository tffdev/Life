#include <stdio.h>
#include <SDL2/SDL.h>

#define SCALE 3
#define SCREEN_WIDTH 100
#define SCREEN_HEIGHT 100

void set_cell(SDL_Surface *surface, int x, int y, int colour) {
    if(x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) return;
     Uint16* pixel = surface->pixels + y * surface->pitch + x * sizeof(Uint16);
     if(colour == 1) *pixel = 0xf000;
     if(colour == 0) *pixel = 0xffff;
}

int get_cell(SDL_Surface *surface, int x, int y) {
    Uint16* pixel = surface->pixels + y * surface->pitch + x * sizeof(Uint16);
    if(*pixel == 0xf000) return 1;
    if(*pixel == 0xffff) return 0;
}

/*
 * Process all cells!
 * Explanations on this found here:
 * https://robertheaton.com/2018/07/20/project-2-game-of-life/
 */
void process_cells(SDL_Surface* result_surface, SDL_Surface* input_surface) {
    // copy current to buffer
    SDL_BlitScaled(result_surface, NULL, input_surface, NULL);
    // Fill white
    SDL_FillRect(result_surface, NULL, 0xffff);

    for(int x = 0; x < SCREEN_WIDTH; x++) {
        for(int y = 0; y < SCREEN_HEIGHT; y++) {

            // Count the surrounding cells
            int count = 0;
            for (int dx = x - 1; dx <= x + 1; dx++) {
                for (int dy = y - 1; dy <= y + 1; dy++) {
                    if(!(dx==x && dy==y) && dx > 0 && dx < SCREEN_WIDTH && dy > 0 && dy < SCREEN_HEIGHT)
                        count += get_cell(input_surface, dx, dy);
                }
            }

            // Rules
            int cell = get_cell(input_surface, x, y);
            if(cell == 1) {
                // Any live cell with 0 or 1 live neighbors becomes dead, because of underpopulation
                if(count == 0 || count == 1) set_cell(result_surface, x, y, 0);
                // Any live cell with 2 or 3 live neighbors stays alive, because its neighborhood is just right
                if(count == 2 || count == 3) set_cell(result_surface, x, y, 1);
                // Any live cell with more than 3 live neighbors becomes dead, because of overpopulation
                if(count > 3)                set_cell(result_surface, x, y, 0);
            } else {
                // Any dead cell with exactly 3 live neighbors becomes alive, by reproduction
                if(count == 3) set_cell(result_surface, x, y, 1);
            }
        }
    }
}


/*
 * Life
 * ==========================
 * Works using 2 SDL surfaces
 * ~> Paint cells onto main screen
 * ~> Copy main screen to a buffer
 * ~> Apply ruleset to the contents of the buffer, the result is project onto the main screen
 * ~> Display the main screen
 */
int WinMain() {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
                "bois", SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH * SCALE, SCREEN_HEIGHT * SCALE, SDL_WINDOW_SHOWN);

    SDL_Surface* window_surface = SDL_GetWindowSurface(window);

    // init playing field! (& buffer)
    SDL_Surface* main_field = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 16, 0x000f, 0x00f0, 0x0f00, 0xf000);
    SDL_Surface* bufr_field = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 16, 0x000f, 0x00f0, 0x0f00, 0xf000);
    SDL_FillRect(main_field, NULL, 0xffff);

    /*
     * The main loop!
     * Every frame checks for mouse paint events,
     * but cells are only processed every 10 frames.
     */
    int frames_before_process = 10;
    for(int quit = 0; quit == 0;) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) quit = 1;
        }

        frames_before_process--;
        if(frames_before_process < 0) {
            frames_before_process = 10;
            process_cells(main_field, bufr_field);
        }

        // Place a live cell at mouse x and y if LMB is pressed
        SDL_PumpEvents();
        int mouse_x, mouse_y;
        if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            set_cell(main_field, mouse_x/SCALE, mouse_y/SCALE, 1);
        }

        // Draw our field to the screen
        SDL_BlitScaled(main_field, NULL, window_surface, NULL);
        SDL_UpdateWindowSurface(window);
        SDL_Delay(1);
    }
    return 0;
}
