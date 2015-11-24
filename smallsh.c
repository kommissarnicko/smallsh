/******************************************************************************
** PROGRAM 3 - smallsh
** NAME: Nickolas A. Williams
** DATE: 11/23/2015
** Shell
******************************************************************************/

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//some predefined limits per the assignment guidelines
#define BUFFER_CHAR_LIMIT 2048
#define ARG_NUMBER_LIMIT 512
#define ARG_DELIMITER " \x0A\x09" //delimited by space, horizontal tab, newline

void changeDirectory(char** argArray)
{
	char* directory;
	if (argArray[1] == NULL)
	{
		directory = getenv("HOME");
	}
	else
	{
		directory = argArray[1];
	}
	// printf("directory = [%s]\n", directory);
	// printf("final char = [%d]\n", directory[strlen(directory) - 1]);
	if (chdir(directory) == -1)
	{
		printf("Invalid directory.\n");
	}
}


int executeInput(char **argArray)
{
	if (argArray[0] == NULL)
	{
		return 1;
	}
	
	if (strcmp("exit", argArray[0]) == 0)
	{
		return 0;
	}
	
	if (strcmp("cd", argArray[0]) == 0)
	{
		changeDirectory(argArray);
		return 1;
	}
	
	if (strcmp("status", argArray[0]) == 0)
	{
		printf("Previous command terminated by signal [%d].\n", status);
		return 1;
	}
	
	if (strcmp("#", argArray[0]) == 0)
	{
		return 1;
	}
	
	else
	{
		return startProcess(argArray);
	}
}


int startProcess(char **argArray)
{
	pid_t pid;
	int status;
	
	pid = fork();
	if (pid == 0) //we are in the child process
	{
		if (execvp(argArray[0], argArray) == -1)
		{
			perror("smallsh: ");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		perror("smallsh: ");
	}
	else
	{
		do
		{
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	
	return status;
}


char* readInput()
{
	size_t bufferSize = BUFFER_CHAR_LIMIT;
	char* buffer = (char*)malloc(sizeof(char) * bufferSize);
	
	if (buffer == NULL) //ensures malloc success
	{
		perror("smallsh: Unable to allocate readInput buffer");
		exit(1);
	}
	
	getline(&buffer, &bufferSize, stdin);
	return buffer;
}


char** parseInput(char *input)
{
	size_t argLimit = ARG_NUMBER_LIMIT;
	int i = 0;
	char* currentToken;
	char** argArray = (char**)malloc(sizeof(char*) * argLimit);
	
	if (argArray == NULL) //ensures malloc success
	{
		perror("smallsh: Unable to allocate parseInput argument token array");
		exit(1);
	}
	
	currentToken = strtok(input, ARG_DELIMITER);
	while (currentToken != NULL)
	{
		argArray[i] = currentToken;
		i++;
		currentToken = strtok(NULL, ARG_DELIMITER);
	}
	argArray[i] = NULL;
	return argArray;
}


void promptLoop()
{
	char *input;
	char **arguments;
	int status;
	
	do
	{
		printf(": ");
		input = readInput();
		arguments = parseInput(input);
		status = executeInput(arguments);
		
		free(input);
		free(arguments);
	} while (1);
}


int main()
{
	promptLoop();
	return EXIT_SUCCESS;
}