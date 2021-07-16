#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/stat.h"
#include "errno.h"
#include "signal.h"
//#include <sys/types.h>
#include "sys/wait.h"
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>


int num_of_nurse;
int num_of_vaccinator;
int num_of_client;
int buffer_size;
int times_of_shot;
char file_path[4000];
int vaccinator_num;
int total_shot;

sem_t * sem_empty;
sem_t * sem_csection; //critic section
sem_t * sem_shot1;
sem_t * sem_shot2;
sem_t * sem_vac;
sem_t * sem_client;
sem_t * sem_pid;

int shm_vac_table;
void * ptr_vac_table;
int shm_cbuffer;
int shm_vroom; //shm vaccine room
void * ptr_vroom;
void * ptr_cbuffer;

int vac_fifo;

void
take_arguments( int argc, char **argv );

void //get two vaccine
get_vaccines();

void
vaccinate();

void
handle_usr1(int sig);

void
handle_sigint(int sig);

int main( int argc, char **argv )
{

    signal(SIGINT, handle_sigint);

    //fprintf(stderr, "vacccccccccccccccccccccccccccc\n" );

    struct sigaction sact;
    sigfillset(&sact.sa_mask);
    sact.sa_flags = 0;
    sact.sa_handler = handle_usr1;
	sigaction(SIGUSR1, &sact, NULL);

    sigset_t block;
    sigset_t unblock;

    sigfillset(&block);
    sigemptyset(&unblock);

    sigprocmask( SIG_SETMASK, &block, NULL );
    sigprocmask( SIG_SETMASK, &unblock, NULL );

	//fprintf(stderr, "In VACCINATOR\n");

	take_arguments( argc, argv );
    vaccinator_num = atoi( argv[13] );

    //sleep( 1 );

    vac_fifo = open( "vac_fifo", O_WRONLY );
    //fprintf(stderr, "OPPENEDDDDDDDDDDDDDDDDDDDDDnoooooooooo\n" );

	shm_vroom = shm_open( "shm_vroom" , O_CREAT | O_RDWR, 0666);
	ptr_vroom = mmap(0, 100, PROT_WRITE, MAP_SHARED, shm_vroom, 0);

    
    shm_cbuffer = shm_open( "shm_cbuffer" , O_CREAT | O_RDWR, 0666);
    ptr_cbuffer = mmap(0, 100, PROT_WRITE, MAP_SHARED, shm_cbuffer, 0);

    shm_vac_table = shm_open( "shm_vac_table" , O_CREAT | O_RDWR, 0666);
    ptr_vac_table = mmap(0, 2000, PROT_WRITE, MAP_SHARED, shm_vac_table, 0);    

    sem_empty = sem_open(  "/empty", O_RDWR, S_IRUSR | S_IWUSR, 1 );
    sem_csection = sem_open(  "/csection", O_CREAT, S_IRUSR | S_IWUSR, 1 );
    sem_shot1 = sem_open(  "/shot1", O_CREAT, S_IRUSR | S_IWUSR, 0 );
    sem_shot2 = sem_open(  "/shot2", O_CREAT, S_IRUSR | S_IWUSR, 0 );
	sem_vac = sem_open(  "/vaccinator", O_CREAT, S_IRUSR | S_IWUSR, 1 );	
	sem_client = sem_open(  "/client", O_CREAT, S_IRUSR | S_IWUSR, 0 );	
	sem_pid = sem_open(  "/pid", O_CREAT, S_IRUSR | S_IWUSR, 0 );	

    
    total_shot = 0;
    while( 1 )
    {
        get_vaccines();
        //fprintf(stderr, "get: %d\n", gett );
        //gett++;

        sigprocmask( SIG_SETMASK, &block, NULL );
        vaccinate();
        total_shot++;
        sigprocmask( SIG_SETMASK, &unblock, NULL );
    }
}

void
vaccinate()
{
    int current_vac1;
    int current_vac2;
    int client_pid;
    int client_num;
    int c_shot_num;
    int remain_client;
    //fprintf(stderr, "vac:::::1\n");
    sem_wait( sem_vac );
    //fprintf(stderr, "vac:::::2\n");
    sem_post( sem_client );
    //fprintf(stderr, "vac:::::3\n");
    sem_wait( sem_pid );
    //fprintf(stderr, "vac:::::4\n");
        memcpy( &client_pid, ptr_vroom, sizeof(int) );
        memcpy( &client_num, ptr_vroom + 1*sizeof(int), sizeof(int) );
        memcpy( &c_shot_num, ptr_vroom + 2*sizeof(int), sizeof(int) );
        fprintf(stderr, "Vaccinator %d (pid=%d) is inviting citizen pid=%d to the clinic:\n",
                        vaccinator_num, getpid(), client_pid );

        sem_wait( sem_csection );
            memcpy( &current_vac1, ptr_cbuffer, sizeof(int) );
            memcpy( &current_vac2, ptr_cbuffer+sizeof(int), sizeof(int) );
            memcpy( &remain_client, ptr_cbuffer + 5*sizeof(int), sizeof(int) );
            fprintf(stderr, "Citizen %d (pid=%d) is vaccinated for the %dth time: ",
                            client_num, client_pid, c_shot_num );
            fprintf(stderr, "the clinic has %d vaccine1 and %d vaccine2.",
                            current_vac1, current_vac2 );
            if( c_shot_num == times_of_shot )
            {
                fprintf(stderr, " The citizen is leaving. Remaining citizens to vaccinate: %d\n", remain_client );
            }
            else fprintf(stderr, "\n" );
        sem_post( sem_csection );
    sem_post( sem_vac );
    //fprintf(stderr, "vac:::::5\n");

}

void //tet two vaccine
get_vaccines()
{
    int current_vac1;
    int current_vac2;

    sem_wait( sem_shot1 );
    //fprintf(stderr, "Afterrrrrrrrrrrrrrrrr shot11111111111111111111\n" );
    sem_wait( sem_csection );
        memcpy( &current_vac1, ptr_cbuffer, sizeof(int) );
        current_vac1--;
        memcpy( ptr_cbuffer, &current_vac1, sizeof(int) );
    sem_post( sem_csection );
    sem_post( sem_empty );



    sem_wait( sem_shot2 );
    sem_wait( sem_csection );
        memcpy( &current_vac2, ptr_cbuffer+sizeof(int), sizeof(int) );
        current_vac2--;
        //printf("::::::::::.%d\n", current_vac2);
        memcpy( ptr_cbuffer+sizeof(int), &current_vac2, sizeof(int) );
    sem_post( sem_csection );    
    sem_post( sem_empty );


    //sem_wait( sem_csection );
    //fprintf(stderr, "------Vaccinator pid: %d got two vaccines\n", getpid() );
    //sem_post( sem_csection );
}


void
take_arguments( int argc, char **argv )
{
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
}

void
handle_usr1(int sig)
{
    //fprintf(stderr, "Vaccinator %d (pid=%d) vaccinated %d doses. ",
                    //vaccinator_num, getpid(), total_shot );

    //fprintf(stderr, "total_shot:::::::::::::::::%d\n", total_shot );
    //fprintf(stderr, "vaccinator_num:::::::::::::::::%d\n", vaccinator_num );

    memcpy( ptr_vac_table + sizeof(int) * (vaccinator_num-1),
            &total_shot, sizeof(int) );


    close( shm_vroom );
    close( shm_cbuffer );
    close( shm_vac_table );

    sem_close( sem_csection );
    sem_close( sem_shot1 );
    sem_close( sem_shot2 );
    sem_close( sem_vac );
    sem_close( sem_client );
    sem_close( sem_pid );
    sem_close( sem_empty );


    int pid = getpid();
    write( vac_fifo, &pid, sizeof(int) );
    close( vac_fifo );



    exit(0);
}

void
handle_sigint(int sig)
{

    close( shm_vroom );
    close( shm_cbuffer );
    close( shm_vac_table );

    sem_close( sem_csection );
    sem_close( sem_shot1 );
    sem_close( sem_shot2 );
    sem_close( sem_vac );
    sem_close( sem_client );
    sem_close( sem_pid );
    sem_close( sem_empty );

    close( vac_fifo );

    exit(0);
}
