#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/stat.h"
#include "errno.h"
#include "signal.h"
//#include <sys/types.h>
#include "sys/wait.h"



int main( int argc, char **argv )
{




	char msg[2];
	msg[0] = '0';
	msg[2] = '\0';
	char par0[300] = "./client";
	char par1[300] = "-i";
	char par2[300] = "1";

	char par3[300] = "-a";
	char par4[300] = "127.0.0.1";
	char par5[300] = "-p";

	char par6[300] = "20000";
	char par7[300] = "-o";
	char par8[300] = "queries.txt";	

	char *newargv[] = { par0, par1, par2, par3, par4, par5, par6, par7, par8, NULL };

	int pid;



	for( int i = 0; i < 10; i++ )
	{

		pid = fork();
		//sleep( 1 );
		if( pid == 0 )
		{
			//printf("in iffffff\n");
			msg[0] = '1' + i;
			newargv[2] = msg;
			printf("::::::%d\n", execve( "./client", newargv, NULL ) );
			perror( "Error" );

		}



	}

}


