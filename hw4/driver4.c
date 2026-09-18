// Multi-threaded program to simulate a lot of organizations requesting and
// freeing space.

#include <stdio.h>
#include <pthread.h>   // for pthreads
#include <stdlib.h>    // for exit
#include <unistd.h>    // for usleep
#include <string.h>    // for strcmp

#include "hall.h"

// True as long as the simulation is running.  This is a way to
// tell all the threads when it's time to exit.
int running = 1;

/** Bound on how much time threads will wait before trying to reserve
    a space. */
#define WAIT_TIME 10000

/** Bound on how much time threads will hold a space before releasing
    it. */
#define HOLD_TIME 10000

// General purpose fail function.  Print a message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

/** This record helps to keep up with each thread. */
typedef struct {
  // Name of this organization.
  char name[ 21 ];

  // Number of contiguous spaces needed.
  int width;

  // Total number of reservations obtained.
  int count;
} ThreadRec;

/** Code used by each thread to repeatedly reserve then free a space. */
void *threadRoutine( void *arg ) {
  // Get my name from the argument.
  ThreadRec *tr = arg;

  // Keep running until the main thread tells us to stop
  while ( running ) {
    // Wait a moment then enter from the west.
    usleep( rand() % WAIT_TIME );

    // Reserve a space.
    int start = allocateSpace( tr->name, tr->width );

    // Hold the space for a little bit.
    usleep( rand() % HOLD_TIME );

    // Release the space.
    freeSpace( tr->name, start, tr->width );
    
    // Count one successful reservation for this thread.
    tr->count += 1;
  }

  return NULL;
}

// Struct instance for each thread we'll use.
static ThreadRec threadRec[] =
  {
   { "Aerosmith", 5 },
   { "Bread", 5 },
   { "Carpenters", 5 },
   { "Delfonics", 5 },
   { "Eagles", 4 },
   { "Foghat", 4 },
   { "Genesis", 4 },
   { "Heart", 4 },
   { "Isis", 3 },
   { "Journey", 3 },
   { "M", 3 },
   { "Ocean", 3 },
   { "Parliament", 2 },
   { "Queen", 2 },
   { "Ramones", 2 },
   { "SANTANA", 2 },
   { "Toto", 1 },
   { "U2", 1 },
   { "Wings", 1 },
   { "YES", 1 },
  };

int main( int argc, char *argv[] ) {
  // Initialize the monitor our threads are using.
  initMonitor( 20 );

  // Thread id for each of the threads
  int tcount = sizeof( threadRec ) / sizeof( threadRec[ 0 ] );
  pthread_t thread[ tcount ];

  // Make a thread for each car.
  for ( int i = 0; i < tcount; i++ ) {
    if ( pthread_create( thread + i, NULL, threadRoutine,
                         threadRec + i ) != 0 )
      fail( "Failed creating a thread\n" );
  }

  // Let them do what they do for a little while.
  sleep( 10 );
  running = 0;

  // Wait for all the threads to finish.
  for ( int i = 0; i < tcount; i++ )
    pthread_join( thread[ i ], NULL );

  
  // Report how many times each thread got a space in the hall, and
  // a total.
  int total = 0;
  for ( int i = 0; i < tcount; i++ ) {
    printf( "%s made %d reservations\n", threadRec[ i ].name,
            threadRec[ i ].count );
    total += threadRec[ i ].count;
  }
  printf( "Total: %d\n", total );

  // Free any monitor resources.
  destroyMonitor();


  return 0;
}
