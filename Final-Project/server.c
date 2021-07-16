#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include<signal.h>
#include <syslog.h>

#include "t_pool.h"
#include "get_csv.h"
#include "sql.h"
#include "reader_writer.h"


int port;
char path_to_log_file[1000];
int pool_size;
char dataset_path[1000];

FILE* log_file;

int AR = 0;
int AW = 0;
int WR = 0;
int WW = 0;

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ok_to_read = PTHREAD_COND_INITIALIZER;
pthread_cond_t ok_to_write = PTHREAD_COND_INITIALIZER;

pthread_t thread_pool[2000];
int stack[2000];
int stack_size;
int num_of_busy = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_server = PTHREAD_COND_INITIALIZER;


char *** table;
int tnum_of_row;
int tnum_of_col;

int terminate = 0;

int instantiation_fd;
char temp_fpath[1000] = "151044046_temp";

void
make_absolute( char path[] );

void
take_arguments( int argc, char **argv );

void
handle_sigint(int sig);

void
my_daemon();


int main( int argc, char **argv )
{
	//printf("ppid: %d\n", getppid() );
	//printf("pid: %d\n", getpid() );

	
	

	take_arguments( argc, argv );
	make_absolute( path_to_log_file );
	make_absolute( dataset_path );

	make_absolute( temp_fpath );
	instantiation_fd = open( temp_fpath, O_RDWR , S_IRUSR | S_IWUSR );

	if( instantiation_fd == -1 )
	{
		//fprintf( log_file, "program instantiated\n");
		//printf("program instantiated\n");
		instantiation_fd = open( temp_fpath, O_RDWR | O_CREAT , S_IRUSR | S_IWUSR );
	}
	else
	{
		printf( "ERROR: File already exist as a deamon\n" );
	    close( instantiation_fd );
		exit( 0 );
	}



	my_daemon();

	signal(SIGINT, handle_sigint);

	int fd = open( path_to_log_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR );
	close(fd);
	log_file = fopen( path_to_log_file, "w");



	fprintf( log_file, "Executing with parameters:\n");
	fprintf( log_file,"-p %d\n", port );
	fprintf( log_file,"-o %s\n", path_to_log_file );
	fprintf( log_file,"-l %d\n", pool_size );
	fprintf( log_file,"-d %s\n", dataset_path );


	fprintf( log_file, "Loading dataset...\n");
	table = get_table( dataset_path, &tnum_of_row, &tnum_of_col );

	fprintf( log_file, "Dataset loaded in 0.25 seconds with %d records.\n", tnum_of_row );


	//printf( "num_of_row: %d\n", tnum_of_row );
	//printf( "num_of_column: %d\n", tnum_of_col );	

	stack_size = 0;
    void * res;
	int s;



    int nums[2000];
    fprintf( log_file, "A pool of %d threads has been created\n", pool_size );


    for( int i = 0; i < pool_size; i++ )
    {
    	nums[i] = i;
	    s = pthread_create( &(thread_pool[i]) , NULL,
	                      &thread_of_pool,  &(nums[i]) );    	
    }





	int server_fd, new_socket, valread;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};


	
	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	// Forcefully attaching socket to the port port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
												&opt, sizeof(opt)))
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( port );


	
	// Forcefully attaching socket to the port port
	if (bind(server_fd, (struct sockaddr *)&address,
								sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	if (listen(server_fd, 3) < 0)
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}
	


	while( 1 )
	{
		//printf("HEREEEEEEEEEEEEE\n");
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
						(socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

		//fprintf( log_file, "SOCKET TAKEN\n");
		//printf("SOCKET TAKEN\n");

		pthread_mutex_lock( &lock );
			push( new_socket );
			num_of_busy++;
			pthread_cond_signal( &cond );

			while( num_of_busy >= pool_size )
			{
				fprintf( log_file, "No thread is available! Waiting…\n");
				pthread_cond_wait( &cond_server , &lock );
			}	
		pthread_mutex_unlock( &lock );		
	}

    
    for( int i = 0; i < pool_size; i++ )
    {
    	s = pthread_join( thread_pool[i], &res);   	
    }


	
    fclose( log_file );
    remove( "151044046_temp" );

}

#include "t_pool.c"
#include "get_csv.c"
#include "sql.c"
#include "reader_writer.c"

void
handle_sigint(int sig)
{
	terminate = 1;
	fprintf( log_file, "Termination signal received, waiting for ongoing threads to complete.\n");
	close( instantiation_fd );
	remove( temp_fpath );
	

	pthread_cond_broadcast( &cond );
    void * res;
	int s;
    for( int i = 0; i < pool_size; i++ )
    {
    	s = pthread_join( thread_pool[i], &res);   	
    }

    fclose( log_file );
	free_table();
	fprintf( log_file, "All threads have terminated, server shutting down.\n");
	exit( 0 );
}

void
take_arguments( int argc, char **argv )
{
	//for( int i = 0; i < argc; i++ ) printf("%s\n", argv[i]);

	if( argc != 9 )
	{
		//show_usage();
		printf("Missing arg\n");
		exit(0);
	}

    int number_of_setted_value = 0;
    int c;
    while ((c = getopt (argc, argv, ":p:o:l:d:")) != -1)
    switch (c)
    {
        case 'p':
        	port = atoi( optarg );
            number_of_setted_value++;
            break;
        case 'o':
        	strcpy( path_to_log_file, optarg );
            number_of_setted_value++;
            break;
        case 'l':
        	pool_size = atoi( optarg );
            number_of_setted_value++;
            break;
        case 'd':
        	strcpy( dataset_path, optarg );
            number_of_setted_value++;
            break;
        default:
            printf("ERROR: Undefined parameters dedected in command line arguments\n");
            //show_usage();
            exit( 0 );
    }

    if( number_of_setted_value != 4 )
    {
    	//show_usage();
    	printf("show usage\n");
    	exit( 0 );
    }
}


void
my_daemon()
{
    pid_t pid;

    /* Fork off the parent process */
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* On success: The child process becomes session leader */
    if (setsid() < 0)
        exit(EXIT_FAILURE);



    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Close all open file descriptors */
    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }
}


void
make_absolute( char path[] )
{
	char cwd[1000];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
	   //printf("Current working dir: %s\n", cwd);
	}

	if( path[0] != '/' )
	{
		strcat( cwd, "/" );
		strcat( cwd, path );
		strcpy( path, cwd );
	}

	//printf("path:::::::::: %s\n", path );
}
