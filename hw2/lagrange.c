#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/stat.h"
#include "errno.h"
#include "signal.h"
#include "find_coeffs.c"

char * file_path;

void catcher(int signum) {
	switch (signum) {
		case SIGUSR1:
			//puts("catcher caught SIGUSR1 in child");
			break;
		case SIGUSR2:
			//puts("catcher caught SIGUSR2");
			break;
		default: ;
			//printf("catcher caught unexpected signal %d\n", signum); // bad idea to call printf in handler
	}
}

double
lagrange(  )
{
    return 1.1;
}

void
line_to_words( char line[], char words[][10], int * size );

void
line_to_float( char line[], float numbers[], int * size );

void
add_num( int line_num, float num );

void
get_line( char line[], int line_num );

float
calc( float datax[], float datay[], int size, float parameter );

void
convert( float numbers[], float datax[], float datay[], int numbers_size );

int main( int argc, char **argv )
{
	file_path = argv[2];

	sigset_t no_mask;
	sigset_t blocked_sig1;
	sigemptyset(&no_mask);
	sigemptyset(&blocked_sig1);
	sigaddset(&blocked_sig1, SIGUSR1);
	sigprocmask( SIG_SETMASK, &blocked_sig1, NULL );


	struct sigaction sact;
	sigfillset(&sact.sa_mask);
	sact.sa_flags = 0;
	sact.sa_handler = catcher;

	if (sigaction(SIGUSR1, &sact, NULL) != 0)
		perror("1st sigaction() error");
	else if (sigaction(SIGUSR2, &sact, NULL) != 0)
		perror("2nd sigaction() error");/**/



	//printf("NUM:%s\n", argv[1] );



	int line_num = atoi( argv[1] );
	int size = 0;
	float numbers[20];
	float datax[10];
	float datay[10];
	char line[1000];
	float first_num;
	float second_num;
	float coeffs[10];


	get_line( line, line_num );
	line_to_float( line, numbers, &size );
	convert( numbers, datax, datay, size );

	//Calculations Done
	//Wait for writing to file
	kill( getppid(), SIGUSR2 );
	sigsuspend( &no_mask );

	//Write to file
	first_num = calc( datax, datay, 6, datax[7] );
	add_num( line_num, first_num );
	/*printf("%s: ", argv[1] );
	for( int i = 0; i < size; i++ ) printf( "%3.1f, ", numbers[i] );
	printf("\n");*/

	//writing copleted
	kill( getppid(), SIGUSR2 );

	//wait for parent process for second round
	sigsuspend( &no_mask );
	//continue second calculation

	second_num = calc( datax, datay, 7, datax[7] );
	//PRINT COEFF
	find_coeffs( coeffs, datax, datay, 7 );
	printf("Polynomial %d: ", line_num);
	for( int i = 0; i < 7; i++ )
	{
		printf("%.1f,", coeffs[i] );
	}
	printf("\n");




	//second round completed
	kill( getppid(), SIGUSR2 );
	//wait for writing
	sigsuspend( &no_mask );
	//printf("I am writing: %d\n", line_num );
	add_num( line_num, second_num );
	//Terminate

	//writing copleted
	kill( getppid(), SIGUSR2 );

	//printf("CHIIIIIIIIIIIIIIIIIIILD\n");
	exit( 0 );

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

void
add_num( int line_num, float num )
{
	char file_content[2000];
	file_content[0] = '\0';
	char str[20];
	int len = 0;

	FILE* file;
	char line[1000];

	file = fopen( file_path, "r");

	for( int i = 0; i < line_num; i++  )
	{
		fgets(line, 1000, file);
		strcat( file_content, line );

	}

	fgets(line, 1000, file);
	//printf( "%s", line );
	str[0] = ',';
    sprintf( (str+1), "%.1f", num );
    len = strlen( str );
    str[ len ] = '\n';
    str[ len+1 ] = '\0';
    line[ strlen(line) -1 ] = '\0';
    strcat( line, str );
	strcat( file_content, line );

	for( int i = line_num+1; i < 8; i++  )
	{
		fgets(line, 1000, file);
		//printf("line::::::%s\n", line );
		strcat( file_content, line );

	}

	//printf("%s\n\n\n", file_content );
	//exit(0);

	fclose(file);/**/

	file = fopen( file_path, "w");
	fprintf( file, "%s", file_content );
	fclose(file);/**/


}

void
get_line( char line[], int line_num )
{
	FILE* file;

	file = fopen( file_path, "r");

	if( file == NULL )
	{
		printf("File NULL in lagrange\n");
		exit(0);
	}

	for( int i = 0; i <= line_num; i++  )
	{
		fgets(line, 1000, file);

	}

	fclose(file);/**/
}

float
calc( float datax[], float datay[], int size, float parameter )
{
	//printf("\n\n\n\n");
	//for( int i = 0; i < size; i++ ) printf("%f\n", datay[i] );
	//printf("PARAMETER: %f\n", parameter );

	int i, j;
	float sum=0.0;
	float factor[20];

	for(i=0; i<size; i++){
		factor[i] = 1.0;
		for(j=0; j<size; j++){
			if(i!=j){
				factor[i] = factor[i] * (parameter - datax[j])/(datax[i]-datax[j]);
			}
		
		}
	}

	for(i=0;i<size;i++){
		sum = sum + factor[i] * datay[i];
	}
	//printf(":::::::::::::::::::::::::%f\n",sum);
	return sum;
}

void
convert( float numbers[], float datax[], float datay[], int numbers_size )
{
	if( numbers_size % 2 != 0 ) printf("ERROR: numbers_size should be even number\n" );
	for( int i = 0; i < numbers_size / 2; i++ )
	{
		datax[i] = numbers[ 2*i ];
		datay[i] = numbers[ 2*i +1 ];
	}
}








