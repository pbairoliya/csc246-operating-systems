/**
 * @file hall.c the actual implemtation of hall.h that the drivers call in this makes sure that monitor is intiatlized utalized and also freed at the end.
 * @author Pratik Bairoliya
 * @date 2022-10-31
 *
 * HW 4
 *
 */
#include <stdio.h>
#include <stdbool.h>
#include <pthread.h> // for pthreads
#include <stdlib.h>  // for exit
#include <unistd.h>  // for sleep/usleep
#include "hall.h"    //interface for this class


// array of stars or occupied spots
char *occupied;
// the monitor itself
pthread_mutex_t mon;
// condition to see if we can enter the monitor
pthread_cond_t freeCond;

//size of the array
int len;
/** Initialize the monitor as a hall with n spaces that can be partitioned  off.
 * @param n the size of the array that the moinotr will utilize.
 * */
void initMonitor(int n)
{
   
    occupied = (char *)malloc(sizeof(char) * n);
    len = n;

    for (int i = 0; i < n; i++)
    {
        occupied[i] = '*';
    }


    pthread_mutex_init(&mon, NULL);
    pthread_cond_init(&freeCond, NULL);
}
/** Destroy the monitor, freeing any resources it uses. */
void destroyMonitor()
{
    pthread_mutex_destroy(&mon);
    pthread_cond_destroy(&freeCond);
    free(occupied);

}

/**
 * Helper method I created to figure out if there is space to put the comapnies name in and if it doesnt it returns -1
 *
 * @param occupied the array we are seeing if there is space
 * @param width  the width of the company it self
 * @return int the startingPosition to put the company name if there is space if not -1
 */
int findSpace(char *occupied, int width)
{

    for (int i = 0; i <= (len- width); i++)
    {
        bool found = true;

        for (int j = 0; j < width; j++)
        {
            if (occupied[j + i] != '*')
            {
                found = false;
            }
        }
        if (!found)
        {
            continue;
        }
        else
        {
            return i;
        }
    }

    return -1;
}
/** Called when an organization wants to reserve the given number
    (width) of contiguous spaces in the hall.  Returns the index of
    the left-most (lowest-numbered) end of the space allocated to the
    organization. */
int allocateSpace(char const *name, int width)
{
    pthread_mutex_lock(&mon);
    int startPos = findSpace(occupied, width);

    if (startPos != -1)
    {
        for (int i = startPos; i < (width + startPos); i++)
        {
            occupied[i] = name[0];
        }

        printf("%s allocated: %s\n", name, occupied);
        pthread_mutex_unlock(&mon);
        return startPos;
    }

    else
    {
        printf("%s waiting: %s\n", name, occupied);
        while (startPos == -1)
        {
            pthread_cond_wait(&freeCond, &mon);
            startPos = findSpace(occupied, width);
        }
        for (int i = startPos; i < (width + startPos); i++)
        {
            occupied[i] = name[0];
        }
        printf("%s allocated: %s\n", name, occupied);
        pthread_mutex_unlock(&mon);
        return startPos;
    }
}
/** Relese the allocated spaces from index start up to (and including)
    index start + width - 1. */
void freeSpace(char const *name, int start, int width)
{
    pthread_mutex_lock(&mon);
    for (int i = start; i < start + width; i++)
    {
        occupied[i] = '*';
    }

    printf("%s freed: %s\n", name, occupied);
    pthread_cond_broadcast(&freeCond);
    pthread_mutex_unlock(&mon);
}