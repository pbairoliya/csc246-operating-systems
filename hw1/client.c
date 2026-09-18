/**
 * @file client.c the client c program that communicates with server and sends in command for the Lights out game.
 * @author Pratik Bairoliya pbairol@ncsu.edu
 * @date 2022-09-15
 * CSC 246 HW 1
 * 
 */
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
/**
 * This is the int main for client where it sends and receives messages from the server and client queue 
 * It sneds in commands to server and receives success or failure of those commands
 * @param argc the number of args
 * @param argv  the args itself in string array
 * @return int 
 */
int main(int argc, char *argv[])
{
    // Make both the server and client message queues.
    mqd_t serverQueue = mq_open(SERVER_QUEUE, O_WRONLY);
    mqd_t clientQueue = mq_open(CLIENT_QUEUE, O_RDONLY);
    //server checking if they open properly
    if (serverQueue == -1 || clientQueue == -1)
        fail("Can't open the needed message queues");
    //checking if they are too few or too many args
    if (argc <= 1 || argc > 4)
    {
        printf("%s\n", "error");
        exit(1);
    }
    //checking if there is a bad command or not
    if ((strcmp(argv[1], "report") != 0) && (strcmp(argv[1], "undo") != 0) && (strcmp(argv[1], "move") != 0))
    {

        printf("%s\n", "error");
        exit(1);
    }
    else
    {
        //report sending to serverQueue and printing board
        if (strcmp(argv[1], "report") == 0)
        {
            if (argc != 2)
            {
                printf("%s\n", "error");
                exit(1);
            }

            char clientMessage[MESSAGE_LIMIT + 1] = "";
            strcpy(clientMessage, argv[1]);
            mq_send(serverQueue, clientMessage, strlen(clientMessage), 0);

            char table[MESSAGE_LIMIT + 1];
            mq_receive(clientQueue, table, sizeof(table), NULL);

            int ctr = 0;
            for (int i = 0; i < 5; i++)
            {
                for (int j = 0; j < 5; j++)
                {
                    printf("%c", table[ctr++]);
                }
                printf("\n");
            }
        }
        //sending undo command to serverQueue and returning success or failure
        else if (strcmp(argv[1], "undo") == 0)
        {
            if (argc != 2)
            {
                printf("%s\n", "error");
                exit(1);
            }
            char clientARGS[MESSAGE_LIMIT + 1] = "";
            strcpy(clientARGS, "");
            strcpy(clientARGS, argv[1]);
            mq_send(serverQueue, clientARGS, strlen(clientARGS), 0);
            char response[MESSAGE_LIMIT] = "";
            mq_receive(clientQueue, response, sizeof(response), 0);
            if (strcmp(response, "error") == 0)
            {
                printf("%s\n", response);
                exit(1);
            }
            else
            {
                printf("%s\n", response);
            }
        }
        //sending move command to serverQueue and returning success or failure of the move lights out worked or not
        else if (strcmp(argv[1], "move") == 0)
        {
            if (argc != 4)
            {
                printf("%s\n", "error");
                exit(1);
            }
            char clientARGS[MESSAGE_LIMIT + 1] = "";
            strcpy(clientARGS, "");
            strcpy(clientARGS, argv[1]);
            mq_send(serverQueue, clientARGS, strlen(clientARGS), 0);
            strcpy(clientARGS, "");
            strcpy(clientARGS, argv[2]);
            mq_send(serverQueue, clientARGS, strlen(clientARGS), 1);
            strcpy(clientARGS, "");
            strcpy(clientARGS, argv[3]);
            mq_send(serverQueue, clientARGS, strlen(clientARGS), 2);
            char response[MESSAGE_LIMIT];
            mq_receive(clientQueue, response, sizeof(response), 0);

            if (strcmp(response, "error") == 0)
            {
                printf("%s\n", response);
                exit(1);
            }
            else
            {
                printf("%s\n", response);
            }
        }
    }
    
    // Close our two message queues (and delete them).
    mq_close(clientQueue);
    mq_close(serverQueue);
}