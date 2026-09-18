// Driver program that's simple enough that we can tell what should
// happen.  It has a thread that can get its space only after the first
// two threads vacate their spaces.

#include <stdio.h>
#include <pthread.h>   // for pthreads
#include <stdlib.h>    // for exit
#include <unistd.h>    // for sleep/usleep

#include "hall.h"

// General-purpose fail function.  Print a message and exit.
static void fail( char const *message ) {
  fprintf( stderr, "%s\n", message );
  exit( 1 );
}

void *hogwartsThread( void *arg ) {
  const char *name = "hogwarts";
  
  sleep( 1 );   // Wait 1 second before requesting space
  
  // Get a double-wide space.
  int width = 2;
  int start = allocateSpace( name, width );
  
  sleep( 4 );   // Then, wait 4 seconds before releasing it.

  freeSpace( name, start, width );
  
  return NULL;
}

void *beauxbatonsThread( void *arg ) {
  const char *name = "beauxbatons";
  
  sleep( 2 );   // Wait 2 seconds before requesting space
  
  // Get a double-wide space.
  int width = 2;
  int start = allocateSpace( name, width );
  
  sleep( 2 );   // Then, wait 2 seconds before releasing it.

  freeSpace( name, start, width );
  
  return NULL;
}

void *durmstrangThread( void *arg ) {
  const char *name = "durmstrang";
  
  sleep( 3 );   // Wait 3 second before requesting space
  
  // Get a quadruple-wide space.
  int width = 4;
  int start = allocateSpace( name, width );
  
  sleep( 1 );   // Then, wait 1 second before releasing it.

  freeSpace( name, start, width );
  
  return NULL;
}

int main( int argc, char *argv[] ) {
  // For testing, to buffer only one output line at a time.
  setvbuf( stdout, NULL, _IOLBF, 0 );
  
  // Initialize our monitor.
  initMonitor( 4 );

  // Make a few threads
  pthread_t thread[ 3 ];
  if ( pthread_create( thread + 0, NULL, hogwartsThread, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, beauxbatonsThread, NULL ) != 0 ||
       pthread_create( thread + 2, NULL, durmstrangThread, NULL ) != 0 )
    fail( "Can't create one of the threads.\n" );

  // Wait until all the threads finish.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyMonitor();

  return 0;
}
