/**
 * @file scrabbleServer.c scrabble server program that does server client connection and allows clients to send in commands to play scrabble
 * the client can either view board, put a word across , put a word down, or quit out of the client connection
 * @author Pratik Bairoliya pbairol@ncsu.edu
 * @date 2022-11-17
 * HW 5 Part 3
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <errno.h>

/**
 * The struct that holds the actual 2 dimensional board and the row and column size
 *
 */
typedef struct
{
  char **board;
  int r;
  int c;
} ScrabbleBoard;
// global scrabble board that will be updated when a correct command for across and down has been there
ScrabbleBoard *scrabbleBoard;

// mutual exclusion moniotr
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * This is another custom function which does  ATOI and that takes in string that we expect to have character digits.
 * If the string cannot be parsed this function returns -1 otherwise it returns the parsed version of the int
 * I wrote this function due to the fact that we were not allowed to use c standard library hence why this function was made
 * @param nums is another char pointer string that we try to parse and convert into an integer
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

/** The initial capcity of the line string*/
#define INITIALCAPCITY 4

/** The double value that we will use to increase capacity of the database */
#define DOUBLER 2

/**
 * This method reads a line from the file and returns it back as a string - My program from 230
 * @param fp the file that we will be reading a line from
 * @return char* the type that is returned is a string taht is a line from the file
 */
char *readLine(FILE *fp)
{
  int capacity = INITIALCAPCITY;
  int len = 0;
  char *line = (char *)malloc(capacity * sizeof(char) + 1);
  char character = '\0';
  int x;
  x = fscanf(fp, "%c", &character);
  while (character != '\n' && x != EOF)
  {
    if (len >= capacity)
    {
      capacity = capacity * DOUBLER;
      line = (char *)realloc(line, capacity * sizeof(char) + 1);
    }
    line[len++] = character;
    x = fscanf(fp, "%c", &character);
  }
  if (x == EOF)
  {
    free(line);
    return NULL;
  }
  line[len++] = '\0';
  return line;
}
/**
 * @brief fills the Scrabble board struct passed in with the correct dimension
 *
 * @param r the nubmer of rows
 * @param c the number of columns
 * @param scrabble  the struct itself that we are filling
 */
void fillBoard(int r, int c, ScrabbleBoard *scrabble)
{

  scrabble->r = r;
  scrabble->c = c;

  scrabble->board = (char **)malloc(r * sizeof(char *));
  for (int i = 0; i < r; i++)
  {
    scrabble->board[i] = (char *)malloc(c * sizeof(char));
  }

  for (int i = 0; i < r; i++)
  {
    for (int j = 0; j < c; j++)
    {
      scrabble->board[i][j] = ' ';
    }
  }
}
/**
 * @brief freeing the memory that was allocated in fillBaord
 *
 * @param scrabble the struct that we are freeing
 */
void freeScrabble(ScrabbleBoard *scrabble)
{
  for (int i = 0; i < scrabble->r; i++)
  {
    free(scrabble->board[i]);
  }
  free(scrabble->board);
  free(scrabble);
  pthread_mutex_destroy(&lock);
}
/**
 * @brief method that prints out the struct with the boarder of + and -
 *
 * @param fp the file pointer we are printing to
 * @param scrabble the struct we are trying to print
 */
void viewBoard(FILE *fp, ScrabbleBoard *scrabble)
{
  fprintf(fp, "+");
  for (int i = 0; i < scrabble->c; i++)
  {
    fprintf(fp, "-");
  }
  fprintf(fp, "+\n");
  for (int i = 0; i < scrabble->r; i++)
  {
    fprintf(fp, "|");
    for (int j = 0; j < scrabble->c; j++)
    {

      fprintf(fp, "%c", scrabble->board[i][j]);
    }
    fprintf(fp, "|\n");
  }
  fprintf(fp, "+");
  for (int i = 0; i < scrabble->c; i++)
  {
    fprintf(fp, "-");
  }

  fprintf(fp, "+\n");
}

/**
 * @brief extra credit method that checks if the word passed in a word in the wrods list
 *
 * @param word the string that we are checking if it is in the word list
 * @return bool the true or false if it is in the word list
 */
bool checkWord(char *word)
{

  for (int i = 0; i < strlen(word); i++)
  {
    if (word[i] < 97 || word[i] > 122)
    {
      return false;
    }
  }

  FILE *wordFile;
  wordFile = fopen("words", "r+");
  char temp[512] = {0};
  bool valid = false;
  while (fscanf(wordFile, "%s\n", temp) == 1)
  {
    if ((strcmp(temp, word)) == 0)
    {
      valid = true;
    }
  }
  fclose(wordFile);
  return valid;
}

/**
 * @brief the method that puts the word across and if it cannot returns false
 *
 * @param scrabble the struct that we are putting the word across
 * @param r the row position we want to put the word in
 * @param c the column position we want to put the word in
 * @param word the word itself we are trying to put in
 * @return bool true or false boolean if it can put word across
 */
bool putAcross(ScrabbleBoard *scrabble, int r, int c, char *word)
{
  pthread_mutex_lock(&lock);
  int length = strlen(word);
  if (c + length > scrabble->c)
  {
    pthread_mutex_unlock(&lock);
    return false;
  }
  int count = 0;
  for (int i = c; i < c + length; i++)
  {
    if (scrabble->board[r][i] != ' ')
    {
      if (scrabble->board[r][i] != word[count])
      {
        pthread_mutex_unlock(&lock);
        return false;
      }
    }
    count++;
  }
  count = 0;
  for (int i = c; i < c + length; i++)
  {
    scrabble->board[r][i] = word[count++];
  }
  pthread_mutex_unlock(&lock);
  return true;
}

/**
 * @brief the method that puts the word down and if it cannot returns false
 *
 * @param scrabble the struct that we are putting the word down
 * @param r the row position we want to put the word in
 * @param c the column position we want to put the word in
 * @param word the word itself we are trying to put in
 * @return bool true or false boolean if it can put word down
 */
bool putDown(ScrabbleBoard *scrabble, int r, int c, char *word)
{
  pthread_mutex_lock(&lock);
  int length = strlen(word);
  if (r + length > scrabble->r)
  {
    pthread_mutex_unlock(&lock);
    return false;
  }
  int count = 0;
  for (int i = r; i < r + length; i++)
  {
    if (scrabble->board[i][c] != ' ')
    {
      if (scrabble->board[i][c] != word[count])
      {
        pthread_mutex_unlock(&lock);
        return false;
      }
    }
    pthread_mutex_unlock(&lock);
    count++;
  }
  count = 0;
  for (int i = r; i < r + length; i++)
  {
    scrabble->board[i][c] = word[count++];
  }
  return true;
}

/**
 * @brief this method does the extra credit handeling if words added are correct both horizontally and vertically it also calls
 * the mehtods for putacross and putdown it does majority of the scrabble game functionality
 * This method creates a temprorary board updates it with the putAcross and putDown method and sees if hte board is still correct and then copies over to the global board
 * @param r the row position we are trying to put across or down depending on the direction
 * @param c the column position we are trying to put across or down depending on the direction
 * @param word the word itself we are checking to put in
 * @param direction the direciton we are trying to put the word in
 * @return bool returns a bool either true or false if the word can be put in the board properly
 */
bool checkBoardLine(int r, int c, char *word, char *direction)
{
  ScrabbleBoard *tempScrabble = (ScrabbleBoard *)malloc(sizeof(ScrabbleBoard));
  int tempR = scrabbleBoard->r;
  int tempC = scrabbleBoard->c;
  fillBoard(tempR, tempC, tempScrabble);

  for (int i = 0; i < tempR; i++)
  {
    for (int j = 0; j < tempC; j++)
    {
      tempScrabble->board[i][j] = scrabbleBoard->board[i][j];
    }
  }

  if (strcmp(direction, "across") == 0)
  {
    if (!putAcross(tempScrabble, r, c, word))
    {
      free(tempScrabble);
      return false;
    }
  }
  else if (strcmp(direction, "down") == 0)
  {
    if (!putDown(tempScrabble, r, c, word))
    {
      free(tempScrabble);
      return false;
    }
  }

  char horizontalCheck[tempC + 1];
  char verticalCheck[tempR + 1];
  for (int i = 0; i < tempC; i++)
  {
    horizontalCheck[i] = tempScrabble->board[r][i];
  }
  for (int i = 0; i < tempR; i++)
  {
    verticalCheck[i] = tempScrabble->board[i][c];
  }
  verticalCheck[tempR] = '\0';
  horizontalCheck[tempC] = '\0';

  char *wordInLine = strtok(horizontalCheck, " ");

  while (wordInLine != NULL)
  {

    if (!checkWord(wordInLine))
    {
      freeScrabble(tempScrabble);
      return false;
    }
    wordInLine = strtok(NULL, " ");
  }
  wordInLine = strtok(verticalCheck, " ");

  while (wordInLine != NULL)
  {

    if (!checkWord(wordInLine))
    {
      freeScrabble(tempScrabble);
      return false;
    }
    wordInLine = strtok(NULL, " ");
  }
  for (int i = 0; i < tempR; i++)
  {
    for (int j = 0; j < tempC; j++)
    {
      scrabbleBoard->board[i][j] = tempScrabble->board[i][j];
    }
  }

  freeScrabble(tempScrabble);
  return true;
}
/** Port number used by my server */
#define PORT_NUMBER "26306"

/** Maximum word length */
#define WORD_LIMIT 26

// Print out an error message and exit.
static void fail(char const *message)
{
  fprintf(stderr, "%s\n", message);
  exit(EXIT_FAILURE);
}

// Print out a usage message, then exit.
static void usage()
{
  printf("usage: scrabbleServer <rows> <cols>\n");
  exit(1);
}

/** handle a client connection, close it when we're done. */
void *handleClient(int sock)
{
  // Here's a nice trick, wrap a C standard IO FILE around the
  // socket, so we can communicate the same way we would read/write
  // a file.
  FILE *fp = fdopen(sock, "a+");

  // Prompt the user for a command.
  fprintf(fp, "cmd> ");

  // Array for parsing commands has one extra space to check if buffer overflow.
  char cmd[WORD_LIMIT + 1];

  while (true)
  {
    char *buffLine = readLine(fp);

    char excess[WORD_LIMIT + 1];
    int match = sscanf(buffLine, "%27s ", cmd);

    if (match != 1)
    {
      fprintf(fp, "%s\n", "Invalid hi command");
      fprintf(fp, "cmd> ");
      continue;
    }

    else if (strlen(cmd) > WORD_LIMIT)
    {
      fprintf(fp, "%s\n", "Invalid command");
      fprintf(fp, "cmd> ");
      continue;
    }
    else if (match == 1 && strcmp(cmd, "quit") == 0)
    {
      break;
    }

    else if (match == 1 && strcmp(cmd, "board") == 0)
    {


      viewBoard(fp, scrabbleBoard);
    }

    else if (strcmp(cmd, "across") == 0 || strcmp(cmd, "down") == 0)
    {
      int c = 0;
      int r = 0;
      char word[WORD_LIMIT + 1];
      char excessWord[WORD_LIMIT + 1];

      int match2 = sscanf(buffLine, "%d %d %27s ", &r, &c, word );

      if (match2 != 3)
      {
        fprintf(fp, "%s\n", "Invalid command");
        fprintf(fp, "cmd> ");
        continue;
      }
      if (strlen(word) > WORD_LIMIT)
      {
        fprintf(fp, "%s\n", "Invalid command");
        fprintf(fp, "cmd> ");
        continue;
      }

      if (c < 0 || r < 0 || c >= scrabbleBoard->c || r >= scrabbleBoard->r)
      {

        fprintf(fp, "%s\n", "Invalid hasdf command");
        fprintf(fp, "cmd> ");
        continue;
      }

      if (!checkBoardLine(r, c, word, cmd))
      {
        fprintf(fp, "%s\n", "Invalid asdf command");
        fprintf(fp, "cmd> ");
        continue;
      }
    }
    else
    {
      fprintf(fp, "%s\n", "Invalid asddf command");
      fprintf(fp, "cmd> ");
      continue;
    }
    // Prompt the user for the next command.
    fprintf(fp, "cmd> ");
  }

  // Close the connection with this client.
  fclose(fp);
  return NULL;
}
/**
 * @brief thread runnable that each client thread will run this runnable calls handleClient
 *
 * @param param the socket number as void pointer
 * @return void*  returns the functionpointer
 */
void *threadRoutine(void *param)
{
  int sock = *((int *)param);
  handleClient(sock);
  return NULL;
}

/**
 * @brief main server method iniatilizes the server client connection then calls handle client which does majority of hte scrabble
 * fucntionality like put across put down and view board.
 * @param argc  the number of arguemnts passed in
 * @param argv  the arguments itself in a string array
 * @return int exit status of this program
 */
int main(int argc, char *argv[])
{

  if (argc != 3)
  {
    usage();
  }

  int r = customATOI(argv[1]);
  int c = customATOI(argv[2]);
  if (r <= 0 || c <= 0)
  {
    usage();
  }
  scrabbleBoard = (ScrabbleBoard *)malloc(sizeof(ScrabbleBoard));
  fillBoard(r, c, scrabbleBoard);

  // Prepare a description of server address criteria.
  struct addrinfo addrCriteria;
  memset(&addrCriteria, 0, sizeof(addrCriteria));
  addrCriteria.ai_family = AF_INET;
  addrCriteria.ai_flags = AI_PASSIVE;
  addrCriteria.ai_socktype = SOCK_STREAM;
  addrCriteria.ai_protocol = IPPROTO_TCP;

  // Lookup a list of matching addresses
  struct addrinfo *servAddr;
  if (getaddrinfo(NULL, PORT_NUMBER, &addrCriteria, &servAddr))
    fail("Can't get address info");

  // Try to just use the first one.
  if (servAddr == NULL)
    fail("Can't get address");

  // Create a TCP socket
  int servSock = socket(servAddr->ai_family, servAddr->ai_socktype,
                        servAddr->ai_protocol);
  if (servSock < 0)
    fail("Can't create socket");

  // Bind to the local address
  if (bind(servSock, servAddr->ai_addr, servAddr->ai_addrlen) != 0)
    fail("Can't bind socket");

  // Tell the socket to listen for incoming connections.
  if (listen(servSock, 5) != 0)
    fail("Can't listen on socket");

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(servAddr);

  // Fields for accepting a client connection.
  struct sockaddr_storage clntAddr; // Client address
  socklen_t clntAddrLen = sizeof(clntAddr);

  while (true)
  {
    // Accept a client connection.
    int sock = accept(servSock, (struct sockaddr *)&clntAddr, &clntAddrLen);
    pthread_t tempThread;
    int *sockPointer = &sock;

    pthread_create(&tempThread, NULL, threadRoutine, sockPointer);
    pthread_detach(tempThread);
  }

  freeScrabble(scrabbleBoard);
  // Stop accepting client connections (never reached).
  close(servSock);

  return 0;
}
