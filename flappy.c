#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define SDL_DISABLE_IMMINTRIN_H
#include <SDL.h>
#include <SDL_ttf.h>
#include <string.h>
#include "Flappy.h"

// Window dimensions
#define W 480
#define H 600

// Game constants
#define GROUND 80
#define PIPE_W 86
#define PHYS_W (W + PIPE_W + 80)
#define GAP 220
#define GRACE 4
#define RANDOM_PIPE_HEIGHT (rand() % (H - GROUND - GAP - 120) + 60)
#define PLYR_X 80
#define PLYR_SZ 60

// Game states
enum gamestates { READY, ALIVE, GAMEOVER } gamestate = READY;

// Player properties
float player_y = (H - GROUND) / 2;
float player_vel = 0.0f;

// Pipe properties
int pipe_x[2] = { W, W };
float pipe_y[2];

// Score tracking
int score = 0;
int best = 0;

// Idle time counter
int idle_time = 30;

// Animation frame
float frame = 0.0f;

// SDL globals
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Event event;
SDL_Texture* pillar = NULL;
SDL_Texture* background = NULL;
SDL_Texture* bird[4] = { NULL };
TTF_Font* font = NULL;

//------------------------------------------------------------------------------
// Centralized error-handling and exit function
void cleanup_and_exit(const char* msg)
{
    if (msg) {
        fprintf(stderr, "%s\n", msg);
    }

    // Destroy textures
    for (int i = 0; i < 4; i++) {
        if (bird[i]) SDL_DestroyTexture(bird[i]);
    }
    if (pillar) SDL_DestroyTexture(pillar);
    if (background) SDL_DestroyTexture(background);

    // Close font and quit TTF
    if (font) {
        TTF_CloseFont(font);
    }
    TTF_Quit();

    // Destroy renderer and window
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);

    // Quit SDL
    SDL_Quit();
    exit(0);
}

//------------------------------------------------------------------------------
// Loads a BMP file into an SDL_Texture with an optional color key
SDL_Texture* load_texture(const char* file, Uint32 colorKey)
{
    SDL_Surface* surf = SDL_LoadBMP(file);
    if (!surf) {
        char errMsg[128];
        snprintf(errMsg, sizeof(errMsg), "Failed to load %s: %s", file, SDL_GetError());
        cleanup_and_exit(errMsg);
    }

    SDL_SetColorKey(surf, SDL_TRUE, colorKey);
    SDL_Texture* tex = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);

    if (!tex) {
        char errMsg[128];
        snprintf(errMsg, sizeof(errMsg), "Failed to create texture from %s: %s", file, SDL_GetError());
        cleanup_and_exit(errMsg);
    }
    return tex;
}

//------------------------------------------------------------------------------
// 1) setup()
// Initializes SDL, SDL_ttf, window, renderer, textures, and font
void setup()
{
    srand((unsigned int)time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cleanup_and_exit("SDL_Init Error");
    }

    window = SDL_CreateWindow("Flappy",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              W, H,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!window) {
        cleanup_and_exit("SDL_CreateWindow Error");
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        cleanup_and_exit("SDL_CreateRenderer Error");
    }

    // Optional: attempt fullscreen desktop
    if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
        fprintf(stderr, "Could not enable fullscreen: %s\n", SDL_GetError());
        // Not fatal; continue
    }

    // Set logical size for rendering
    if (SDL_RenderSetLogicalSize(renderer, W, H) != 0) {
        fprintf(stderr, "SDL_RenderSetLogicalSize Error: %s\n", SDL_GetError());
    }

    // Load textures
    pillar = load_texture("res/pillar.bmp", 0xFFFF00);
    background = load_texture("res/background.bmp", 0x000000);

    for (int i = 0; i < 4; i++) {
        char file[64];
        snprintf(file, sizeof(file), "res/bird-%d.bmp", i);
        bird[i] = load_texture(file, 0xFFFF00);
    }

    // Initialize TTF
    if (TTF_Init() == -1) {
        cleanup_and_exit("TTF_Init Error");
    }

    // Load font (adjust the path to the font on your system)
    font = TTF_OpenFont("/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf", 42);
    if (!font) {
        cleanup_and_exit("Failed to load font");
    }
}

//------------------------------------------------------------------------------
// 2) new_game()
// Resets the game variables to start a new round
void new_game()
{
    gamestate = ALIVE;
    player_y = (H - GROUND) / 2;
    player_vel = -11.7f;
    score = 0;
    pipe_x[0] = PHYS_W + PHYS_W / 2 - PIPE_W;
    pipe_x[1] = PHYS_W - PIPE_W;
    pipe_y[0] = RANDOM_PIPE_HEIGHT;
    pipe_y[1] = RANDOM_PIPE_HEIGHT;
}

//------------------------------------------------------------------------------
// 3) game_over_func()
// Moves the game to GAMEOVER state and updates best score
void game_over_func()
{
    gamestate = GAMEOVER;
    idle_time = 0;
    if (best < score) {
        best = score;
    }
}

//------------------------------------------------------------------------------
// 4) update_stuff()
// Updates player movement, gravity, and pipes
void update_stuff()
{
    if (gamestate != ALIVE) return;

    // Gravity
    player_y += player_vel;
    player_vel += 0.61f;

    // Bird frame animation
    if (player_vel > 10.0f) {
        frame = 0.0f;
    } else {
        frame -= (player_vel - 10.0f) * 0.03f;
    }

    // Check ground collision
    if (player_y > H - GROUND - PLYR_SZ) {
        game_over_func();
    }

    // Update pipes
    for (int i = 0; i < 2; i++) {
        update_pipe(i);
    }
}

//------------------------------------------------------------------------------
// 5) update_pipe(int i)
// Handles pipe movement, collision detection, scoring, and respawning
void update_pipe(int i)
{
    // Collision detection
    if (PLYR_X + PLYR_SZ >= pipe_x[i] + GRACE &&
        PLYR_X <= pipe_x[i] + PIPE_W - GRACE)
    {
        // Outside the gap
        if (player_y <= pipe_y[i] - GRACE ||
            player_y + PLYR_SZ >= pipe_y[i] + GAP + GRACE)
        {
            game_over_func();
        }
    }

    // Move pipe left
    pipe_x[i] -= 5;

    // Score increment
    if (pipe_x[i] + PIPE_W < PLYR_X && pipe_x[i] + PIPE_W > PLYR_X - 5) {
        score++;
        printf("Score incremented! Current score: %d\n", score);
    }

    // Respawn pipe if it goes off-screen
    if (pipe_x[i] <= -PIPE_W) {
        pipe_x[i] = PHYS_W - PIPE_W;
        pipe_y[i] = RANDOM_PIPE_HEIGHT;
    }
}

//------------------------------------------------------------------------------
// 6) render_text(const char *msg, int height)
// Renders text on the screen at a specified vertical position
void render_text(const char *msg, int height)
{
    if (!font) {
        fprintf(stderr, "Font not loaded. Cannot render text.\n");
        return;
    }

    int w, h;
    if (TTF_SizeText(font, msg, &w, &h) != 0) {
        fprintf(stderr, "TTF_SizeText Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Color white = { 255, 255, 255 };
    SDL_Surface *msgsurf = TTF_RenderText_Blended(font, msg, white);
    if (!msgsurf) {
        fprintf(stderr, "TTF_RenderText_Blended Error: %s\n", TTF_GetError());
        return;
    }

    SDL_Texture *msgtex = SDL_CreateTextureFromSurface(renderer, msgsurf);
    if (!msgtex) {
        fprintf(stderr, "SDL_CreateTextureFromSurface Error: %s\n", SDL_GetError());
        SDL_FreeSurface(msgsurf);
        return;
    }

    SDL_Rect fromrec = { 0, 0, msgsurf->w, msgsurf->h };
    SDL_Rect torec = { (W - w) / 2, height, w, h };
    SDL_RenderCopy(renderer, msgtex, &fromrec, &torec);

    SDL_DestroyTexture(msgtex);
    SDL_FreeSurface(msgsurf);
}

//------------------------------------------------------------------------------
// 7) render_score(int current_score, int height)
// Displays the current game score
void render_score(int current_score, int height)
{
    char score_str[32];
    snprintf(score_str, sizeof(score_str), "Score: %d", current_score);
    render_text(score_str, height);
}

//------------------------------------------------------------------------------
// 8) render_high_score(int high_score, int height)
// Displays the best (high) score
void render_high_score(int high_score, int height)
{
    char high_score_str[32];
    snprintf(high_score_str, sizeof(high_score_str), "High Score: %d", high_score);
    render_text(high_score_str, height);
}

//------------------------------------------------------------------------------
// 9) draw_stuff()
// Renders all game elements: background, pipes, bird, scores, messages
void draw_stuff()
{
    // Background
    SDL_Rect dest = { 0, 0, W, H };
    SDL_RenderCopy(renderer, background, NULL, &dest);

    // Pipes
    for (int i = 0; i < 2; i++) {
        int lower = (int)(pipe_y[i] + GAP);

        // Top pipe
        SDL_Rect topPipeRect = { pipe_x[i], (int)pipe_y[i] - H, PIPE_W, H };
        SDL_RenderCopy(renderer, pillar, NULL, &topPipeRect);

        // Bottom pipe
        SDL_Rect src = { 0, 0, PIPE_W, H - lower - GROUND };
        SDL_Rect bottomPipeRect = { pipe_x[i], lower, PIPE_W, H - lower - GROUND };
        SDL_RenderCopy(renderer, pillar, &src, &bottomPipeRect);
    }

    // Bird
    SDL_RenderCopy(renderer, bird[(int)frame % 4], NULL,
                   &(SDL_Rect){ PLYR_X, (int)player_y, PLYR_SZ, PLYR_SZ });

    // Score
    render_score(score, 10);

    // Messages depending on game state
    if (gamestate == READY) {
        render_text("Press any key", 150);
    }
    else if (gamestate == GAMEOVER) {
        render_high_score(best, 150);
    }

    SDL_RenderPresent(renderer);
}

//------------------------------------------------------------------------------
// Main Function
void main()
{
    setup();

    // Main game loop
    while (1)
    {
        // Process events
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    cleanup_and_exit(NULL);
                    break;

                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        cleanup_and_exit(NULL);
                    }
                    // Fall through to handle other key presses

                case SDL_MOUSEBUTTONDOWN:
                    if (gamestate == ALIVE) {
                        // Bird jump
                        player_vel = -11.7f;
                        frame += 1.0f;
                    } 
                    else if (idle_time > 30) {
                        new_game();
                    }
                    break;
            }
        }

        // Update and render
        update_stuff();
        draw_stuff();

        SDL_Delay(1000 / 60);
        idle_time++;
    }

    // Normally unreachable, but just in case
    cleanup_and_exit(NULL);
}