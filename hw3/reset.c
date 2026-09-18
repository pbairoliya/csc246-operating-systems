/**
 * @file reset.c This initalizes the game board and makes sure that lihghts out is added to the shared memory. It also initalizes
 * the named semaphore it also utalizes a semaphore when it reads in the file.
 * It fills in the board given the file name the user provides the program also provides hte error checking of all of that.
 * @author Pratik Bairoliya
 * @date 2022-10-12
 *
 * CSC 246 HW3
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
#include <semaphore.h>
#include <sys/syscall.h>

// named semaphore that both lights out and reset utalize to provide mutual exclusion, and solves the critical section problem
sem_t *semLock;
// Print out an error message and exit.
static void fail(char const *message)
{
  fprintf(stderr, "%s\n", message);
  exit(1);
}

// Print out a usage message and exit.
static void usage()
{
  fprintf(stderr, "usage: reset <board-file>\n");
  exit(1);
}

/**
 * This function takes in the file name the user provides and fills in the shared memory struct Lightout.
 * It makes sure that the file is not invalid as well.
 *
 * @param filename the string that provides the path to the file name
 * @param lighout the struct itself we will be iniatlizing.
 */
static void fillGameBoard(char *filename, GameState *lighout)
{

  // opening the file that the user inputs
  FILE *fp = fopen(filename, "r");
  // if it cannot open the file print to standard error incorrect file and exit with status 1;
  if (fp == NULL)
  {
    char failMessage[] = "Invalid input file: ";
    strcat(failMessage, filename);
    fail(failMessage);
  }

  int c = 0;
  int fileCols = 0;
  int fileRows = 0;
  bool validFile = true;
  bool firstRow = true;
  c = getc(fp);
#ifndef UNSAFE
  sem_wait(semLock);
#endif
  // File checking for invalid Chars and then claiming if the file is valid
  while (c != EOF)
  {
    if (c == '\n')
    {
      firstRow = false;
      fileRows++;
    }
    if ((c == '*' || c == '.') && firstRow)
    {
      fileCols++;
    }
    else if (c != '*' && c != '.' && c != '\n' && c != EOF)
    {
      validFile = false;
    }
    c = getc(fp);
  }

  // File checking for invalid dimesnion
  if (fileCols != 5 || fileRows != 5)
  {
    validFile = false;
  }

  // closes off the file and goes to stderror and prints invalid file message.
  if (!validFile)
  {
    fclose(fp);
    char failMessage[] = "Invalid input file: ";
    strcat(failMessage, filename);
    sem_post(semLock);
    fail(failMessage);
  }
  // starts the file stream from the beggining of the file
  rewind(fp);

  for (int i = 0; i < fileRows; i++)
  {
    for (int j = 0; j < fileCols; j++)
    {

      c = getc(fp);
      if (c == '\n')
      {
        c = getc(fp);
      }
      lighout->board[i][j] = c;
    }

  }
  // close file as no more reading in needs to be done
  fclose(fp);
  
  lighout->undo = false;
  sem_post(semLock);
}

/**
 * the main function that checks the arugments provided in and sees if that is valid or not
 * It also intalizes the shared memory and checks if it can be created and attatched or not.
 *
 * @param argc  the number of arugments provided by the main function.
 * @param argv  the arugments itself in a string array,
 * @return int  the exit status
 */
int main(int argc, char *argv[])
{
  // if the number of arguments is too many or too less print the following error message and exit with a status of 1;
  if (argc != 2)
  {
    usage();
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
  // more error cehcking
  if (lightout == (GameState *)-1)
  {
    fail("Can't map shared memory segment into address space");
  }
  sem_unlink(SEMAPHORE_NAME);
  semLock = sem_open(SEMAPHORE_NAME, O_CREAT, 0600, 1);

  fillGameBoard(argv[1], lightout);
  shmdt(lightout);
  sem_close(semLock);
  return 0;
}
