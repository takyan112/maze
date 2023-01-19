#include <stdio.h>

/* The state must be initialized to non-zero */
unsigned int xorshift32_state = 123;
unsigned int xorshift32()
{
	/* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
	unsigned int x = xorshift32_state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	return xorshift32_state = x;
}

typedef enum {unvisited, wall, visited} Cell;

#define MAX_MAZE_SIZE 128 * 128
typedef struct {
    int width;
    int height;
    Cell cells[MAX_MAZE_SIZE];
} Maze;

Cell maze_cell_get(Maze *m, int x, int y) {
    int i = x + m->width * y;
    if (0 <= i && i < MAX_MAZE_SIZE)
        return m->cells[i];
    return unvisited;
}

void maze_cell_set(Maze *m, int x, int y, Cell cell) {
    int i = x + m->width * y;
    if (0 <= i && i < MAX_MAZE_SIZE)
        m->cells[i] = cell;
}

void maze_fill_with(Maze *m, Cell fill) {
    for (int i = 0; i < m->width * m->height && i < MAX_MAZE_SIZE; ++i)
        m->cells[i] = fill;
}

// int maze_has_any(Maze *m, Cell any) {
//     for (int i = 0; i < m->width * m->height && i < MAX_MAZE_SIZE; ++i)
//         if (m->cells[i] == any)
//             return 1;
//     return 0;
// }

void maze_generation(Maze *m) {
    int x = 0;
    int y = 0;
    
    int stopAll = 0;

    for (int i = 0; i < 500 && !stopAll; ++i) {
        // mark visited
        maze_cell_set(m, x, y, visited);
        
        // make around walls if is not visited
        if (x     > 0        ) if (maze_cell_get(m, x - 1, y) == unvisited) maze_cell_set(m, x - 1, y, wall);
        if (x + 1 < m->width ) if (maze_cell_get(m, x + 1, y) == unvisited) maze_cell_set(m, x + 1, y, wall);
        if (y     > 0        ) if (maze_cell_get(m, x, y - 1) == unvisited) maze_cell_set(m, x, y - 1, wall);
        if (y + 1 < m->height) if (maze_cell_get(m, x, y + 1) == unvisited) maze_cell_set(m, x, y + 1, wall);

        // pick a random wall that if remove, still surround by walls
        // if can not, stop the function
        Cell randomWall;
        int valid;
        int try = 0;
        do {
            ++try;
            valid = 1;
            x = xorshift32() % m->width;
            y = xorshift32() % m->height;
            randomWall = maze_cell_get(m, x, y);

            if (randomWall != wall) valid = 0;
            
            int count = 0;
            if (x     > 0        ) if (maze_cell_get(m, x - 1, y) == visited) ++count;
            if (x + 1 < m->width ) if (maze_cell_get(m, x + 1, y) == visited) ++count;
            if (y     > 0        ) if (maze_cell_get(m, x, y - 1) == visited) ++count;
            if (y + 1 < m->height) if (maze_cell_get(m, x, y + 1) == visited) ++count;

            if (count != 1) valid = 0;
            if (try >= 1000) stopAll = 1;
        } while (!valid && !stopAll);
    }
}

void maze_print_cli(Maze *m) {
    for (int y = 0; y < m->height; ++y) {
        for (int x = 0; x < m->width; ++x) {
            switch (maze_cell_get(m, x, y)) {
                case visited: putchar('`'); break;
                case wall:    putchar('#'); break;
                default:      putchar('&');
            }
        }
        putchar('\n');
    }
}

int main() {
    Maze maze;
    maze.width = 32;
    maze.height = 16;

    maze_fill_with(&maze, unvisited);
    maze_generation(&maze);
    maze_print_cli(&maze);

    printf("done\n");

    return 0;
}