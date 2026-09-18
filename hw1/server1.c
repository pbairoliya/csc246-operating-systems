#include "common.h"
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <signal.h>
#include <errno.h>
#include <string.h>


// Print out an error message and exit.
static void fail(char const *message)
{
  fprintf(stderr, "%s\n", message);
  exit(1);
}

// Flag for telling the server to stop running because of a sigint.
// This is safer than trying to print in the signal handler.
static int running = 1;

static void sighandler(int sgn)
{
  running = 0;
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
 * This is a struct that holds the Game board characteristics: the grid itself with alll the lights on and off represented by . and *
 * It also contains the state which is a string either the previous move was a move or undo and this determines if undo can happen
 * It also contains the x and y move values of the current move when parsed in
 * 
 */
typedef struct
{
  char board[GRID_SIZE][GRID_SIZE];
  char state[MESSAGE_LIMIT]; // what move the user sends in
  // if it is a move operation these values will change to the corersponding x and y coordinates
  int x;
  int y;
} GameBoard;


/**
 * The main method that handles the manipulation of the board struct and returns error and success of the commands client sends in to server
 * It also sends messages and receives messages to and from the client.
 * @param argc the amount of args
 * @param argv the args itself
 * @return int success or failure
 */
int main(int argc, char *argv[])
{
  // Remove both queues, in case, last time, this program terminated
  // abnormally with some queued messages still queued.
  mq_unlink(SERVER_QUEUE);
  mq_unlink(CLIENT_QUEUE);

  // Prepare structure indicating maximum queue and message sizes.
  struct mq_attr attr;
  attr.mq_flags = 0;
  attr.mq_maxmsg = 1;
  attr.mq_msgsize = MESSAGE_LIMIT;

  // Make both the server and client message queues.
  mqd_t serverQueue = mq_open(SERVER_QUEUE, O_RDONLY | O_CREAT, 0600, &attr);
  mqd_t clientQueue = mq_open(CLIENT_QUEUE, O_WRONLY | O_CREAT, 0600, &attr);
  if (serverQueue == -1 || clientQueue == -1)
    fail("Can't create the needed message queues");

  if (argc != 2)
  {
    fail("usage: server <board-file>");
  }
  //opening the file that the user inputs
  FILE *fp = fopen(argv[1], "r");
  // if file cannot be opened fail the file and print out the message to standard error
  if (fp == NULL)
  {
    char failMessage[] = "Invalid input file: ";
    strcat(failMessage, argv[1]);
    fail(failMessage);
  }

  int c = 0;
  int fileCols = 0;
  int fileRows = 0;
  bool validFile = true;
  bool firstRow = true;
  c = getc(fp);
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
  //closes off the file and goes to stderror and prints invalid file message.
  if (!validFile)
  {
    fclose(fp);
    char failMessage[] = "Invalid input file: ";
    strcat(failMessage, argv[1]);
    fail(failMessage);
  }
  //starts the file stream from the beggining of the file
  rewind(fp);
  GameBoard b1;
  // reading in the gameboard by changing the values of the grid inside the struct Gameboard
  for (int i = 0; i < fileRows; i++)
  {
    for (int j = 0; j < fileCols; j++)
    {

      c = getc(fp);
      if (c == '\n')
      {
        c = getc(fp);
      }
      b1.board[i][j] = c;
    }
  }
  //close file as no more reading in needs to be done
  fclose(fp);
  // Copies over the ready state which means the first move the client sends in cant be undo and this condition is under the undo if statement
  strcpy(b1.state, "ready");

  struct sigaction act;

  // Fill in a structure to redirect the alarm signal.
  act.sa_handler = sighandler;
  sigemptyset( &( act.sa_mask ) );
  act.sa_flags = 0;
   //signal to quit when ^C is pressed
  sigaction( SIGINT, &act, 0 );
  while (running)
  {
   

    // creates a new clientMessage string holder every time client sends in a message and server is running
    char clientMessage[MESSAGE_LIMIT] ="";
    // Try to get a message (this will wait until one arrives).
    int len = mq_receive(serverQueue, clientMessage, sizeof(clientMessage), NULL);
    //checks if the client message acc sends back a message then conducts all other functionalities
    if (len >= 0)
    {
      //functionality if client sends in a report command
      if (strcmp(clientMessage, "report") == 0)
      {
        char temp[MESSAGE_LIMIT] ="";
        int ctr = 0;
        for(int i =0;i<5;i++){
          for(int j = 0; j<5;j++){
            temp[ctr++] = b1.board[i][j];
          }
         
         
        }
        mq_send(clientQueue, temp, strlen(temp), 0);
      }
      //functioanlity if client sends in a move command
      if (strcmp(clientMessage, "move") == 0)
      {
        char x[MESSAGE_LIMIT+1] = "";
        char y[MESSAGE_LIMIT+1] = "";

        mq_receive(serverQueue, x, sizeof(x), NULL);
        mq_receive(serverQueue, y, sizeof(y), NULL);
 
        int xCoord = customATOI(x);
        int yCoord = customATOI(y);



        if (xCoord < 0 || yCoord < 0 || xCoord >= 5 || yCoord >= 5)
        {
          mq_send(clientQueue, "error", sizeof("error"), 0);
        }
        else
        {
          strcpy(b1.state, clientMessage);
          b1.x = xCoord;
          b1.y = yCoord;
          if (xCoord >= 0 && xCoord < 5 && yCoord >= 0 && (yCoord) < 5)
          {
            if (b1.board[xCoord][yCoord] == '.')
            {
              b1.board[xCoord][yCoord] = '*';
            }
            else
            {
              b1.board[xCoord][yCoord] = '.';
            }
          }
          if ((xCoord - 1) >= 0 && (xCoord - 1) < 5 && yCoord >= 0 && (yCoord) < 5)
          {
            if (b1.board[xCoord - 1][yCoord] == '.')
            {
              b1.board[xCoord - 1][yCoord] = '*';
            }
            else
            {
              b1.board[xCoord - 1][yCoord] = '.';
            }
          }
          if ((xCoord + 1) >= 0 && (xCoord + 1) < 5 && yCoord >= 0 && (yCoord) < 5)
          {
            if (b1.board[xCoord + 1][yCoord] == '.')
            {
              b1.board[xCoord + 1][yCoord] = '*';
            }
            else
            {
              b1.board[xCoord + 1][yCoord] = '.';
            }
          }
          if ((xCoord) >= 0 && (xCoord) < 5 && (yCoord + 1) >= 0 && (yCoord + 1) < 5)
          {
            if (b1.board[xCoord][yCoord + 1] == '.')
            {
              b1.board[xCoord][yCoord + 1] = '*';
            }
            else
            {
              b1.board[xCoord][yCoord + 1] = '.';
            }
          }
          if ((xCoord) >= 0 && (xCoord) < 5 && (yCoord - 1) >= 0 && (yCoord - 1) < 5)
          {
            if (b1.board[xCoord][yCoord - 1] == '.')
            {
              b1.board[xCoord][yCoord - 1] = '*';
            }
            else
            {
              b1.board[xCoord][yCoord - 1] = '.';
            }
          }
           mq_send(clientQueue, "success", sizeof("success"), 0);
        }
       
      }
      //functionality if the client sends in an undo command      
      if (strcmp(clientMessage, "undo") == 0)
      {
        if (strcmp(b1.state, "move") != 0)
        {
          mq_send(clientQueue, "error", sizeof("error"), 0);
        }
        else
        {
          strcpy(b1.state, "undo");
          int xCoord = b1.x;
          int yCoord = b1.y;
          if (xCoord >= 0 && xCoord < 5 && yCoord >= 0 && (yCoord) < 5)
          {
            if (b1.board[xCoord][yCoord] == '.')
            {
              b1.board[xCoord][yCoord] = '*';
            }
            else
            {
              b1.board[xCoord][yCoord] = '.';
            }
          }
          if ((xCoord - 1) >= 0 && (xCoord - 1) < 5 && yCoord >= 0 && (yCoord) < 5)
          {
            if (b1.board[xCoord - 1][yCoord] == '.')
            {
              b1.board[xCoord - 1][yCoord] = '*';
            }
            else
            {
              b1.board[xCoord - 1][yCoord] = '.';
            }
          }
          if ((xCoord + 1) >= 0 && (xCoord + 1) < 5 && yCoord >= 0 && (yCoord) < 5)
          {
            if (b1.board[xCoord + 1][yCoord] == '.')
            {
              b1.board[xCoord + 1][yCoord] = '*';
            }
            else
            {
              b1.board[xCoord + 1][yCoord] = '.';
            }
          }
          if ((xCoord) >= 0 && (xCoord) < 5 && (yCoord + 1) >= 0 && (yCoord + 1) < 5)
          {
            if (b1.board[xCoord][yCoord + 1] == '.')
            {
              b1.board[xCoord][yCoord + 1] = '*';
            }
            else
            {
              b1.board[xCoord][yCoord + 1] = '.';
            }
          }
          if ((xCoord) >= 0 && (xCoord) < 5 && (yCoord - 1) >= 0 && (yCoord - 1) < 5)
          {
            if (b1.board[xCoord][yCoord - 1] == '.')
            {
              b1.board[xCoord][yCoord - 1] = '*';
            }
            else
            {
              b1.board[xCoord][yCoord - 1] = '.';
            }
          }
           mq_send(clientQueue, "success", sizeof("success"), 0);
        }
       
      }
      
    }
  }
  // after the server quits  it prints the final state of the game board.
  if (running==0)
  {
   printf("\n"); 
    for (int i = 0; i < fileRows; i++)
    {
      for (int j = 0; j < fileCols; j++)
      {
        printf("%c", b1.board[i][j]);
      }
      printf("\n");
    }
  }
  // Close our two message queues (and delete them).
  mq_close(clientQueue);
  mq_close(serverQueue);

  //unlink the two server queue names as well
  mq_unlink(SERVER_QUEUE);
  mq_unlink(CLIENT_QUEUE);

  return EXIT_SUCCESS;
}