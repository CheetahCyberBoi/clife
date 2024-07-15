#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
//GLOBAL PROGRAM SETTINGS
#define CLIFE_DBG       //Controls whether or not the program should have debug output.
#define TICK_RATE 1000   //Controls the tick-rate of the game. NOTE: this will eventually be controllable.


//A direction.
typedef enum {
  DIR_UP,
  DIR_RIGHT,
  DIR_DOWN,
  DIR_LEFT,
} dir_e;

//Cell state.
typedef enum {
  STATE_ALIVE,
  STATE_DEAD,
} cell_state_e;

#define cell_t bool

//The entire app's state.
typedef struct {
  struct grid_t {
    cell_t* cells;    //The list of cells.
    cell_t* swap;     //A copy of `cells`, used to enable all-at-once updating.
    int num_cells;         //The total number of cells, equal to size_x * size_y.

    size_t size_x, size_y; //Dimensions of the grid.
  }grid;
  unsigned long tick;      //The current tick of the game.
  struct timespec sleep_time;//The number of nanoseconds spent by the game sleeping. The tickrate minus this equals the total time spent processing by the game.

} state_t;

// FUNCTIONS




#ifdef CLIFE_DBG
//DEBUG: Prints a cell to the terminal.
void dbg_print_cell(cell_t cell) {
  printf("Cell:\n");
  printf("\tState: %d\n", cell);
  //printf("\tXpos: %d\n", cell.pos_x);
  //printf("\tYpos: %d\n", cell.pos_y);
}

//DEBUG: prints the cell grid to the terminal.
void dbg_print_grid(state_t* state) {
  for (int i = 0; i < state->grid.size_x; i++) {
    for (int j = 0; j < state->grid.size_y; j++) {
      cell_t cell = state->grid.cells[j * state->grid.size_x + i];
      if (cell) {
        putchar('@');
      } else {
        putchar(' ');
      }
    }
    putchar('\n');
  }
}
#endif

//Initializes the cell linked-list and grid, allocating it on the heap and giving a pointer to the first one. 
void init_grid(state_t* state, size_t size_x, size_t size_y) {
  //Acquire a zeroed out buffer of cells.
  cell_t* cell_alloc = (cell_t*)calloc(size_x * size_y, sizeof(cell_t));
  cell_t* swap_alloc = (cell_t*)calloc(size_x * size_y, sizeof(cell_t));

  if (cell_alloc == NULL || swap_alloc == NULL) {
    perror("clife: failed to allocate space for cell grid");
    exit(1);
  }

  //Safely have the head of the list - time to go
  for (int i = 0; i < size_x * size_y; i++) {
    cell_alloc[i] = false;
    swap_alloc[i] = false;
  }
  //Finished initalizing it, time to give it back to the state
  state->grid.cells = cell_alloc;
  state->grid.swap = swap_alloc;
  state->grid.num_cells = size_x * size_y;
  state->grid.size_x = size_x;
  state->grid.size_y = size_y;
}
cell_t get_cell(state_t* state, int x, int y) {
  return state->grid.cells[y * state->grid.size_x + x];
}
//Determines whether or not a cell will survive by counting up only the neighbours specified in the `neighbours_to_check` parameter.
void get_cell_neighbours(state_t* state, int cell_x, int cell_y, bool neighbours_to_check[8]) {
  cell_t cell = get_cell(state, cell_x, cell_y);
  //Stores the offsets in each direction from the cell.
  int neighbour_offsets_y[] = {
    -1,
    -1,
    -1,
    0,
    0,
    1,
    1, 
    1,
  };
  int neighbour_offsets_x[] = {
    -1,
    0,
    1, 
    -1, 
    1, 
    -1, 
    0, 
    1,
  };

  //Total number of alive neighbours around this cell.
  int alive_neighbours = 0;
  for (int i = 0; i < 8; i++) {
    if (neighbours_to_check[i]) {
      alive_neighbours += get_cell(state, neighbour_offsets_x[i], neighbour_offsets_y[i]);
    }
  }

  
  //THE CENTRAL RULE! This determines whether or not the cell will live based on its neighbourcount. Rules stolen from https://github.com/mrivnak/game-of-life/blob/community/c/src/game.c .
  int lives = (alive_neighbours == 3) | (cell && alive_neighbours == 2);
  //printf("(%d, %d): lives?: %d",cell_x, cell_y, lives);
  state->grid.swap[cell_y * state->grid.size_x + cell_x] = lives;
}
//Determines if a cell should be updated.
void update_cell(state_t* state, int x, int y) {
  //Top row
  get_cell_neighbours(state, 0, 0, (bool[]) {
    false, false, false,
    false,        true,
    false, true,  true,
  });
  for (int x = 1; x < state->grid.size_x - 1; x++) {
    get_cell_neighbours(state, x, 0, (bool[]) {
      false, false, false,
      true,         true,
      true,  true,  true,
    });
  }
  get_cell_neighbours(state, state->grid.size_x - 1, 0, (bool[]) {
    false, false, false,
    false,        true,
    false, true,  true,
  });
  //Middle row, and center
  for (int y = 1; y < state->grid.size_y - 1; y++) {
    get_cell_neighbours(state, 0, y, (bool[]) {
      false, true, true,
      false,       true,
      false, true, true,
    });
    for (int x = 1; x < state->grid.size_x - 1; x++) {
      get_cell_neighbours(state, x, y, (bool[]) {
        true, true, true,
        true,       true,
        true, true, true,
      });
    }
    get_cell_neighbours(state, state->grid.size_x - 1, y, (bool[]) {
      true, true, false,
      true,       false, 
      true, true, false,
    });
  }
  //Bottom row.
  get_cell_neighbours(state, 0, state->grid.size_y - 1, (bool[]) {
    false, true,  true,
    false,        true,
    false, false, false,
  });
  for(int x = 1; x < state->grid.size_x - 1; x++) {
    get_cell_neighbours(state, x, state->grid.size_y - 1, (bool[]) {
      true,   true,  true,
      true,          true,
      false, false, false,
    });
  }
  get_cell_neighbours(state, state->grid.size_x - 1, state->grid.size_y - 1, (bool[]) {
    true, true,   false,
    true,         false,
    false, false, false,
  });

  //swap the swap and main cellgrids
  cell_t* temp = state->grid.cells;
  state->grid.cells = state->grid.swap;
  state->grid.swap = temp;
}


//STATE-RELEATED FUNCTIONS

//Initialize the program.
state_t initialize(size_t size_x, size_t size_y) {
  state_t state = (state_t){0};

  //Initialize the grid
  init_grid(&state, size_x, size_y);
  //Initialize the timing parts of the app.
  state.sleep_time.tv_sec = 0;
  state.sleep_time.tv_nsec = TICK_RATE * 1000 * 1000; //Converts from milliseconds to nanoseconds (millisecond = 1M nanoseconds iirc)
  //for(cell_t* cell = state.grid.first_cell; cell != NULL; cell = cell->next) {
      //dbg_print_cell(*cell);    
  //}

  return state;
}

//Called once per tick.
void tick(state_t* state) {
  printf("Tick!\n");
  printf("Time to process: %d", TICK_RATE - state->sleep_time.tv_nsec / 1000000);
  for (int i = 0; i < state->grid.size_x * state->grid.size_y; i++) {
    update_cell(state, i % state->grid.size_x, i / state->grid.size_y);
  }
}

//Called to render to the terminal.
void render(state_t* state) {
  printf("Render!\n");
  dbg_print_grid(state);
}

//Called every frame. Handles game logic, rendering, and user input.
void update(state_t* state) {
  struct timespec start_time, end_time;
  clock_gettime(CLOCK_MONOTONIC, &start_time);

  //Update the game, and render.
  tick(state);
  render(state);

  //Sleep for the rest of the tick.
  clock_gettime(CLOCK_MONOTONIC, &end_time);
  long elapsed_time = (end_time.tv_sec - start_time.tv_sec) * 1000000000L + (end_time.tv_nsec - start_time.tv_nsec);

  //adjust time to sleep for based on the time already spent.
  long sleep_duration = TICK_RATE * 1000L * 1000L - elapsed_time;
  if (sleep_duration > 0) {
    state->sleep_time.tv_nsec = sleep_duration;
    nanosleep(&state->sleep_time, NULL);
  }
}

//Cleanup everything after we're done
void clean_up(state_t* state) {
  free(state->grid.cells);
  free(state->grid.swap);
}


int main(int argc, char** argv) {
  state_t state = initialize((size_t)24, (size_t)24);
  printf("We initialized!");
  
  state.grid.cells[2] = 1;
  state.grid.cells[state.grid.size_x] = 1;
  state.grid.cells[state.grid.size_x + 2] = 1;
  state.grid.cells[2 * state.grid.size_x + 1] = 1;
  state.grid.cells[2 * state.grid.size_x + 2] = 1;

  
  printf("grid vis:\n");
  #ifdef CLIFE_DBG
  dbg_print_grid(&state);
  #endif
  for(int i = 0; i < 1; i ++) {
    update(&state);

  }
  clean_up(&state);
  return 0;
}
