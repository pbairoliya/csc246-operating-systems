// Driver program that's simple enough that we can tell what should
// happen.  It has two threads trying to reserve spaces that must overlap.

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

void *nationalistsThread( void *arg ) {
  const char *name = "nationalists";
  
  sleep( 1 );   // Wait 1 second before requesting space

  // Get a double-wide space
  int width = 2;
  int start = allocateSpace( name, width );
  
  sleep( 2 );   // Then, wait for two seconds before returning the space.
  
  freeSpace( name, start, width );
  
  return NULL;
}

void *federalistsThread( void *arg ) {
  const char *name = "federalists";
  
  sleep( 2 );   // Wait 2 second before requesting space

  // Get a double-wide space
  int width = 2;
  int start = allocateSpace( name, width );
  
  sleep( 2 );   // Then, wait for two seconds before returning the space.
  
  freeSpace( name, start, width );
  
  return NULL;
}

int main( int argc, char *argv[] ) {
  // For testing, to buffer only one output line at a time.
  setvbuf( stdout, NULL, _IOLBF, 0 );
  
  // Initialize our monitor.
  initMonitor( 3 );

  // Make a few threads
  pthread_t thread[ 2 ];
  if ( pthread_create( thread + 0, NULL, nationalistsThread, NULL ) != 0 ||
       pthread_create( thread + 1, NULL, federalistsThread, NULL ) != 0 )
    fail( "Can't create one of the threads.\n" );

  // Wait until all the threads finish.
  for ( int i = 0; i < sizeof( thread ) / sizeof( thread[ 0 ] ); i++ )
    pthread_join( thread[ i ], NULL );

  // Free any monitor resources.
  destroyMonitor();

  return 0;
}
