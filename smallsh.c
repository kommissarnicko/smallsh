/******************************************************************************
** PROGRAM 3 - smallsh
** NAME: Nickolas A. Williams
** DATE: 11/23/2015
** Shell
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

//some predefined limits per the assignment guidelines
#define BUFFER_CHAR_LIMIT 2048
#define ARG_NUMBER_LIMIT 512
#define ARG_DELIMITER " \x0A\x09" //delimited by space, horizontal tab, newline

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
	
	return 1;
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
	const char* delimiters = ARG_DELIMITER
	size_t argLimit = ARG_NUMBER_LIMIT;
	int i = 0;
	char* currentToken;
	char** argArray = (char**)malloc(sizeof(char*) * argLimit);
	
	if (argArray == NULL) //ensures malloc success
	{
		perror("smallsh: Unable to allocate parseInput argument token array");
		exit(1);
	}
	currentToken = strtok(input, delimiters);
	while (currentToken != NULL)
	{
		argArray[i] = currentToken;
		i++;
		currentToken = strtok(NULL, delimiters);
	}
	argArray[i] = NULL;
	return argArray;
}


void promptLoop()
{
	char *input;
	char **arugments;
	int status;
	
	do
	{
		printf(": ");
		input = readInput();
		arguments = parseInput(input);
		status = startProcess(arguments);
	} while (status);
}


int main()
{
	promptLoop();
	return EXIT_SUCCESS;
}