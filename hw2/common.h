// Height and width of the playing area.
#define GRID_SIZE 5

#define PATH_NAME "/afs/unity.ncsu.edu/users/p/pbairol"

#define UNIQUE_ID 64

/**
 * Struct Gamestate
 * This is the struct that is accessed by both reset.c and lightsout.c
 * It holds the 2d board with the chars * and . 
 * It also holds the x and y position values of the last move
 * It also holds the boolean condition if undo is valid or not
 */
typedef struct
{
  char board[GRID_SIZE][GRID_SIZE];
  bool undo;
  // if it is a move operation these values will change to the corersponding x and y coordinates
  int x;
  int y;
} GameState;