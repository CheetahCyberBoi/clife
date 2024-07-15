#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//GLOBAL APP SETTINGS
//Defines whether or not Debugging should be enabled.
#define CLIFE_DBG


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
  struct cell_t* next;       //A pointer to the next cell in the linked list.
} cell_t;

//The entire app's state.
typedef struct {
  struct grid_t {
    cell_t* first_cell; //The linked list of cells.
    int num_cells;      //The total number of cells, equal to size_x * size_y.

    size_t size_x, size_y; //Dimensions of the grid.
  }grid;
} state_t;

// FUNCTIONS



//Adds the given cell to the cell linked list.
void add_to_cell_ll(cell_t* head, cell_state_e state, int x, int y) {
  cell_t* current = head;

  //go to the end of the cell linked-list
  while (current->next != NULL) {
    current = current->next;
  }

  //add a new cell on the end
  current->next = (cell_t*) malloc(sizeof(cell_t));
  current->next->state = state;
  current->next->pos_x = x;
  current->next->pos_y = y;
  current->next->next = NULL;
}

//Retrieves a COPY of a cell in a linked list.
//TODO: figure out how to get the actual cell.
cell_t* get_cell_ll(cell_t* head, int index) {
  cell_t* current = head;

  int count = 0;
  while (count != index && current->next != NULL) {
    current = current->next;
    count++;
  }

  return current;
}
#ifdef CLIFE_DBG
//DEBUG: Prints a cell to the terminal.
void dbg_print_cell(cell_t cell) {
  printf("Cell:\n");
  printf("\tState: %d\n", cell.state);
  printf("\tXpos: %d\n", cell.pos_x);
  printf("\tYpos: %d\n", cell.pos_y);
  printf("\tNext cell (pointer): %d\n", cell.next);
}

//DEBUG: prints the cell grid to the terminal.
void dbg_print_grid(state_t* state) {
  for (int i = 0; i < state->grid.size_x; i++) {
    for (int j = 0; j < state->grid.size_y; j++) {
      cell_t* cell = get_cell_ll(state->grid.first_cell, j*state->grid.size_y + i);
      if (cell == NULL) { fprintf(stderr, "clife: debug gridprint cell not found"); }
      switch (cell->state) {
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
  cell_t* cell_alloc = (cell_t*)malloc(sizeof(cell_t));

  if (cell_alloc == NULL) {
    perror("clife: failed to allocate space for cell grid");
    exit(1);
  }
  cell_alloc->state = STATE_DEAD;
  cell_alloc->pos_x = 0;
  cell_alloc->pos_y = 0;
  cell_alloc->next = NULL;
  
  //Safely have the head of the list - time to go
  for (int i = 0; i < size_x * size_y; i++) {
    //printf("On cell index: %d, coords: (%d, %d)", i, i % size_x, i % size_y);
    add_to_cell_ll(cell_alloc, STATE_DEAD, i % size_x, i / size_y);
  }
  //Finished initalizing it, time to give it back to the state
  state->grid.first_cell = cell_alloc;
  state->grid.num_cells = size_x * size_y;
  state->grid.size_x = size_x;
  state->grid.size_y = size_y;
}

//Destroys the cell linked list at the end of the program.
void destroy_cell_ll(state_t* state) {
  free(state->grid.first_cell);
}
//Initialize the program.
state_t initialize(size_t size_x, size_t size_y) {
  state_t state = (state_t){0};

  //Initialize the grid
  init_grid(&state, size_x, size_y);
  //for(cell_t* cell = state.grid.first_cell; cell != NULL; cell = cell->next) {
      //dbg_print_cell(*cell);    
  //}

  return state;
}

//Cleanup everything after we're done
void clean_up(state_t* state) {
  destroy_cell_ll(state);
}


int main(int argc, char** argv) {
  state_t state = initialize((size_t)8, (size_t)8);
  printf("We initialized!");
  //make funni pattern
  for(cell_t* cell = state.grid.first_cell; cell != NULL; cell = cell->next) {
    cell->state = ((cell->pos_x * cell->pos_y) % 2 == 0) ? STATE_ALIVE : STATE_DEAD;
  }
  printf("grid vis:\n");
  #ifdef CLIFE_DBG
  dbg_print_grid(&state);
  #endif
  printf("Hello World!!\n");
  clean_up(&state);
  return 0;
}
