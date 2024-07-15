#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
//GLOBAL PROGRAM SETTINGS
#define CLIFE_DBG       //Controls whether or not the program should have debug output.
#define TICK_RATE 100   //Controls the tick-rate of the game. NOTE: this will eventually be controllable.


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

//The main cell structure. Contains state, direction, and a pointer to the next cell (this is a linked list)
typedef struct cell_t{
  cell_state_e state; //The current state of this cell.
  int pos_x, pos_y;   //A cache of this cell's position in the grid.
} cell_t;

//The entire app's state.
typedef struct {
  struct grid_t {
    cell_t* cells;    //The linked list of cells.
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
  printf("\tState: %d\n", cell.state);
  printf("\tXpos: %d\n", cell.pos_x);
  printf("\tYpos: %d\n", cell.pos_y);
}

//DEBUG: prints the cell grid to the terminal.
void dbg_print_grid(state_t* state) {
  for (int i = 0; i < state->grid.size_x; i++) {
    for (int j = 0; j < state->grid.size_y; j++) {
      cell_t cell = state->grid.cells[j * state->grid.size_x + i];
      switch (cell.state) {
        case STATE_DEAD: putchar(' ');
            break;
        case STATE_ALIVE: putchar('@');
            break;
      }
    }
    putchar('\n');
  }
}
#endif

//Initializes the cell linked-list and grid, allocating it on the heap and giving a pointer to the first one. 
void init_grid(state_t* state, size_t size_x, size_t size_y) {
  cell_t* cell_alloc = (cell_t*)malloc(sizeof(cell_t) * size_x * size_y);

  if (cell_alloc == NULL) {
    perror("clife: failed to allocate space for cell grid");
    exit(1);
  }
  memset((void*)cell_alloc, 0, sizeof(cell_t) * size_x * size_y);  
  //Safely have the head of the list - time to go
  for (int i = 0; i < size_x * size_y; i++) {
    //printf("On cell index: %d, coords: (%d, %d)", i, i % size_x, i % size_y);
    cell_t cell = (cell_t) {
      STATE_DEAD,
      i % size_x,
      i / size_y
    };
    cell_alloc[i] = cell;
  }
  //Finished initalizing it, time to give it back to the state
  state->grid.cells = cell_alloc;
  state->grid.num_cells = size_x * size_y;
  state->grid.size_x = size_x;
  state->grid.size_y = size_y;
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
}

//Called to render to the terminal.
void render(state_t* state) {
  printf("Render!\n");
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
}


int main(int argc, char** argv) {
  state_t state = initialize((size_t)8, (size_t)8);
  printf("We initialized!");
  //make funni pattern
  for(int i = 0; i < state.grid.size_x * state.grid.size_y; i++) {
    cell_t* cell = &state.grid.cells[i];
    cell->state = ((cell->pos_x * cell->pos_y) % 2 == 0) ? STATE_ALIVE : STATE_DEAD;
  }
  printf("grid vis:\n");
  #ifdef CLIFE_DBG
  dbg_print_grid(&state);
  #endif
  while(1) {
    update(&state);

  }
  clean_up(&state);
  return 0;
}
