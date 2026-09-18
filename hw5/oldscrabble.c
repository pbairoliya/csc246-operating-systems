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

typedef struct
{
  char **board;

  int r;
  int c;
} ScrabbleBoard;

ScrabbleBoard *scrabble;

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

void fillBoard(int r, int c)
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

void viewBoard(FILE *fp)
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
bool checkBoardLine(int r, int c, char *word, char *direction, FILE *fp, int height, int width)
{

  int length = strlen(word);
  int count = 0;
  char horizontalCheck[width];
  for (int i = 0; i < scrabble->c; i++)
  {
    horizontalCheck[i] = scrabble->board[r][i];
    if (i >= c && i < c + length)
    {
      horizontalCheck[i] = word[count++];
    }
  }

  fprintf(fp, "horizontal Line we are checking is %s\n", horizontalCheck);
  char *wordInLine = strtok(horizontalCheck, " ");

  while (wordInLine != NULL)
  {
    fprintf(fp, "word in line is %s\n", wordInLine);
    if (!checkWord(wordInLine))
    {
      return false;
    }
    wordInLine = strtok(NULL, " ");
  }

  count = 0;
  char verticalCheck[height + 1];
  for (int i = 0; i < scrabble->r; i++)
  {
    verticalCheck[i] = scrabble->board[i][c];
    
    if (i >= r && i < r + length)
    {
      verticalCheck[i] = word[count++];
    }
  }
  fprintf(fp, "THE LENGTH IN THIS METHOD IS %d\n", height);
  verticalCheck[height] = '\0';

  fprintf(fp, "\nline we are checking is %s\n", verticalCheck);
  wordInLine = strtok(verticalCheck, " ");

  while (wordInLine != NULL)
  {
    fprintf(fp, "word in line is %s\n", wordInLine);
    if (!checkWord(wordInLine))
    {
      return false;
    }
    wordInLine = strtok(NULL, " ");
  }
return true;
}
bool putAcross(ScrabbleBoard *scrabble, int y, int x, char *word)
{
  int length = strlen(word);
  if (x + length > scrabble->c)
  {
    return false;
  }
  int count = 0;
  for (int i = x; i < x + length; i++)
  {
    if (scrabble->board[y][i] != ' ')
    {
      if (scrabble->board[y][i] != word[count])
      {
        return false;
      }
    }
    count++;
  }
  count = 0;
  for (int i = x; i < x + length; i++)
  {
    scrabble->board[y][i] = word[count++];
  }
  return true;
}
bool putDown(ScrabbleBoard *scrabble, int y, int x, char *word)
{

  int length = strlen(word);
  if (y + length > scrabble->r)
  {
    return false;
  }
  int count = 0;
  for (int i = y; i < y + length; i++)
  {
    if (scrabble->board[i][x] != ' ')
    {
      if (scrabble->board[i][x] != word[count])
      {
        return false;
      }
    }
    count++;
  }
  count = 0;
  for (int i = y; i < y + length; i++)
  {
    scrabble->board[i][x] = word[count++];
  }
  return true;
}

/** Port number used by my server */
#define PORT_NUMBER "27560"

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

  // Temporary values for parsing commands.
  char cmd[11];

  while (fscanf(fp, "%10s", cmd) == 1 &&
         strcmp(cmd, "quit") != 0)
  {

    if (strcmp(cmd, "board") == 0)
    {
      viewBoard(fp);
    }

    else if (strcmp(cmd, "across") == 0)
    {

      int c = 0;
      int r = 0;
      char word[50];

      if (fscanf(fp, " %d %d %s", &r, &c, word) != 3)
      {
        fprintf(fp, "%s\n", "Invalid command");
        fprintf(fp, "cmd> ");
        continue;
      }

      if (c < 0 || r < 0 || c >= scrabble->c || r >= scrabble->r)
      {

        fprintf(fp, "%s\n", "Invalid command");
        fprintf(fp, "cmd> ");
        continue;
      }

      if (!checkWord(word))
      {
        if (!checkBoardLine(r, c, word, cmd, fp, scrabble->r, scrabble->c))
        {
          fprintf(fp, "%s\n", "Invalid command");
          fprintf(fp, "cmd> ");
          continue;
        }
      }

      if (!checkBoardLine(r, c, word, cmd, fp, scrabble->r, scrabble->c))
      {
        fprintf(fp, "%s\n", "Invalid command");
        fprintf(fp, "cmd> ");
        continue;
      }
      if (!putAcross(scrabble, r, c, word))
      {
        fprintf(fp, "%s\n", "Invalid command");
        fprintf(fp, "cmd> ");
        continue;
      }
    }
    else if (strcmp(cmd, "down") == 0)
    {
      int c = 0;
      int r = 0;
      char word[50];

      if (fscanf(fp, " %d %d %s", &r, &c, word) != 3)
      {
        fprintf(fp, "%s\n", "Invalid command");
        fprintf(fp, "cmd> ");
        continue;
      }
      if (c < 0 || r < 0 || c >= scrabble->c || r >= scrabble->r)
      {
        fprintf(fp, "%s\n", "Invalid command");
        fprintf(fp, "cmd> ");
        continue;
      }
      if (!checkWord(word))
      {
        if (!checkBoardLine(r, c, word, cmd, fp, scrabble->r, scrabble->c))
        {
          fprintf(fp, "%s\n", "Invalid command");
          fprintf(fp, "cmd> ");
          continue;
        }
      }
      if (!checkBoardLine(r, c, word, cmd, fp, scrabble->r, scrabble->c))
      {
        fprintf(fp, "%s\n", "Invalid command");
        fprintf(fp, "cmd> ");
        continue;
      }

      if (!putDown(scrabble, r, c, word))
      {
        fprintf(fp, "%s\n", "Invalid command");
        fprintf(fp, "cmd> ");
        continue;
      }
    }
    else
    {
      fprintf(fp, "%s\n", "Invalid command");
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

void *threadRoutine(void *param)
{
  int sock = *((int *)param);
  handleClient(sock);

  return NULL;
}

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
  scrabble = (ScrabbleBoard *)malloc(sizeof(ScrabbleBoard));
  fillBoard(r, c);

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

  for (int i = 0; i < scrabble->r; i++)
  {
    free(scrabble->board[i]);
  }
  free(scrabble->board);
  free(scrabble);

  // Stop accepting client connections (never reached).
  close(servSock);

  return 0;
}
