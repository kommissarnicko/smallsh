/******************************************************************************
** PROGRAM 3 - smallsh
** NAME: Nickolas A. Williams
** DATE: 11/23/2015
** DESCRIPTION: Very simple UNIX Shell with limited functionality
** CREDITS: Most of my reference for how this was going to work came from
** three places:
** 		1. Lectures 9 through 13, Benjamin Brewster
**		   <https://oregonstate.instructure.com/courses/1555023/pages/block-3>
**		2. Tutorial Write A Shell In C, Stephen Brennan (last accessed 11/23/2015)
**		   <http://stephen-brennan.com/2015/01/16/write-a-shell-in-c/>
**		3. mini-shell, by "Bharathi" (last accessed 11/23/2015)
**		   <https://code.google.com/p/mini-shell/>
******************************************************************************/

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

//some predefined limits per the assignment guidelines
#define BUFFER_CHAR_LIMIT 2048
#define ARG_NUMBER_LIMIT 512
#define ARG_DELIMITER " \x0A\x09" //delimited by space, horizontal tab, newline


/******************************************************************************
** changeDirectory(char** argArray)
** DESCRIPTION: changes current working directory to string in argArray[1]
** or if null, will change directory to HOME.
** PARAMETERS: pointer to array of char pointers (argument list); first
** element will contain "cd", second element contains path or NULL
** RETURNS: n/a
******************************************************************************/
void changeDirectory(char** argArray)
{
	char* directory;
	if (argArray[1] == NULL) //user entered just "cd"
	{
		directory = getenv("HOME");
	}
	else
	{
		directory = argArray[1];
	}
	if (chdir(directory) == -1) //chdir has failed, invalid directory
	{
		printf("Invalid directory.\n");
	}
}


/******************************************************************************
** printStatus(int status)
** DESCRIPTION: prints to stdout status last command executed by startProcess()
** PARAMETERS: status returned by waitpid in startProcess()
** RETURNS: n/a
******************************************************************************/
void printStatus(int status)
{
	if ((status >= 0) && (status <= 1)) //"normal" exit
	{
		printf("Exit value %d\n", status);
	}
	if (status > 1) //signaled termination
	{
		printf("Terminated by signal %d\n", status);
	}
}


/******************************************************************************
** int executeInput(char **argArray, int status)
** DESCRIPTION: if command is built-in, will execute. Built-in commands are:
**				~exit: exits the shell
**				~cd: calls changeDirectory
**				~status: calls printStatus and passes last status
**				~#: just a comment, doesn't do anything
**				~NULL: if passed null, that means the user just pushed enter
**				If not built in, will call startProcess and pass it argument list
** PARAMETERS: argument list argArray, parsed user input, command to execute will
** be in argArray[0].
** RETURNS: status of executed command, either built-in or with startProcess()
******************************************************************************/
int executeInput(char **argArray, int status)
{
	if (argArray[0] == NULL)
	{
		return 1; //user just pushed enter and did nothing
	}
	
	if (strcmp("exit", argArray[0]) == 0)
	{
		exit(EXIT_SUCCESS); //exits the shell
	}
	
	if (strcmp("cd", argArray[0]) == 0)
	{
		changeDirectory(argArray); //errors here are handled in function
		return 1;
	}
	
	if (strcmp("status", argArray[0]) == 0)
	{
		printStatus(status); //cannot have an error here, always success
		return 1;
	}
	
	if (strcmp("#", argArray[0]) == 0)
	{
		return 1; //ignore input that begins with a #
	}
	
	else
	{
		return startProcess(argArray); //not built in, return status when done
	}
}


/******************************************************************************
** int fileInputOutput(char* term)
** DESCRIPTION: checks out argArray[1] to see if it's a redirection for
** input/output for use in startProcess()
** PARAMETERS: string of chars from argArray[1], either a < > or neither
** RETURNS: 0 means neither < nor >; 1 = > (pipe output of command in argArray[0] 
** to file), or 2 = < (pipe output of command in argArray[0] to file)
******************************************************************************/
int fileInputOutput(char* term)
{
	if (strcmp(">", term) == 0)
	{
		return 1;
	}
	if (strcmp("<", term) == 0)
	{
		return 2;
	}
	else //no applicable item in argArray[1]
	{
		return 0;
	}
}


/******************************************************************************
** int startProcess(char **argArray)
** DESCRIPTION: Given array of arguments, checks if file I/O operations are
** needed, if so, opens or creates file, then executes program stored in
** first argument space
** PARAMETERS: array of pointers to strings; argArray[0] = command, 
** argArray[1] = could be < or > for file I/O, argArray[2]... optional
** RETURNS: integer returned by waitpid once child process terminates
** CREDITS: The file I/O portion of this is derived directly from Lecture 12
** "Pipes and IPC" by Benjamin Brewster,
** <https://oregonstate.instructure.com/courses/1555023/pages/block-3>
******************************************************************************/
int startProcess(char **argArray)
{
	pid_t pid;
	int status;
	int fd, fd2;
	int redirect = 0;
	if (argArray[1] != NULL) //prevents against segfaults in single-arg commands
	{
		redirect = fileInputOutput(argArray[1]);
	}
	
	pid = fork();
	if (pid == 0) //we are in the child process
	{
		if (redirect == 1) //second item in argArray is >
		{
			//file I/O from lectures
			fd = open(argArray[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
			if (fd == -1)
			{
				perror("open");
				exit(1);
			}
			fd2 = dup2(fd, 1);
			if (fd2 == -1)
			{
				perror("dup2");
				exit(2);
			}
			if (execlp(argArray[0], argArray[0], NULL) == -1)
			{
				perror("smallsh: ");
			}
			exit(EXIT_FAILURE);
		}
		else if (redirect == 2) //second item in argArray is <
		{
			//file I/O from lectures
			fd = open(argArray[2], O_RDONLY);
			if (fd == -1)
			{
				perror("open");
				exit(1);
			}
			fd2 = dup2(fd, 0);
			if (fd2 == -1)
			{
				perror("dup2");
				exit(2);
			}	
			if (execlp(argArray[0], argArray[0], NULL) == -1)
			{
				perror("smallsh: ");
			}
			exit(EXIT_FAILURE);
		}
		else //second item in argArray is not < or >
		{
			//non file-I/O command
			if (execvp(argArray[0], argArray) == -1)
			{
				perror("smallsh: ");
			}
			exit(EXIT_FAILURE);
		}
	}
	else if (pid < 0) //error!
	{
		perror("smallsh: ");
	}
	else //shell will wait for child to complete process
	{
		do
		{
			waitpid(pid, &status, WUNTRACED);
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	
	return status; //for use by status command
}


/******************************************************************************
** char* readInput()
** DESCRIPTION: reads input from stdin
** PARAMETERS: n/a
** RETURNS: array containing raw data from stdin
******************************************************************************/
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


/******************************************************************************
** char** parseInput(char *input)
** DESCRIPTION: separates input from readInput() into an array of strings for
** use as a list of arguments
** PARAMETERS: raw array of chars from readInput()
** RETURNS: array of pointers to arrays of chars (strings), each string
** is NULL-terminated
******************************************************************************/
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


/******************************************************************************
** void promptLoop()
** DESCRIPTION: main loop for performing shell operations: gets user input,
** parses user input, executes user input, holds status of execution
** PARAMETERS: N/A
** RETURNS: N/A
******************************************************************************/
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
		status = executeInput(arguments, status);
		
		free(input);
		free(arguments);
	} while (1);
}


/******************************************************************************
** MAIN!
******************************************************************************/
int main()
{
	promptLoop();
	return EXIT_SUCCESS;
}