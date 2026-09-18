/**
 * @file stash.c is a c program file that makes its own shell in which you can run the same commands that a terminal or a linux bash would
 * @author Pratik Bairoliya pbairol@ncsu.edu
 * Class: CSC 246  Dr. Sturgill
 * Assignment: HW_0
 * @date 2022-09-01
 */

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**
 * This is a custom string length function that takes in a string and return the lenght as an integer
 * I personally like my own custom string length function and that is the reason why I am using this function.
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
 * @param nums is anotehr char pointer string that we try to parse and convert into an integer
 * @return int  the converted integer value if the string can be parsed is returned or if cannot -1 is returned
 */
int customATOI(char *nums)
{
    int numLength = customStrLen(nums);
    int number = 0;
    for (int i = 0; i < numLength-1; i++)
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
 * This funciton was on the guide line that we have to follow the point of this function is to parse through the line removing the space and replacing it with '/0' 
 * Then using the passed in argument char *words[] is an array that points to each word in the line
 * @param line this is a string line that is read in per input of the stash shell which is then parsed by space adn replaced with '/0' so we can identify the words in the line
 * @param words this is an array of strings or words which the array points to each word in the line string
 * @return int the number of words that are in the line 
 */
int parseCommand(char *line, char *words[])
{
    int length = customStrLen(line);
    for (int i = 0; i < length; i++)
    {
        if (line[i] == ' ' || line[i] == 10||line[i] == EOF || line[i] == '\0')
        {
            line[i] = '\0';
        }
    }
    int wordCounter = 0;
    words[wordCounter++] = &line[0];
    for (int i = 0; i < length; i++)
    {
        if (line[i] == '\0')
        {
            if (i + 1 < length)
            {
                words[wordCounter++] = &line[i + 1];
            }
        }
    }
    return wordCounter;
}
/**
 * This function runs the exit command that the user may input it checks if the status of the exit is valid and takes in the count of the words per line which helps to determine if the exit command is valid or not
 * @param words This is the array of strings that is suppsoe to hold both the exit word and the status word 
 * @param count This is the count of words in the array of words
 */
void runExit(char *words[], int count)
{
    if (count != 2)
    {
        char errorMessage[] = "Invalid command\n";
        write(STDOUT_FILENO, errorMessage, customStrLen(errorMessage));
        return;
    }
    else
    {
        int num = customATOI(words[1]);
        
        if (num < 0)
        {
            
            char errorMessage[] = "Invalid command\n";
            write(STDOUT_FILENO, errorMessage, customStrLen(errorMessage));
            return;
        }
        exit(num);
    }
}
/**
 * This function runs the CD command that the user may input CD and the directory afterwards the function checks only 2 words were inputed and both words are valid for the cd command
 * @param words This is the array of strings that is suppsoe to hold both the cd word and the directory that we want to change into
 * @param count This is the count of words in the array of words
 */
void runCd(char *words[], int count)
{
    if (count != 2)
    {
        char errorMessage[] = "Invalid command\n";
        write(STDOUT_FILENO, errorMessage, customStrLen(errorMessage));
        return;
    }
    else
    {
        chdir(words[1]);
    }
}
/**
 * This function runs every other command that the user may input this checks that if the first word is a valid command start
 * @param words This is the array of strings that is suppsoe to hold the command call and everything else that the command might need
 * @param count This is the count of words in the array of words
 */
void runCommand(char *words[], int count)
{
    if (count < 1)
    {
        return;
    }
    int pid = fork();
    int status;
    if (pid == -1)
    {
        char errorMessage[] = "Can’t create a child ";
        write(STDOUT_FILENO, errorMessage, customStrLen(errorMessage));
        return;
    }
    else if (pid == 0)
    {
        words[count] = NULL;
        if (execvp(words[0], words) < 0)
        {
            char errorMessage[] = "Can’t run command ";
            write(STDOUT_FILENO, errorMessage, customStrLen(errorMessage));
            for (int i = 0; i < count; i++)
            {
                write(STDOUT_FILENO, words[i], customStrLen(words[i]));
            }
             write(STDOUT_FILENO, "\n", 1);
             _exit(0);
        }
    }
    else
    {
        while (wait(&status) != pid);
    }
}
/**
 * main function that calls all other functions to put the terminal together
 * @return int the return is the exit status in int form.
 */
int main()
{
    while (true)
    {
        char line[1024];
        char *words[513];
        write(STDOUT_FILENO, "stash> ", customStrLen("stash> "));
        int accRead = read(STDIN_FILENO, line, 1024);
        line[accRead] = '\0';
        int count = parseCommand(line, words);

        if (strcmp(words[0], "cd") == 0)
        {
            runCd(words, count);
        }
        else if (strcmp(words[0], "exit") == 0)
        {
            runExit(words, count);
        }
        else
        {
            runCommand(words, count);
        }
    }
}
