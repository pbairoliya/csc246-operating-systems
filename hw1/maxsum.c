/**
 * @file maxsum.c maxsum program that finds the longest contiguous maximum sum of an array using fork and pipe.
 * @author Pratik Bairoliya pbairol@ncsu.edu
 * @date 2022-09-15
 * 
 * CSC 246 HW1
 * 
 */
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdbool.h>

// Print out an error message and exit.
static void fail(char const *message)
{
  fprintf(stderr, "%s\n", message);
  exit(1);
}

// Print out a usage message, then exit.
static void usage()
{
  printf("usage: maxsum <workers>\n");
  printf("       maxsum <workers> report\n");
  exit(1);
}

// Input sequence of values.
int *vList;

// Number of values on the list.
int vCount = 0;

// Capacity of the list of values.
int vCap = 0;
// Read the list of values.
void readList()
{
  // Set up initial list and capacity.
  vCap = 5;
  vList = (int *)malloc(vCap * sizeof(int));

  // Keep reading as many values as we can.
  int v;
  while (scanf("%d", &v) == 1)
  {
    // Grow the list if needed.
    if (vCount >= vCap)
    {
      vCap *= 2;
      vList = (int *)realloc(vList, vCap * sizeof(int));
    }

    // Store the latest value in the next array slot.
    vList[vCount++] = v;
  }
}

int main(int argc, char *argv[])
{
  bool report = false;
  int workers = 4;

  // Parse command-line arguments.
  if (argc < 2 || argc > 3)
    usage();

  if (sscanf(argv[1], "%d", &workers) != 1 || workers < 1)
    usage();

  // If there's a second argument, it better be the word, report
  if (argc == 3)
  {
    if (strcmp(argv[2], "report") != 0)
      usage();
    report = true;
  }

  readList();

  // a pipe for our two children to talk over.
  int pfd[2];
  pipe(pfd);

  for (int i = 0; i < workers; i++)
  {
    pid_t id = fork();
    if (id == -1)
    {
      fail("Can't create child");
    }
    // Based on the return from fork, are we the parent or the child?
    if (id == 0)
    {
      // maxSum of the child worker
      int maxSum = 0;
      // the current Max of one for loop iteration
      int curr_max = 0;
      for (int j = i; j < vCount; j += workers)
      {
        curr_max = 0;
        for (int k = j; k < vCount; k++)
        {
          curr_max += vList[k];

          if (maxSum < curr_max)
          {
            maxSum = curr_max;
          }
        }
      }
      if (report)
      {
        printf("I'm process %d. The maximum sum I found is %d.\n", getpid(), maxSum);
      }
      lockf(pfd[1], F_LOCK, 0);

      write(pfd[1], &maxSum, sizeof(maxSum));
      lockf(pfd[1], F_ULOCK, 0);
      close(pfd[1]);
      exit(0);
    }
  }
  for (int i = 0; i < workers; i++)
  {
    wait(NULL);
  }
  int n = 0;
  int maxSum = 0;
  close(pfd[1]); // Close writing end of second pipe
  while (read(pfd[0], &n, sizeof(n)) > 0)
  {
    if (maxSum <= n)
    {
      maxSum = n;
    }
  }
  close(pfd[0]);                       // Close reading end of second pipe
  printf("Maximum Sum: %d\n", maxSum); // Print the max sum after everything is done

  return EXIT_SUCCESS;
}
