#include <stdio.h>
#include <SDL2/SDL.h>
#include <time.h>

// initialize and setup
const int SCREEN_WIDTH  = 900;
const int SCREEN_HEIGHT = 900;
const int BOARD_WIDTH   = 20;
const int BOARD_HEIGHT  = 20;
// ticks per second
const int GAME_SPEED    = 20;

const int TILE_WIDTH  = SCREEN_WIDTH / BOARD_WIDTH;
const int TILE_HEIGHT = SCREEN_HEIGHT / BOARD_HEIGHT;

struct snake_node {
    struct snake_node *next;
    int x, y;
};

enum direction {
    DIRECTION_RIGHT, DIRECTION_UP, DIRECTION_LEFT, DIRECTION_DOWN
};

struct snake_node *snake_node_alloc()
{
    struct snake_node *node = malloc(sizeof(struct snake_node));
    node->next = NULL;
    return node;
}

void snake_free(struct snake_node *head)
{
    // cleanup
    struct snake_node *tmp = head;
    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}

void reset(struct snake_node **head, struct snake_node **tail, enum direction *snake_direction)
{
    if (head != NULL) {
        snake_free(*head);
    }

    *snake_direction = DIRECTION_RIGHT;
    *head = snake_node_alloc();
    *tail = *head;
    (*head)->x = 1;
    (*head)->y = 0;
}

void get_apple_non_intersecting(struct snake_node *head, int *apple_x, int *apple_y)
{
    int did_intersect;
    do {
        did_intersect = 0;

        *apple_x = rand() % BOARD_WIDTH;
        *apple_y = rand() % BOARD_HEIGHT;

        struct snake_node *node = head;
        while (node != NULL) {
            if (*apple_x == node->x && *apple_y == node->y) {
                did_intersect = 1;
            }
            node = node->next;
        }

    } while (did_intersect);
}

SDL_Texture *load_texture_flushed(SDL_Renderer *renderer)
{
    SDL_Surface *flushed_surface = SDL_LoadBMP("flushed.bmp");
    SDL_Texture *texture_flushed = SDL_CreateTextureFromSurface(renderer, flushed_surface);
    SDL_FreeSurface(flushed_surface);
    return texture_flushed;
}

void snake_change_direction(SDL_Scancode *buffered_key, enum direction *snake_direction)
{
    // direction change handling
    switch (*buffered_key) {
        case SDL_SCANCODE_D:
            if (*snake_direction != DIRECTION_LEFT) {
                *snake_direction = DIRECTION_RIGHT;
            }
            break;
        case SDL_SCANCODE_W:
            if (*snake_direction != DIRECTION_DOWN) {
                *snake_direction = DIRECTION_UP;
            }
            break;
        case SDL_SCANCODE_A:
            if (*snake_direction != DIRECTION_RIGHT) {
                *snake_direction = DIRECTION_LEFT;
            }
            break;
        case SDL_SCANCODE_S:
            if (*snake_direction != DIRECTION_UP) {
                *snake_direction = DIRECTION_DOWN;
            }
            break;

        default:
            break;
    }

    *buffered_key = SDL_SCANCODE_UNKNOWN;
}

void snake_move(struct snake_node **head, struct snake_node **tail, enum direction snake_direction, int *apple_x, int *apple_y)
{
    int tail_prev_x = (*tail)->x;
    int tail_prev_y = (*tail)->y;
    (*tail)->x = (*head)->x;
    (*tail)->y = (*head)->y;
    switch (snake_direction) {
        case DIRECTION_RIGHT:
            (*tail)->x++;
            if ((*tail)->x >= BOARD_WIDTH) {
                (*tail)->x = 0;
            }
            break;
        case DIRECTION_UP:
            (*tail)->y--;
            if ((*tail)->y < 0) {
                (*tail)->y = BOARD_HEIGHT - 1;
            }
            break;
        case DIRECTION_LEFT:
            (*tail)->x--;
            if ((*tail)->x < 0) {
                (*tail)->x = BOARD_WIDTH - 1;
            }
            break;
        case DIRECTION_DOWN:
            (*tail)->y++;
            if ((*tail)->y >= BOARD_HEIGHT) {
                (*tail)->y = 0;
            }
            break;
    }

    // move old tail node to front and set it as the new head
    (*tail)->next = *head;
    *head = *tail;

    // find the new last node and set it as tail
    struct snake_node *node = *head;
    while (node->next != *tail) {
        node = node->next;
    }

    // alloc new tail if apple was eaten
    if ((*head)->x == *apple_x && (*head)->y == *apple_y) {
        *tail = snake_node_alloc();
        (*tail)->x = tail_prev_x;
        (*tail)->y = tail_prev_y;
        node->next = *tail;
        (*tail)->next = NULL;
        get_apple_non_intersecting(*head, apple_x, apple_y);
    } else {
        *tail = node;
        (*tail)->next = NULL;
    }
}

void draw(SDL_Renderer *renderer, struct snake_node *head, SDL_Texture *texture_flushed, int apple_x, int apple_y)
{
    // draw snek
    struct snake_node *node = head;
    if (texture_flushed != NULL)
        node = head->next;
    while (node != NULL) {
        SDL_Rect fill_rect = { node->x * TILE_WIDTH, node->y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT };
        SDL_SetRenderDrawColor(renderer, 0x80, 0x80, 0x80, 0xFF);
        SDL_RenderFillRect(renderer, &fill_rect);

        node = node->next;
    }

    // draw apple
    SDL_Rect fill_rect = { apple_x * TILE_WIDTH, apple_y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT };
    SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
    SDL_RenderFillRect(renderer, &fill_rect);

    // spice things up
    if (texture_flushed != NULL) {
        SDL_Rect fill_rect = { head->x * TILE_WIDTH, head->y * TILE_HEIGHT, TILE_WIDTH, TILE_HEIGHT };
        SDL_RenderCopy(renderer, texture_flushed, NULL, &fill_rect);
    }
}

int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window      = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer  = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_Texture *texture_flushed = NULL;
    for (int i = 1; i < argc; i++) {
        printf("%s\n", argv[i]);
        if (strcmp(argv[i], "-f") == 0) {
            texture_flushed = load_texture_flushed(renderer);
            printf("%p\n", texture_flushed);
        }
    }

    srand(time(NULL));

    // setup game variables
    enum direction snake_direction;
    struct snake_node *head = NULL;
    struct snake_node *tail = NULL;
    reset(&head, &tail, &snake_direction);

    int apple_x, apple_y;
    get_apple_non_intersecting(head, &apple_x, &apple_y);

    int time_previous = SDL_GetTicks();

    int quit = 0;
    int pause = 0;
    SDL_Event event;

    SDL_Scancode buffered_key;
    // game loop
    while (!quit) {
        // event polling and key buffering
        while (SDL_PollEvent(&event) != 0) {
            switch (event.type) {
                case SDL_QUIT:
                    quit = 2;
                    break;

                case SDL_KEYDOWN:
                    buffered_key = event.key.keysym.scancode;
                    if (buffered_key == SDL_SCANCODE_ESCAPE) {
                        quit = 1;
                    } else if (buffered_key == SDL_SCANCODE_P) {
                        pause = !pause;
                    }
                    break;
            }
        }

        int current_time = SDL_GetTicks();
        if (current_time - time_previous > 1000 / GAME_SPEED && !pause) {
            time_previous = current_time;

            snake_change_direction(&buffered_key, &snake_direction);
            snake_move(&head, &tail, snake_direction, &apple_x,&apple_y);

            // now check for game-over intersections
            struct snake_node *node = head->next;
            while (node != NULL) {
                if (head->x == node->x && head->y == node->y) {
                    reset(&head, &tail, &snake_direction);
                    break;
                }
                node = node->next;
            }
        }

        // render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        draw(renderer, head, texture_flushed, apple_x, apple_y);

        SDL_RenderPresent(renderer);
    }

    if (texture_flushed != NULL) {
        SDL_DestroyTexture(texture_flushed);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
