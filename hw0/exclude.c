/**
 * @file exclude.c: program that removes the line that the user provides and outputs it to an
 * @author Pratik Bairoliya pbairol@ncsu.edu
 * Class: CSC 246  Dr. Sturgill
 * Assignment: HW_0
 * @date 2022-08-31
 */

#include <unistd.h>
#include <fcntl.h>

/**
 * This is a custom string length function that takes in a string and return the lenght as an integer
 * Due to the fact that we were not allowed to use the c standard library I had to create this function
 * @param string the char pointer string that we are trying to calculate the length of
 * @return int  the return value is an integer length of the string.
 */
int customStrLen(char *string)
{
    int length = 0;
    while (*string != '\0')
    {
        length++;
        string++;
    }
    return length;
}

/**
 * This is another custom function which does  ATOI and that takes in string that we expect to have character digits. 
 * If the string cannot be parsed this function returns -1 otherwise it returns the parsed version of the int
 * I wrote this function due to the fact that we were not allowed to use c standard library hence why this function was made
 * @param nums is anotehr char pointer string that we try to parse and convert into an integer
 * @return int  the converted integer value if the string can be parsed is returned or if cannot -1 is returned
 */
int customATOI(char *nums)
{
    int numLength = customStrLen(nums);
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

/**
 * int main this is the main function of exclude.c and returns the exit status 0 if everything ran smoothly
 * It interacts with all the other classes to create the program wordle
 * @param argc this is an int argument that tells the main function how any arguments were given from terminal
 * @param argv the array with the arguments stored in chars this helps us figure out what filename list we are using and what seed number it is if provided
 * @return int it returns either a 0 - Exit SUCCESS status or terminates in the middle due to some exception or failing test case
 */
int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        // error
        char errorMessage[] = "usage: exclude <input-file> <output-file> <line-number>\n";
        write(STDERR_FILENO, errorMessage, customStrLen(errorMessage));
        _exit(-1);
    }
    int fd_write;
    int fd_read;

    fd_read = open(argv[1], O_RDONLY);
    if (fd_read < 0)
    {
        char errorMessage[] = "usage: exclude <input-file> <output-file> <line-number>\n";
        write(STDERR_FILENO, errorMessage, customStrLen(errorMessage));
        _exit(-1);
    }
    fd_write = open(argv[2], O_RDONLY | O_WRONLY | O_CREAT, 0600);
    if (fd_write < 0)
    {
        close(fd_read);
        char errorMessage[] = "usage: exclude <input-file> <output-file> <line-number>\n";
        write(STDERR_FILENO, errorMessage, customStrLen(errorMessage));
        _exit(-1);
    }

    int lineExclude = customATOI(argv[3]);

    if (lineExclude < 0)
    {
        close(fd_read);
        close(fd_write);
        char errorMessage[] = "usage: exclude <input-file> <output-file> <line-number>\n";
        write(STDERR_FILENO, errorMessage, customStrLen(errorMessage));
        _exit(-1);
    }

    char buffer[64];
    int lineCounter = 0;
    int readIN = 0;

    readIN = read(fd_read, buffer, 64);
    int writerCounter = 0;
    char buffer2[64];
    while (readIN > 0)
    {
        for (int i = 0; i < readIN; i++)
        {
            if (lineCounter != lineExclude - 1)
            {
                buffer2[writerCounter++] = buffer[i];
            }
            if (buffer[i] == '\n')
            {
                lineCounter++;
            }
            if (writerCounter == 64)
            {
                write(fd_write, buffer2, writerCounter);
                writerCounter = 0;
            }
        }
        readIN = read(fd_read, buffer, 64);
    }
    // leftover if the writer doesnt fill the 64 bits fully
    if (writerCounter > 0)
    {
        write(fd_write, buffer2, writerCounter);
    }
    close(fd_read);
    close(fd_write);
}