#ifndef __HALL_H__
#define __HALL_H__

/** Interface for the Meeting Hall monitor.  If I was programming in
    Java or C++, this would all be wrapped up in a class.  Since we're using
    C, it's just a collection of functions, with an initialization
    function to init the state of the whole monitor. */


/** Initialize the monitor as a hall with n spaces that can be partitioned
    off. */
void initMonitor( int n );

/** Destroy the monitor, freeing any resources it uses. */
void destroyMonitor();

/** Called when an organization wants to reserve the given number
    (width) of contiguous spaces in the hall.  Returns the index of
    the left-most (lowest-numbered) end of the space allocated to the
    organization. */
int allocateSpace( char const *name, int width );

/** Relese the allocated spaces from index start up to (and including)
    index start + width - 1. */
void freeSpace( char const *name, int start, int width );

#endif
