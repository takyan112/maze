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
                case wall:    putchar('%'); break;
                default:      putchar('&');
            }
        }
        putchar('\n');
    }
}

int maze_path_find(Maze *m, int x, int y, int a, int b) {
    if (x < 0          || y < 0           ||
        x >= m->width  || y >= m->height  ||
        a < 0          || b < 0           ||
        a >= m->width  || b >= m->height  ||
        maze_cell_get(m, x, y) != visited || 
        maze_cell_get(m, a, b) != visited) {
        return -1;
    }
    int distance[MAX_MAZE_SIZE];
    for (int i = 0; i < MAX_MAZE_SIZE; ++i) {
        distance[i] = -1;
    }
    
    int count = 1;
    int stack[MAX_MAZE_SIZE];

    stack[0] = x + m->width * y;
    distance[stack[0]] = 0;
    
    for (int iter = 0; 0 < count && count < MAX_MAZE_SIZE-100 && iter < 10000; ++iter) {
        int i = stack[--count];
        int d = ++distance[i];
        
        int x = i % m->width;
        int y = i / m->width;
        // if neighbouring cells have no distance recorded and inside the maze
        // record the distance and add to the stack for later exploring its neignbour  
        if (x     > 0        ) {int j = i - 1;        if (distance[j] < 0 && m->cells[j] == visited) {distance[j] = d; stack[count++] = j; }}
        if (x + 1 < m->width ) {int j = i + 1;        if (distance[j] < 0 && m->cells[j] == visited) {distance[j] = d; stack[count++] = j; }}
        if (y     > 0        ) {int j = i - m->width; if (distance[j] < 0 && m->cells[j] == visited) {distance[j] = d; stack[count++] = j; }}
        if (y + 1 < m->height) {int j = i + m->width; if (distance[j] < 0 && m->cells[j] == visited) {distance[j] = d; stack[count++] = j; }}
    }

    int i = a + m->width * b;
    int d = distance[i];

    Maze path;
    path.width = m->width;
    path.height = m->height;
    maze_fill_with(&path, unvisited);
    
    for (int iter = 0; d > 1 && iter < 10000; ++iter) {
        int x = i % m->width;
        int y = i / m->width;
        maze_cell_set(&path, x, y, visited);
        // if neighbourings has shorter distance 
        // move there temporary
        int tmp = i;
        if (x     > 0        ) {int j = i - 1;        if (distance[j] < d && m->cells[j] == visited) {d = distance[i - 1];        tmp = j; }}
        if (x + 1 < m->width ) {int j = i + 1;        if (distance[j] < d && m->cells[j] == visited) {d = distance[i + 1];        tmp = j; }}
        if (y     > 0        ) {int j = i - m->width; if (distance[j] < d && m->cells[j] == visited) {d = distance[i - m->width]; tmp = j; }}
        if (y + 1 < m->height) {int j = i + m->width; if (distance[j] < d && m->cells[j] == visited) {d = distance[i + m->width]; tmp = j; }}
        i = tmp;
    }
    maze_cell_set(&path, i % m->width, i / m->width, visited);
    maze_print_cli(&path);
}

int main() {
    Maze maze;
    maze.width = 32;
    maze.height = 16;

    maze_fill_with(&maze, unvisited);
    maze_generation(&maze);
    maze_print_cli(&maze);

    printf("\n");

    maze_path_find(&maze, 0, 0, 31, 15);

    printf("done\n");

    return 0;
}