/**
 * @file lightsout.c the c file that runs after reset.c it takes in user input and allows them to move undo and report. 
 * this file provides the funcitonality and erorr checking of all of that.
 * @author Pratik Bairoliya
 * @date 2022-09-29
 *
 * CSC 246 HW2
 *
 */
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include <string.h>
#include "common.h"

// Print out an error message and exit.
static void fail(char const *message)
{
  fprintf(stderr, "%s\n", message);
  exit(1);
}

/**
 * This is another custom function which does  ATOI and that takes in string that we expect to have character digits.
 * If the string cannot be parsed this function returns -1 otherwise it returns the parsed version of the int
 * I wrote this function due to the fact that we were not allowed to use c standard library hence why this function was made
 * @param nums is anotehr char pointer string that we try to parse and convert into an integer
 * @return int  the converted integer value if the string can be parsed is returned or if cannot -1 is returned
 */
int customATOI(char *nums)
{
  int numLength = strlen(nums);
  int number = 0;
  for (int i = 0; i < numLength; i++)
  {
    number *= 10;

    int digit = nums[i] - 48;
    if (digit > 9 || digit < 0)
    {
      return -1;
    }
    number += digit;
  }
  return number;
}

/**
 * Reports out the current status of the lights out program, this is accessing the shared memroy which has the struct GameState
 * @param lightout the game itself that contains the board that we are reporting out. lightout is a type of struct
 */
static void report(GameState *lightout)
{
  for (int i = 0; i < GRID_SIZE; i++)
  {
    for (int j = 0; j < GRID_SIZE; j++)
    {
      printf("%c", lightout->board[i][j]);
    }
    printf("\n");
  }
}

/**
 * Method that changes the state of the lights out game it turns on and off the lights in the 2d board depending on the coordiantes passed in x and y
 * @param lightout The struct itself that contains the board and the coordinates of the last move
 * @param x the x cooridnate the user provides
 * @param y the y coordinate the user provides
 */
static void move(GameState *lightout, int x, int y)
{
  if (x < 0 || y < 0 || x >= 5 || y >= 5)
  {
    printf("%s\n", "error");
    exit(1);
  }
  lightout->x = x;
  lightout->y = y;
  lightout->undo = true;

  if (x >= 0 && y >= 0 && x < 5 && y < 5)
  {
    if (lightout->board[x][y] == '.')
    {
      lightout->board[x][y] = '*';
    }
    else
    {
      lightout->board[x][y] = '.';
    }
  }
  if ((x + 1) >= 0 && y >= 0 && (x + 1) < 5 && y < 5)
  {
    if (lightout->board[(x + 1)][y] == '.')
    {
      lightout->board[(x + 1)][y] = '*';
    }
    else
    {
      lightout->board[(x + 1)][y] = '.';
    }
  }
  if ((x - 1) >= 0 && y >= 0 && (x - 1) < 5 && y < 5)
  {
    if (lightout->board[(x - 1)][y] == '.')
    {
      lightout->board[(x - 1)][y] = '*';
    }
    else
    {
      lightout->board[(x - 1)][y] = '.';
    }
  }
  if (x >= 0 && (y + 1) >= 0 && x < 5 && (y + 1) < 5)
  {
    if (lightout->board[x][(y + 1)] == '.')
    {
      lightout->board[x][(y + 1)] = '*';
    }
    else
    {
      lightout->board[x][(y + 1)] = '.';
    }
  }
  if (x >= 0 && (y - 1) >= 0 && x < 5 && (y - 1) < 5)
  {
    if (lightout->board[x][(y - 1)] == '.')
    {
      lightout->board[x][(y - 1)] = '*';
    }
    else
    {
      lightout->board[(x)][(y - 1)] = '.';
    }
  }
  printf("%s\n", "success");
}

/**
 * This is the last type of move that the user can do and it is undo, it accesses the last move that the user did granted it is not an undo itself
 * There is a check in the light out struct to make sure the User can only do one undo after a move and no more
 * @param lightout this the struct that is passed in which we access the board and undo the last move the user has done.
 */
static void undo(GameState *lightout)
{
  move(lightout, lightout->x, lightout->y);
  lightout->undo = false;
}

/**
 * this is the int main and controlls the entirety of the game it continues to take in input making sure if it is valid or not and exits when ^C is entered by the user
 * @param argc the number of arguments being passed in
 * @param argv  the arugments itself in an array of strings
 * @return int
 */
int main(int argc, char *argv[])
{
  /*Checks if there are too many arugments which is invlaid*/
  if (argc < 2 || argc > 4)
  {
    printf("%s\n", "error");
    exit(1);
  }

  GameState *lightout;
  int key = ftok(PATH_NAME, UNIQUE_ID);
  int gameId = shmget(key, sizeof(GameState), IPC_CREAT | 0600);

  // error checking
  if (gameId == -1)
  {
    fail("Can't create shared memory");
  }

  lightout = (GameState *)shmat(gameId, NULL, 0);

  // Error checking if lightout cant be attatched to memory
  if (lightout == (GameState *)-1)
  {
    fail("Can't map shared memory segment into address space");
  }
  // report command  method
  if (strcmp(argv[1], "report") == 0)
  {
    if (argc != 2)
    {
      printf("%s\n", "error");
      exit(1);
    }
    report(lightout);
  }
  // move command method
  else if (strcmp(argv[1], "move") == 0)
  {
    if (argc != 4)
    {
      printf("%s\n", "error");
      exit(1);
    }
    int x = customATOI(argv[2]);
    int y = customATOI(argv[3]);

    move(lightout, x, y);
  }
  // undo command method
  else if (strcmp(argv[1], "undo") == 0)
  {
    if (argc != 2)
    {
      printf("%s\n", "error");
      exit(1);
    }
    if (!(lightout->undo))
    {
      printf("%s\n", "error");
      exit(1);
    }
    undo(lightout);
  }

  // not a valid command
  else
  {
    printf("%s\n", "error");
    exit(1);
  }
  // detaches the struct from shared memory
  shmdt(lightout);
  return EXIT_SUCCESS;
}
