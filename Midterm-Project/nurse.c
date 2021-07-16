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
#include <sys/file.h>

int num_of_nurse;
int num_of_vaccinator;
int num_of_client;
int buffer_size;
int times_of_shot;
char file_path[4000];
int nurse_num;

int fd;
void * ptr_offset;
void * ptr_cbuffer;
int number_of_vaccine;
char vaccine_kind;

int shm_offset;
int shm_cbuffer;

sem_t * sem_empty;
sem_t * sem_csection; //critic section
sem_t * sem_shot1;
sem_t * sem_shot2;

void
take_arguments( int argc, char **argv );

void
get_vaccine();

void
put_vaccine();

void
handle_sigint(int sig);

int main( int argc, char **argv )
{
	signal(SIGINT, handle_sigint);

    sigset_t unblock;
    sigemptyset(&unblock);
    sigprocmask( SIG_SETMASK, &unblock, NULL ); 

	take_arguments( argc, argv );
	nurse_num = atoi( argv[13] );


	number_of_vaccine = num_of_client * times_of_shot * 2;

	
	shm_offset = shm_open( "shm_offset" , O_CREAT | O_RDWR, 0666);
	ptr_offset = mmap(0, 100, PROT_WRITE, MAP_SHARED, shm_offset, 0);

	
	shm_cbuffer = shm_open( "shm_cbuffer" , O_CREAT | O_RDWR, 0666);
	ptr_cbuffer = mmap(0, 100, PROT_WRITE, MAP_SHARED, shm_cbuffer, 0);


    sem_empty = sem_open(  "/empty", O_CREAT, S_IRUSR | S_IWUSR, 1 );   
    sem_csection = sem_open(  "/csection", O_CREAT, S_IRUSR | S_IWUSR, 1 );
    sem_shot1 = sem_open(  "/shot1", O_CREAT, S_IRUSR | S_IWUSR, 0 );
    sem_shot2 = sem_open(  "/shot2", O_CREAT, S_IRUSR | S_IWUSR, 0 );


	fd = open( file_path, O_RDWR, S_IRWXU );
	if( fd == -1 )
	{
		perror( "ERROR" );
		exit( 0 );
	}

	while(1)
	{
		get_vaccine();
		//printf("vaccine_kind: %d pid: %d\n", vaccine_kind, getpid() );
		//All vaccines taked from file
		if( vaccine_kind == 'e' )
		{
			break;
		}
		//fprintf(stderr, "i::%d\n", i );
		//i++;
		//sleep(3);
		put_vaccine();
	}/**/



	close( shm_offset );
	close( shm_cbuffer );

	sem_close( sem_csection );
	sem_close( sem_shot1 );
	sem_close( sem_shot2 );
	sem_close( sem_empty );


	close( fd );/**/

}


void
put_vaccine()
{
	//fprintf(stderr, "put_vac innnnnnnnnnnnnnn\n" );
	int current_vac1;
	int current_vac2;
	int count;

	//fprintf(stderr, "put_vac 1111111111111111111111\n" );
	int temp;
	sem_getvalue( sem_empty, &temp );
	//fprintf(stderr, "sem_empty:::::::%d\n", temp );
	sem_wait( sem_empty );

	//fprintf(stderr, "put_vac 2222222222222\n" );

	sem_wait( sem_csection );
		//fprintf(stderr, "put_vac 33333333333333333\n" );
		memcpy( &current_vac1, ptr_cbuffer, sizeof(int) );
		memcpy( &current_vac2, ptr_cbuffer+sizeof(int), sizeof(int) );
		memcpy( &count, ptr_cbuffer+ 2*sizeof(int), sizeof(int) );
		count++;
		memcpy( ptr_cbuffer+ 2*sizeof(int), &count, sizeof(int) );
		if( vaccine_kind == '1' )
		{
	        current_vac1++;
	        memcpy( ptr_cbuffer, &current_vac1, sizeof(int) );
		}
		else if( vaccine_kind == '2' )
		{
	        current_vac2++;
	        memcpy( ptr_cbuffer+sizeof(int), &current_vac2, sizeof(int) );
		}

		fprintf(stderr, "Nurse%d (pid=%d) has brought vaccine%c: ", nurse_num, getpid(), vaccine_kind);
		fprintf(stderr, "the clinic has %d vaccine1 and %d vaccine2\n", current_vac1, current_vac2 );
		if( count == (2*times_of_shot*num_of_client) )
		{
			fprintf(stderr, "Nurses have carried all vaccines to the buffer, terminating.\n" );
		}
	sem_post( sem_csection );
	//fprintf(stderr, "put_vac 4444444444\n" );
	if( vaccine_kind == '1' )
	{
		sem_post( sem_shot1 );
	}
	else if( vaccine_kind == '2' )
	{
		sem_post( sem_shot2 );
	}
	else
	{
		fprintf(stderr, "ERROR IN put_vaccine function\n");
		//printf("vaccine_kind: %c\n", vaccine_kind );
		exit(0);
	}
	//fprintf(stderr, "put_vac outttttttttttttttttt\n" );
}

void
get_vaccine()
{
	int offset;

	//fprintf(stderr, "get_vac innnnnnnnnnnnnnn\n" );

    if (flock(fd, LOCK_EX) != 0) {}
    offset = 0;
    memcpy( &offset, ptr_offset, sizeof(int) );

	//fprintf(stderr, "OFFFFFFFFFFFFFFFSET::::::::%d\n", offset );

    if( offset >= number_of_vaccine )
    {
	    if (flock(fd, LOCK_UN) != 0) {  }
	    vaccine_kind = 'e';
	    return;
    }

	lseek( fd, offset, SEEK_SET );
	read( fd, &vaccine_kind, 1 );

	//printf("offset: %d\n", offset );
	//printf( "PID: %d    vaccine_kind: %c\n", pid, vaccine_kind );
    
	offset++;
    memcpy( ptr_offset, &offset, sizeof(int) );

    if (flock(fd, LOCK_UN) != 0) {}

    //fprintf(stderr, "get_vac outttttttttttttttttttttttt\n" );
}


void
take_arguments( int argc, char **argv )
{
	//for( int i = 0; i < argc; i++ ) printf("%s\n", argv[i]);

	/*if( argc != 9 )
	{
		show_usage();
		exit(0);
	}*/



    int number_of_setted_value = 0;
    int c;
    while ((c = getopt (argc, argv, ":n:v:c:b:t:i:")) != -1)
    switch (c)
    {
        case 'n':
            num_of_nurse = atoi( optarg );
            number_of_setted_value++;
            break;
        case 'v':
            num_of_vaccinator = atoi( optarg );
            number_of_setted_value++;
            break;
        case 'c':
            num_of_client = atoi( optarg );
            number_of_setted_value++;
            break;
        case 'b':
            buffer_size = atoi( optarg );
            number_of_setted_value++;
            break;
        case 't':
            times_of_shot = atoi( optarg );
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


    /*if( number_of_setted_value != 4 )
    {
    	show_usage();
    	exit( 0 );
    }*/
}


void
handle_sigint(int sig)
{
	close( shm_offset );
	close( shm_cbuffer );

	sem_close( sem_csection );
	sem_close( sem_shot1 );
	sem_close( sem_shot2 );
	sem_close( sem_empty );


	close( fd );/**/

    exit(0);
}