/**
 * @file maxsum-sem.c calculates the maximum contiguous summation of an array using threads and synchronization provided by semaphores. I utalized the consumer and producer alogrithm that Dr. Sturgil taught in class.
 * @author Pratik Bairoliya
 * @date 2022-10-13
 * 
 * HW 3 
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <semaphore.h>
#include <sys/syscall.h>

// Print out an error message and exit.
static void fail(char const *message)
{
  fprintf(stderr, "%s\n", message);
  exit(EXIT_FAILURE);
}

// Print out a usage message, then exit.
static void usage()
{
  printf("usage: maxsum-sem <workers>\n");
  printf("       maxsum-sem <workers> report\n");
  exit(1);
}
//semaphores utalized in the consumer producer algorithm taught in class.
sem_t ecSem;
sem_t fcSem;
sem_t lockSem;


//semaphore that waits and posts to make sure the updating of the global max_sum to be synchronized 
sem_t maxSumSem;


// True if we're supposed to report what we find.
bool report = false;

// Maximum sum we've found.
int max_sum = INT_MIN;

// Fixed-sized array for holding the sequence.
#define MAX_VALUES 500000


//length of the bounded buffer.
#define BUFFERLENGTH 500000

// arrray that contains all the values of a file that the user will provide.
int vList[MAX_VALUES];

// Current number of values on the list.
int vCount = 0;

// bool check if done reading in finally
bool doneReading = false;

// position of first in the buffer - synch slides
int first = 0;

// number of items in the buffer - synch slides
int num = 0;

//global variable of workers
int numWorkers = 4 ;

// actual buffer array that is used for producer and consumer - synch slides
int buff[BUFFERLENGTH];

// Read the list of values.
//Producer method that fills in the buffer for the consumer method to figure out what indicies the thread should chcker for
void readList()
{
  // Keep reading as many values as we can.
  int v;

  while (scanf("%d", &v) == 1)
  {
    sem_wait(&ecSem);
    sem_wait(&lockSem);

    // Make sure we have enough room, then store the latest input.
    if (vCount > MAX_VALUES)
      fail("Too many input values");

    // Store the latest value.
    vList[vCount++] = v;
    buff[(first + num) % BUFFERLENGTH] = vCount;
    num++;
    sem_post(&lockSem);
    sem_post(&fcSem);
  }
  
  for(int i =0; i<numWorkers;i++){
    sem_wait(&ecSem);
    sem_wait(&lockSem);

    buff[(first + num) % BUFFERLENGTH] = -1;
    num++;
    sem_post(&lockSem);
    sem_post(&fcSem);


  }
  
}


//Consumer method that gets indicies from the bounded buffer and returns the index that the thread will utialize to calculate its personal maxSum
int getWork()
{
  sem_wait(&fcSem);
  sem_wait(&lockSem);

  
  int getIndex = buff[first];
  first = (first + 1) % BUFFERLENGTH;
  num--;
  sem_post(&lockSem);
  sem_post(&ecSem);

  return getIndex;
}

/** Start routine for each worker. */
void *workerRoutine(void *arg)
{

  int curr_sum = 0;
  int max = 0;
  int i = getWork();

  while(i!=-1)
  {
    curr_sum = 0;
    for (int j = i; j >= 0; j--)
    {
      curr_sum += vList[j];
      if (max < curr_sum)
      {
        max = curr_sum;
      }
    }
    i = getWork();
  }

  if (report)
  {
    printf("I'm thread %lu. The maximum sum I found is %d.\n",pthread_self(), max);
  }

  sem_wait(&maxSumSem);
  if (max_sum < max)
  {
    max_sum = max;
  }
  sem_post(&maxSumSem);
  return NULL;
}


//Main  thread that error checks and creates and destorys the annonymous sempahores. It also creates and destorys the worker threads and runs them
int main(int argc, char *argv[])
{
  int workers = 4;

  // Parse command-line arguments.
  if (argc < 2 || argc > 3)
    usage();

  if (sscanf(argv[1], "%d", &workers) != 1 ||
      workers < 1)
    usage();

  // If there's a second argument, it better be "report"
  if (argc == 3)
  {
    if (strcmp(argv[2], "report") != 0)
      usage();
    report = true;
  }
  //initalizes all the semaphores to all the correct values 
  sem_init(&fcSem, 0, 0);
  sem_init(&ecSem, 0, BUFFERLENGTH);
  sem_init(&lockSem, 0, 1);
  sem_init(&maxSumSem, 0, 1);

  numWorkers = workers;
  // Make each of the workers.
  pthread_t worker[workers];
  // runs each of the program
  for (int i = 0; i < workers; i++)
  {
    pthread_create(&worker[i], NULL, workerRoutine, NULL);
  }

  // Then, start getting work for them to do.
  readList();
  //flags if it is done reading
  doneReading = true;

  // Wait until all the workers finish.
  for (int i = 0; i < workers; i++)
  {
    pthread_join(worker[i], NULL);
  }

  // Report the max product and release the semaphores.
  printf("Maximum Sum: %d\n", max_sum);

  //destroys anonymous semaphores
  sem_destroy(&fcSem);
  sem_destroy(&ecSem);
  sem_destroy(&lockSem);
  sem_destroy(&maxSumSem);

  return EXIT_SUCCESS;
}
