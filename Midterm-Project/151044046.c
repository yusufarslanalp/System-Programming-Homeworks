#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/stat.h"
#include "errno.h"
#include "signal.h"
#include <sys/types.h>
#include "sys/wait.h"
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


char num_of_nurse[20];
char num_of_vaccinator[20];
char num_of_client[20];
char buffer_size[20];
char times_of_shot[20];
char file_path[4000];

int nurse_pids[1000];
int vac_pids[1000];
int citizen_pids[1000];

sem_t * sem_empty;		//initially buffer size
sem_t * sem_csection;	//initially 1
sem_t * sem_shot1;		//initially 0
sem_t * sem_shot2;		//initially 0
sem_t * sem_vac;		//initially 1
sem_t * sem_client;		//initially 0
sem_t * sem_pid;		//initially 0

int shm_vroom; //shm vaccine room
void * ptr_vroom;
int shm_offset; //shm  ///
void * ptr_offset;
int shm_vac_table;
void * ptr_vac_table;
int shm_cbuffer;
void * ptr_cbuffer;

int vac_fifo;
int citizen_fifo;

void
take_arguments( int argc, char **argv );

void
handle_sigint(int sig);

void
show_usage();

int main( int argc, char **argv )
{
	signal(SIGINT, handle_sigint);

    sigset_t block;
    sigset_t unblock;
    sigfillset(&block);
    sigemptyset(&unblock);
	sigprocmask( SIG_SETMASK, &block, NULL );
    


	//fprintf(stderr, "mainnnnnnnnnnnnnnnnnnnnnn\n" );

	take_arguments( argc, argv );
	/*printf("num_of_nurse: %s\n", num_of_nurse );
	printf("num_of_vaccinator: %s\n", num_of_vaccinator );
	printf("num_of_client: %s\n", num_of_client );
	printf("buffer_size: %s\n", buffer_size );
	printf("times_of_shot: %s\n", times_of_shot );
	printf("file_path: %s\n", file_path );*/
	fprintf(stderr, "Welcome to the GTU344 clinic. Number of citizen to vaccinate c=%d with t=%d doses.\n",
			atoi( num_of_client ), atoi( times_of_shot ) );

	mkfifo( "vac_fifo" , S_IRUSR | S_IWUSR );
	mkfifo( "citizen_fifo" , S_IRUSR | S_IWUSR );

	char arg1[50] = "-n";
	char arg2[50];
	strcpy( arg2, num_of_nurse );

	char arg3[50] = "-v";
	char arg4[50];
	strcpy( arg4, num_of_vaccinator );

	char arg5[50] = "-c";
	char arg6[50];
	strcpy( arg6, num_of_client );

	char arg7[50] = "-b";
	char arg8[50];
	strcpy( arg8, buffer_size );

	char arg9[50] = "-t";
	char arg10[50];
	strcpy( arg10, times_of_shot );

	char arg11[50] = "-i";
	char arg12[4000];	
	strcpy( arg12, file_path );

	char arg13[50] = "";	

	char *newargv[] = {	"", arg1, arg2, arg3, arg4, arg5,
						arg6, arg7, arg8, arg9, arg10, arg11, arg12, arg13, NULL };



	int t = atoi( times_of_shot );
	int c = atoi( num_of_client );

	if( atoi( buffer_size ) < t*c+1 )
	{
		printf("ERROR: buffer size must be greather than t * c\n");
		exit( 0 );
	}

	
	sem_empty = sem_open(  "/empty", O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, atoi( buffer_size ) );

	if( sem_empty == 0 )
	{
		sem_unlink( "/empty" );
		perror("ERROR");
		fprintf(stderr, "Run the program again.\n" );
		exit(0);
	}

	sem_csection = sem_open(  "/csection", O_CREAT, S_IRUSR | S_IWUSR, 1 );
	sem_shot1 = sem_open(  "/shot1", O_CREAT, S_IRUSR | S_IWUSR, 0 );
	sem_shot2 = sem_open(  "/shot2", O_CREAT, S_IRUSR | S_IWUSR, 0 );
	sem_vac = sem_open(  "/vaccinator", O_CREAT, S_IRUSR | S_IWUSR, 1 );	
	sem_client = sem_open(  "/client", O_CREAT, S_IRUSR | S_IWUSR, 0 );	
	sem_pid = sem_open(  "/pid", O_CREAT, S_IRUSR | S_IWUSR, 0 );	


	int temp1;
	sem_getvalue( sem_empty, &temp1 );
	//fprintf(stderr, "sem_empty MAINNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNexcl:::::::%d\n", temp1 );


	shm_vroom = shm_open( "shm_vroom" , O_CREAT | O_RDWR, 0666);
	ftruncate(shm_vroom, 100 );
	ptr_vroom = mmap(0, 100, PROT_WRITE, MAP_SHARED, shm_vroom, 0);



	shm_offset = shm_open( "shm_offset" , O_CREAT | O_RDWR, 0666);
	ftruncate(shm_offset, 100 );
	ptr_offset = mmap(0, 100, PROT_WRITE, MAP_SHARED, shm_offset, 0);



	shm_vac_table = shm_open( "shm_vac_table" , O_CREAT | O_RDWR, 0666);
	ftruncate(shm_vac_table, 2000 );
	ptr_vac_table = mmap(0, 2000, PROT_WRITE, MAP_SHARED, shm_vac_table, 0);


	int zero = 0;
	memcpy( ptr_offset, &zero, sizeof(int) );


	shm_cbuffer = shm_open( "shm_cbuffer" , O_CREAT | O_RDWR, 0666);
	ftruncate(shm_cbuffer, 100 );
	ptr_cbuffer = mmap(0, 100, PROT_WRITE, MAP_SHARED, shm_cbuffer, 0);

	memcpy( ptr_cbuffer, &zero, sizeof(int) );
	memcpy( ptr_cbuffer+sizeof(int), &zero, sizeof(int) );
	int temp = atoi( num_of_client );
	memcpy( ptr_cbuffer + 5*sizeof(int), &temp, sizeof(int) );

	int pid;

	for( int i = 0; i < atoi( num_of_nurse ); i++ )
	{
		pid = fork();
		if( pid == 0 )
		{
			sprintf(arg13, "%d", i+1);
			printf("::::::%d\n", execve( "./nurse" , newargv, NULL ) );
			perror( "Error" );
		}
		nurse_pids[i] = pid;
	}
	for( int i = 0; i < atoi( num_of_vaccinator ); i++ )
	{
		pid = fork();
		if( pid == 0 )
		{
			sprintf(arg13, "%d", i+1);
			//vac_pids[i] = getpid();
			printf("::::::%d\n", execve( "./vaccinator" , newargv, NULL ) );
			perror( "Error" );
		}
		vac_pids[i] = pid;
	}
	for( int i = 0; i < atoi( num_of_client ); i++ )
	{
		pid = fork();
		if( pid == 0 )
		{
			sprintf(arg13, "%d", i+1);
			printf("::::::%d\n", execve( "./citizen" , newargv, NULL ) );
			perror( "Error" );
		}
		citizen_pids[i] = pid;
	}

	sigprocmask( SIG_SETMASK, &unblock, NULL );
	    
    //
    int vac_pid;
    int client_pid;


    //fprintf(stderr, "BEFORE OPEN\n" );
	vac_fifo = open( "vac_fifo", O_RDONLY );
	//fprintf(stderr, "AFTER OPEN\n" );
	//fprintf(stderr, "vac_fifo:::::::%d\n", vac_fifo );

	citizen_fifo = open( "citizen_fifo", O_RDONLY );

	//fprintf(stderr, "atoi num of client:::::::::: %d\n", atoi(num_of_client) );

    //wait termination of all client processes
    for( int i = 0; i < atoi(num_of_client); i++ )
    {
    	//fprintf(stderr, "iiiiiiiiiiiiii:::::::%d\n", i );
    	read( citizen_fifo, &client_pid, sizeof(int) );
	}

	//sleep( 5 );

	

    for( int i = 0; i < atoi( num_of_vaccinator ); i++ )
    {
    	kill( vac_pids[i], SIGUSR1 );
	}

	//wait for termination of waccinators
    for( int i = 0; i < atoi(num_of_vaccinator); i++ )
    {
    	read( vac_fifo, &vac_pid, sizeof(int) );
    	//fprintf(stderr, "read:::::::::%d\n", read( vac_fifo, &vac_pid, sizeof(int) ) );
    	//fprintf(stderr, "vac pid: %d\n", vac_pid );
	}

	//

	fprintf(stderr, "All citizens have been vaccinated .\n" );


	int vaccinator_num;
	int total_shot;
    for( int i = 0; i < atoi(num_of_vaccinator); i++ )
    {
    	
    	vaccinator_num = i+1;
    	memcpy( &total_shot, ptr_vac_table + (i*sizeof(int)), sizeof(int) );
	    fprintf(stderr, "Vaccinator %d (pid=%d) vaccinated %d doses. ",
	                    vaccinator_num, vac_pids[i], total_shot );
	}


	//sleep(2);

	fprintf(stderr, "The clinic is now closed. Stay healthy.\n" );

	
	close( citizen_fifo );
	close( vac_fifo );

	close( shm_offset );
	close( shm_vroom );
	close( shm_cbuffer );
	close( shm_vac_table );

	shm_unlink( "shm_offset" );
	shm_unlink( "shm_vroom" );
	shm_unlink( "shm_cbuffer" );
	shm_unlink( "shm_vac_table" );


	sem_close( sem_csection );
	sem_close( sem_shot1 );
	sem_close( sem_shot2 );
	sem_close( sem_vac );
	sem_close( sem_client );
	sem_close( sem_pid );
	sem_close( sem_empty );

	sem_unlink( "/vaccinator" );
    sem_unlink( "/client" );
	sem_unlink( "/pid" );
	sem_unlink( "/empty" );
    sem_unlink( "/csection" );
	sem_unlink( "/shot1" );
	sem_unlink( "/shot2" );
    

	exit(0);	


}


void
show_usage()
{
	printf("ERROR: command line arguments are missing/invalid\n");
	printf("Sample usage: ./program -n 3 -v 2 -c 3 -b 11 -t 3 -i test.txt\n");
}

void
take_arguments( int argc, char **argv )
{
	//for( int i = 0; i < argc; i++ ) printf("%s\n", argv[i]);

	if( argc != 13 )
	{
		show_usage();
		exit(0);
	}



    int number_of_setted_value = 0;
    int c;
    while ((c = getopt (argc, argv, ":n:v:c:b:t:i:")) != -1)
    switch (c)
    {
        case 'n':
        	strcpy( num_of_nurse, optarg );
            number_of_setted_value++;
            break;
        case 'v':
        	strcpy( num_of_vaccinator, optarg );
            number_of_setted_value++;
            break;
        case 'c':
        	strcpy( num_of_client, optarg );
            number_of_setted_value++;
            break;
        case 'b':
        	strcpy( buffer_size, optarg );
            number_of_setted_value++;
            break;
        case 't':
        	strcpy( times_of_shot, optarg );
            number_of_setted_value++;
            break;
        case 'i':
            strcpy( file_path, optarg );
            number_of_setted_value++;
            break;            

        default:
            printf("ERROR: Undefined parameters dedected in command line arguments\n");
            //show_usage();
            exit( 0 );
    }

    if( number_of_setted_value != 6 )
    {
    	show_usage();
    	exit( 0 );
    }
}


void
handle_sigint(int sig)
{
	for( int i = 0; i < atoi( num_of_nurse ); i++ )
	{
		kill( nurse_pids[i], SIGINT );
	}
	for( int i = 0; i < atoi( num_of_vaccinator ); i++ )
	{
		kill( vac_pids[i], SIGINT );
	}
	for( int i = 0; i < atoi( num_of_client ); i++ )
	{
		kill( citizen_pids[i], SIGINT );
	}

	close( citizen_fifo );
	close( vac_fifo );

	close( shm_offset );
	close( shm_vroom );
	close( shm_cbuffer );
	close( shm_vac_table );

	shm_unlink( "shm_offset" );
	shm_unlink( "shm_vroom" );
	shm_unlink( "shm_cbuffer" );
	shm_unlink( "shm_vac_table" );


	sem_close( sem_csection );
	sem_close( sem_shot1 );
	sem_close( sem_shot2 );
	sem_close( sem_vac );
	sem_close( sem_client );
	sem_close( sem_pid );
	sem_close( sem_empty );

	sem_unlink( "/vaccinator" );
    sem_unlink( "/client" );
	sem_unlink( "/pid" );
	sem_unlink( "/empty" );
    sem_unlink( "/csection" );
	sem_unlink( "/shot1" );
	sem_unlink( "/shot2" );
    

	exit(0);	

}
