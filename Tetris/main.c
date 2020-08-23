#include <SDL2/SDL.h>
#include <stdio.h>
#include <time.h>

const int WINDOW_WIDTH  = 800;
const int WINDOW_HEIGHT = 800;
const int CELLS_X = 10;
const int CELLS_Y = 20;
const int SIDEBAR_X = WINDOW_WIDTH * 0.5f;
const int CELL_WIDTH  = (WINDOW_WIDTH - SIDEBAR_X) / CELLS_X;
const int CELL_HEIGHT = WINDOW_HEIGHT / CELLS_Y;
// time in ms per unit
const float TETRO_DELAY = 10000 / (float)CELLS_Y;

struct tetro {
    SDL_FPoint rotation_origin;
    SDL_Color color;
    SDL_Point geometry[4];
};

struct tetro create_tetro(int index, struct tetro *tetro_templates)
{
    struct tetro new_tetro = tetro_templates[index];
    memcpy(new_tetro.geometry, tetro_templates[index].geometry, sizeof(SDL_Point) * 4);

    return new_tetro;
}

void rotate_tetro(struct tetro *tetro, int direction, int map[CELLS_X][CELLS_Y])
{
    SDL_Point new_geometry[4];
    for (int i = 0; i < 4; i++) {
        new_geometry[i].x = tetro->rotation_origin.x + direction * tetro->rotation_origin.y - direction * tetro->geometry[i].y;
        new_geometry[i].y = tetro->rotation_origin.y - direction * tetro->rotation_origin.x + direction * tetro->geometry[i].x;

        if (map[new_geometry[i].x][new_geometry[i].y] != -1 || new_geometry[i].x < 0 || new_geometry[i].x >= CELLS_X) {
            return;
        }
    }

    memcpy(tetro->geometry, new_geometry, sizeof(SDL_Point) * 4);
}

// return 1 if the tetro has collided with the floor or another tile (y-movement only)
// return 2 if the tetro has collided with the floor and is not yet fully visible (game over)
int move_tetro(struct tetro *tetro, int dx, int dy, int map[CELLS_X][CELLS_Y])
{
    int part_not_visible = 0;
    SDL_Point new_geometry[4];
    for (int i = 0; i < 4; i++ ) {
        new_geometry[i].x = tetro->geometry[i].x + dx;
        new_geometry[i].y = tetro->geometry[i].y + dy;

        // skip checking for tiles that are not yet visible
        if (new_geometry[i].y > 0) {
            if (map[new_geometry[i].x][new_geometry[i].y] != -1) {
                if (abs(dx) > 0) {
                    return 0;
                } else {
                    // it collided with the floor while still new (game over)
                    if (part_not_visible) {
                        return 2;
                    } else {
                        return 1;
                    }
                }
            }
        } else {
            part_not_visible = 1;
        }

        if (new_geometry[i].x < 0 || new_geometry[i].x >= CELLS_X) {
            return 0;
        } else if (new_geometry[i].y >= CELLS_Y) {
            return 1;
        }
    }

    memcpy(tetro->geometry, new_geometry, sizeof(SDL_Point) * 4);
    tetro->rotation_origin.x += dx;
    tetro->rotation_origin.y += dy;

    return 0;
}

int main(int argc, char **argv)
{
    srand(time(NULL));

    struct tetro tetro_templates[] = {
        { {1.5f,  0.5f}, {255, 0,      0  }, { {0, 0}, {1, 0}, {2, 0},  {3, 0}}},
        { {1.0f,  1.0f}, {0,   0,      255}, { {0, 1}, {1, 1}, {2, 1},  {2, 2}}},
        { {1.0f,  1.0f}, {0,   255,    255}, { {0, 1}, {0, 2}, {1, 1},  {2, 1}}},
        { {1.0f,  1.0f}, {255, 128,    255}, { {0, 2}, {1, 1}, {1, 2},  {2, 1}}},
        { {1.0f,  1.0f}, {0,   255,    255}, { {0, 1}, {1, 1}, {1, 2},  {2, 1}}},
        { {1.0f,  1.0f}, {0,   255,    0  }, { {0, 1}, {1, 1}, {1, 2},  {2, 2}}},
        { {0.5f,  0.5f}, {255, 255,    0  }, { {0, 0}, {1, 0}, {0, 1},  {1, 1}}}
    };
    const int tetro_template_count = sizeof(tetro_templates) / sizeof(struct tetro);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow("Tetris", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    int map[CELLS_X][CELLS_Y];
    memset(map, -1, sizeof(map));

    int quit = 0;
    int score = 0;
    int pause = 0;
    int tile_over = 0;
    int previous_time = SDL_GetTicks();

    int current_tetro_index = rand() % tetro_template_count;
    struct tetro current_tetro = create_tetro(current_tetro_index, tetro_templates);
    move_tetro(&current_tetro, CELLS_X / 2, -3, map);
    int next_tetro_index = rand() % tetro_template_count;

    SDL_Event event;
    while (!quit) {
        // event handling
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.scancode) {
                        case SDL_SCANCODE_ESCAPE:
                            quit = 1;
                            break;
                        case SDL_SCANCODE_P:
                            pause = !pause;
                            break;

                        case SDL_SCANCODE_K:
                            if (!pause) {
                                rotate_tetro(&current_tetro, 1, map);
                            }
                            break;
                        case SDL_SCANCODE_J:
                            if (!pause) {
                                tile_over = move_tetro(&current_tetro, 0, 1, map);
                                if (!tile_over) {
                                    score += 1;
                                    printf("Score: %i\n", score);
                                }
                            }
                            break;
                        case SDL_SCANCODE_H:
                            if (!pause) {
                                move_tetro(&current_tetro, -1, 0, map);
                            }
                            break;
                        case SDL_SCANCODE_L:
                            if (!pause) {
                                move_tetro(&current_tetro, 1, 0, map);
                            }
                            break;

                        default:
                            break;
                    }
                    break;
            }
        }

        // updating
        if (!pause) {
            // moving and collision
            int current_time = SDL_GetTicks();
            if (current_time - previous_time >= TETRO_DELAY) {
                previous_time = current_time;
                tile_over = move_tetro(&current_tetro, 0, 1, map);
            }

            if (tile_over == 1) {
                for (int i = 0; i < 4; i++) {
                    map[current_tetro.geometry[i].x][current_tetro.geometry[i].y] = current_tetro_index;
                }
                current_tetro = create_tetro(next_tetro_index, tetro_templates);
                move_tetro(&current_tetro, CELLS_X / 2, -3, map);
                score += 1;
                printf("Score: %i\n", score);
                current_tetro_index = next_tetro_index;
                next_tetro_index = rand() % tetro_template_count;
                tile_over = 0;

                // finished line checking
                for (int y = CELLS_Y - 1; y >= 0; y--) {
                    int completed = 1;
                    for (int x = 0; x < CELLS_X; x++) {
                        if (map[x][y] == -1) {
                            completed = 0;
                        }
                    }

                    if (completed) {
                        score += 50;
                        printf("Score: %i\n", score);
                        for (int my = y; my > 0; my--) {
                            for (int mx = 0; mx < CELLS_X; mx++) {
                                map[mx][my] = map[mx][my - 1];
                            }
                        }

                        y--;
                    }
                }
            // game over
            } else if (tile_over == 2) {
                printf("game over\n");
                exit(0);
            }
        }

        // drawing
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_Rect draw_rect;
        draw_rect.w = CELL_WIDTH;
        draw_rect.h = CELL_HEIGHT;

        // draw existing cells
        for (int x = 0; x < CELLS_X; x++) {
            for (int y = 0; y < CELLS_Y; y++) {
                if (map[x][y] != -1) {
                    draw_rect.x = x * CELL_WIDTH;
                    draw_rect.y = y * CELL_HEIGHT;
                    SDL_Color *color = &tetro_templates[map[x][y]].color;
                    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);
                    SDL_RenderFillRect(renderer, &draw_rect);
                }
            }
        }

        // draw current tetromino
        for (int i = 0; i < 4; i++) {
            draw_rect.x = current_tetro.geometry[i].x * CELL_WIDTH;
            draw_rect.y = current_tetro.geometry[i].y * CELL_HEIGHT;
            SDL_SetRenderDrawColor(renderer, current_tetro.color.r, current_tetro.color.g, current_tetro.color.b, current_tetro.color.a);
            SDL_RenderFillRect(renderer, &draw_rect);
        }

        // draw tetromino preview
        SDL_Color *next_color = &tetro_templates[next_tetro_index].color;
        SDL_SetRenderDrawColor(renderer, next_color->r, next_color->g, next_color->b, next_color->a);
        for (int i = 0; i < 4; i++) {
            draw_rect.x = tetro_templates[next_tetro_index].geometry[i].x * CELL_WIDTH + SIDEBAR_X;
            draw_rect.y = tetro_templates[next_tetro_index].geometry[i].y * CELL_HEIGHT;
            SDL_RenderFillRect(renderer, &draw_rect);
        }

        // draw grid
        SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
        for (int x = 0; x < CELLS_X; x++) {
            for (int y = 0; y < CELLS_Y; y++) {
                draw_rect.x = x * CELL_WIDTH;
                draw_rect.y = y * CELL_HEIGHT;
                SDL_RenderDrawRect(renderer, &draw_rect);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_RenderPresent(renderer);
    }

    return 0;
}
