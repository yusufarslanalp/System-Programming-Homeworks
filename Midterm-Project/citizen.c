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
int citizen_num;

int shm_vroom; //shm vaccine room
int shm_cbuffer;
//void * ptr_cbuffer;

int citizen_fifo;

sem_t * sem_client;
sem_t * sem_csection; //critic section
sem_t * sem_pid;

void
take_arguments( int argc, char **argv );

void
handle_sigint(int sig);

int main( int argc, char **argv )
{
    //setbuf(stdout, NULL);
    signal(SIGINT, handle_sigint);

    sigset_t unblock;
    sigemptyset(&unblock);
    sigprocmask( SIG_SETMASK, &unblock, NULL );  

    int client_pid = getpid();
	//fprintf(stderr, "In Citizen\n");

	take_arguments( argc, argv );
    citizen_num = atoi( argv[13] );

    citizen_fifo = open( "citizen_fifo", O_WRONLY );

    //fprintf(stderr, "IN CITIZENNNNNNNNNNNaaaaaaaaaaaaaaa\n" );


    void * ptr_vroom;
    shm_vroom = shm_open( "shm_vroom" , O_CREAT | O_RDWR, 0666);
    //ftruncate(shm_vroom, 100 );
    ptr_vroom = mmap(0, 100, PROT_WRITE, MAP_SHARED, shm_vroom, 0);

    
    void * ptr_cbuffer;
    shm_cbuffer = shm_open( "shm_cbuffer" , O_CREAT | O_RDWR, 0666);
    ptr_cbuffer = mmap(0, 100, PROT_WRITE, MAP_SHARED, shm_cbuffer, 0);
    


	char name_client[50];
	strcpy( name_client, "/client" );    
	sem_client = sem_open(  name_client, O_CREAT, S_IRUSR | S_IWUSR, 0 );	

	char name_pid[50];
	strcpy( name_pid, "/pid" );
	sem_pid = sem_open(  name_pid, O_CREAT, S_IRUSR | S_IWUSR, 0 );	

    
    sem_csection = sem_open(  "/csection", O_RDWR, S_IRUSR | S_IWUSR, 1 );

    //int shm_cbuffer;
    //shm_cbuffer = shm_open( "shm_cbuffer" , O_CREAT | O_RDWR, 0666);
    //ptr_cbuffer = mmap(0, 100, PROT_WRITE, MAP_SHARED, shm_cbuffer, 0);

    //ptr_vroom = [ client_pid,  ]

    int shot_num;
    for( int i = 0; i < times_of_shot; i++ )
    {
        //fprintf(stderr, "CTITZENNNNN FOR BEGIN %d\n", i);

        shot_num = i+1;
        sem_wait( sem_client );
        
        memcpy( ptr_vroom, &client_pid, sizeof(int) );
        memcpy( ptr_vroom + 1*sizeof(int), &citizen_num, sizeof(int) );
        memcpy( ptr_vroom + 2*sizeof(int), &shot_num, sizeof(int) );

        if( shot_num == times_of_shot )
        {
            sem_wait( sem_csection );
                //fprintf(stderr, "CITIZENNNNNNNNN: %d DOSEE::::%d\n", citizen_num, shot_num );
                int remain_client;
                memcpy( &remain_client, ptr_cbuffer + 5*sizeof(int), sizeof(int) );
                remain_client--;
                memcpy( ptr_cbuffer + 5*sizeof(int), &remain_client, sizeof(int) );
            sem_post( sem_csection );
        }

        sem_post( sem_pid );
        //fprintf(stderr, "citizen:::::3\n");

        //fprintf(stderr, "CTITZENNNNN FOR ENDDDDDDDDD %d\n", i);

    }


    close( shm_vroom );
    close( shm_cbuffer );

    sem_close( sem_client );
    sem_close( sem_pid );
    sem_close( sem_csection );


    //printf("Citizen SENDDDDDDDDDD: %d\n", getpid());

    write( citizen_fifo, &client_pid, sizeof(int) );
    close( citizen_fifo );

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
    printf("HEREEEEEEEEEEEEE\n");

    close( shm_vroom );
    close( shm_cbuffer );

    sem_close( sem_client );
    sem_close( sem_pid );
    sem_close( sem_csection );

    close( citizen_fifo );

    exit(0);

}
