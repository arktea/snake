#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

#define INIT_LENGTH 5
#define MAX_LENGTH 100000
#define START_X 10
#define START_Y 10
#define BORDER 5
#define INITIAL_DELAY 200000
#define ACCELERATION_RATIO 0.98
#define SNAKE_REPR "o"
#define APPLE_REPR "a"
#define SNAKE_HEAD "@"

typedef enum {
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4
} direction_t;

typedef struct {
    short x;
    short y;
} point_t;

typedef struct {
    size_t size;
    point_t* body;
    direction_t direction;
} snake_t;

direction_t scan_direction() {
    if (getch() == '\033') {
        getch();
        switch(getch()) {
            case 'A':
                return UP;
            case 'B':
                return DOWN;
            case 'C':
                return RIGHT;
            case 'D':
                return LEFT;
        }
    }
    return 0;
}


snake_t* new_snake() {
    snake_t* snake = malloc(sizeof(snake_t));
    snake->body = calloc(MAX_LENGTH, sizeof(point_t));
    if (snake == NULL || snake->body == NULL) {
        printf("Error while allocating snake structure");
        exit(EXIT_FAILURE);
    }
    snake->size = INIT_LENGTH;
    for (int x=0; x<INIT_LENGTH; x++) {
        snake->body[x] = (point_t) {START_X+x, START_Y};
    }
    snake->direction = RIGHT;
    return snake;
}

void display_snake(WINDOW* win, snake_t* snake) {
    for (int i=0; i<snake->size-1; i++) {
        point_t point = snake->body[i];
        mvwprintw(win, point.y, point.x, SNAKE_REPR);
    }
    mvwprintw(win, snake->body[snake->size-1].y, snake->body[snake->size-1].x, SNAKE_HEAD);
} 

void display_apple(WINDOW* win, point_t apple) {
    mvwprintw(win, apple.y, apple.x, APPLE_REPR);
}

void move_forward(snake_t* snake, direction_t direction, size_t max_x, size_t max_y) {
    if (direction != 0) {
        snake->direction = direction;
    }
    for (int i=0; i<snake->size-1; i++) {
        snake->body[i] = snake->body[i+1];
    }
    point_t head = snake->body[snake->size-2];
    point_t new_head;
    switch(snake->direction) {
        case UP:
            new_head = (point_t) {head.x, (head.y+max_y-2) % max_y + 1};
            break;
        case DOWN:
            new_head = (point_t) {head.x, (head.y) % max_y + 1};
            break;
        case LEFT:
            new_head = (point_t) {(head.x+max_x-2) % max_x + 1, head.y};
            break;
        case RIGHT:
            new_head = (point_t) {(head.x) % max_x + 1, head.y};
            break;
    }
    snake->body[snake->size-1] = new_head;
}


bool snake_eat(snake_t* snake, point_t apple) {
    if (snake->body[snake->size-1].x == apple.x && snake->body[snake->size-1].y == apple.y) {
        snake->body[snake->size] = apple;
        snake->size++;
        return true;
    }
    return false;
}

point_t generate_apple(snake_t* snake, short max_x, short max_y) {
    short position[2], maxxy[2] = {max_x, max_y};
    bool is_valid = false;
    while (!is_valid) {
        for (int i=0; i<2; i++) {
            position[i] = rand() % maxxy[i] + 1;
        }
        is_valid = true;
        for (int i=0; i<snake->size; i++) {
            if (position[0] == snake->body[i].x && position[1] == snake->body[i].y) {
                is_valid = false;
                break;
            }
        }
    }
    return (point_t) {position[0], position[1]};
}

bool has_collision(snake_t* snake) {
    point_t head = snake->body[snake->size-1];
    for (int i=0; i<snake->size-1; i++) {
        point_t body_part = snake->body[i];
        if (body_part.x == head.x && body_part.y == head.y) {
            return true;
        }
    }
    return false;
}


WINDOW* init_ncurses_window(size_t* size_x, size_t* size_y) {
    int max_x, max_y;
    initscr();
    getmaxyx(stdscr, max_y, max_x);
    *size_x = 2*max_y - BORDER;
    *size_y = max_y - BORDER;
    WINDOW *win = newwin(*size_y, *size_x, BORDER/2, BORDER);
    noecho();
    curs_set(FALSE);
    timeout(0);
    return win;
}


int main(int argc, char *argv[]) {

    srand(time(NULL));    
    size_t size_x, size_y;
    WINDOW* win = init_ncurses_window(&size_x, &size_y);    
    snake_t* snake = new_snake();
    point_t apple = generate_apple(snake, size_x-2, size_y-2);
    int delay = INITIAL_DELAY;
    unsigned int score = 100;
    while (true) {
        wclear(win);
        box(win, 0, 0);
        display_apple(win, apple);
        if (snake_eat(snake, apple)) {
            apple = generate_apple(snake, size_x-2, size_y-2);
            delay *= ACCELERATION_RATIO;
            score += 100;
        }
        move_forward(snake, scan_direction(), size_x-2, size_y-2);
        if (has_collision(snake)) {
            wclear(win);
            box(win, 0, 0);
            mvwprintw(win, size_y/2-1, size_x/2 - 5, "GAME OVER");
            mvwprintw(win, size_y/2+1, size_x/2 - 5, "Score %d", score);
            wrefresh(win);
            sleep(3);
            endwin();
            exit(0);
        }
        display_snake(win, snake);
        wrefresh(win);
        mvprintw(1, 1, "     Score %d", score);
        refresh();
        usleep(delay);
    }
      
    endwin();
    return 0;
}