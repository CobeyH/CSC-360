/*
 * main.c
 *
 * A simple program to illustrate the use of the GNU Readline library
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>


int main ( void )
{
	for (;;)
	{
		char 	*cmd = readline ("shell>");
		printf ("Got: [%s]\n", cmd);
		
		free (cmd);
	}	
}
