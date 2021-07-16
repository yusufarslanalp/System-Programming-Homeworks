#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/stat.h"
#include "errno.h"
#include "signal.h"
//#include <sys/types.h>
#include "sys/wait.h"


volatile sig_atomic_t handled_signals = 0;
volatile sig_atomic_t reaped_childs = 0;
char * file_path;


void
line_to_float( char line[], float numbers[], int * size );

void
get_line( char line[], int line_num );

void
line_to_words( char line[], char words[][10], int * size );

void
get_values( float values[], int index_at_line );

float
calc_error( int one_or_two );

void catcher(int signum) {
	switch (signum) {
		case SIGINT:
			printf("Process interrupted\n");
			exit(0);
			break;		
		case SIGUSR1:
			//puts("catcher caught SIGUSR1");
			break;
		case SIGUSR2:
			handled_signals = handled_signals + 1;
			//puts("catcher caught SIGUSR2");
			break;
		case SIGCHLD:
			reaped_childs = reaped_childs + 1;
			//puts("CHILD REAPED");
			break;			
		default: 
			printf("catcher caught unexpected signal %d\n", signum); // bad idea to call printf in handler
	}
}

int main( int argc, char **argv )
{
	file_path = argv[1];

	if( argc != 2 )
	{
		printf("Error: file path should be passed as argument\n");
		printf("Sample usage: \n");
	}

	float error_deg5 = 0;
	float error_deg6 = 0;

	int child_pids[10];
	sigset_t no_mask;
	sigset_t blocked_sig2;
	sigset_t unblocked_sig2;		

	sigemptyset(&no_mask);

	sigemptyset(&blocked_sig2);
	sigaddset(&blocked_sig2, SIGUSR2);
	sigaddset(&blocked_sig2, SIGCHLD);

	sigemptyset( &unblocked_sig2 );
	sigaddset(&unblocked_sig2, SIGCHLD);

	struct sigaction sact;
	sigfillset(&sact.sa_mask);
	sact.sa_flags = 0;
	sact.sa_handler = catcher;

	if (sigaction(SIGUSR1, &sact, NULL) != 0)
		perror("1st sigaction() error");
	else if (sigaction(SIGUSR2, &sact, NULL) != 0)
		perror("2nd sigaction() error");
	else if (sigaction(SIGINT, &sact, NULL) != 0)
		perror("2nd sigaction() error");
	else if (sigaction(SIGCHLD, &sact, NULL) != 0)
		perror("2nd sigaction() error");	


	char msg[2];
	msg[0] = '0';
	msg[1] = '\0';
	char par0[300] = "./lagrange";
	char par1[300] = "pid";
	char par2[300] = "file_path";
	char *newargv[] = { par0, par1, par2, NULL };

	int pid;

	//printf("hereee\n");


	sigprocmask( SIG_SETMASK, &blocked_sig2, NULL );


	for( int i = 0; i < 8; i++ )
	{

		pid = fork();
		//sleep( 1 );
		if( pid == 0 )
		{
			//printf("in iffffff\n");
			msg[0] = '0' + i;
			newargv[1] = msg;
			newargv[2] = argv[1];
			printf("::::::%d\n", execve( "./lagrange", newargv, NULL ) );
			perror( "Error" );

		}
		child_pids[i] = pid;



	}


	//printf("No one can wake up me 5 second\n");
	//int remain = sleep( 5 );
	//if( remain != 0 ) printf("Who waked up me\n");


	//Wait all childrens for first calculations
	handled_signals = 0;
	while( 1 )
	{
		//printf("HEREEEEEEEEEEE1\n");
		sigsuspend( &unblocked_sig2 );
		if( handled_signals == 8 ) break;

	}

	//let childs for writing to file
	for(int i = 0; i < 8; i++) //////////
	{
		kill( child_pids[i], SIGUSR1 );
		//printf("HERE3\n");
		sigsuspend( &unblocked_sig2 );
		//exit(0); /////////////////////////////

	}

	error_deg5 = calc_error( 1 );
	printf("Error of polynomial of degree 5: %f\n", error_deg5 );

	//HEPSİNİN SINYALI ALMAK ICIN HAZIR OLDUGU NE MALUM

	//awake childs for second round
	for(int i = 0; i < 8; i++) //////////
	{
		kill( child_pids[i], SIGUSR1 );
	}


	//wait all childs for completion of second round
	handled_signals = 0;
	while( 1 )
	{
		//printf("handled: %d\n", handled_signals);
		sigsuspend( &unblocked_sig2 );
		if( handled_signals == 8 ) break;

	}
	

	//printf("HEREEEEEEEEEEE2222222\n");


	//let childs for writing to file
	for(int i = 0; i < 8; i++) //////////
	{
		kill( child_pids[i], SIGUSR1 );
		//printf("HERE3\n");
		sigsuspend( &unblocked_sig2 );
		//exit(0); /////////////////////////////

	}

	error_deg6 = calc_error( 2 );
	printf("Error of polynomial of degree 6: %f\n", error_deg6 );



	//int status;
	//reap all childs
	for(int i = 0; i < 8; i++) 
	{
		//printf( "INNNNNNNNN\n" );
		//sigsuspend( &no_mask );
		wait( NULL );
		//printf("%d\n", wait( NULL ) );
		//printf("reaped_childs: %d\n", reaped_childs );
	}


	//printf("main terminated\n");


}

float
calc_error( int one_or_two )
{
	float org_values[8]; //original values
	float calc_values[8]; //calculated values

	float total_org = 0;
	float total_calc = 0;

	get_values( org_values, 15 );
	get_values( calc_values, 15 + one_or_two );

	for( int i = 0; i < 8; i++ )
	{
		total_org += org_values[i];
		total_calc += calc_values[i];
	}

	//printf("Total Calc%f\n", total_calc);
	return (total_org - total_calc) / 8.0;
}

void
get_values( float values[], int index_at_line )
{
	int size = 0;
	float numbers[20];
	char line[1000];

	for( int i = 0; i < 8; i++ )
	{
		get_line( line, i );
		line_to_float( line, numbers, &size );
		values[i] = numbers[index_at_line];
	}
}

void
get_line( char line[], int line_num )
{
	FILE* file;

	file = fopen( file_path, "r");

	if( file == NULL )
	{
		printf("File NULL\n");
		exit(0);
	}

	for( int i = 0; i <= line_num; i++  )
	{
		fgets(line, 1000, file);

	}

	fclose(file);/**/
}

void
line_to_words( char line[], char words[][10], int * size )
{
	int i = 0;
	*size = 0;
	int word_index = 0;

	while( 1 )
	{
		if( line[i] == '\n' )
		{
			words[*size][word_index] = '\0';
			*size = *size + 1;
			break;
		}
		else if(  line[i] == ',' )
		{
			words[*size][word_index] = '\0';
			word_index = 0;
			*size = *size + 1;
			i++;
		}
		else
		{
			words[*size][word_index] = line[i];
			i++;
			word_index++;
		}
	}
}

void
line_to_float( char line[], float numbers[], int * size )
{
	char words[30][10];

	line_to_words( line, words, size );
	for( int i = 0; i < *size; i++ )
	{
		numbers[i] = atof( words[i] );
		//printf( "%f\n", numbers[i] );

	}
	//printf("here\n");
}


