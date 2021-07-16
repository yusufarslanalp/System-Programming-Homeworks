#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/stat.h"
#include "errno.h"
#include "signal.h"
#include "sys/wait.h"
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

volatile sig_atomic_t interrupt = 0;

char haspotatoornot[2];
char nameofsharedmemory[256];
char filewithfifonames[256];
char namedsemaphore[256];

int fds[100];
int read_fd;
int fifo_index = -1; //read fifo
int shm_fd;
sem_t * sem;

void
show_usage()
{
	printf(" Error: command line arguments are missing/invalid\n" );
    printf( "Usage Pattern: ./player –b haspotatoornot –s nameofsharedmemory –f " );
    printf( "filewithfifonames –m namedsemaphore\n" );
    printf("Sample Usage: ./hw3 -b 10 -s shm -f fifo_paths.txt -m sem\n");
    exit( 0 );	
}


void
take_arguments( int argc, char **argv )
{
	//for( int i = 0; i < argc; i++ ) printf("%s\n", argv[i]);

	if( argc != 9 )
	{
		show_usage();
		exit(0);
	}

    int number_of_setted_value = 0;
    int c;
    while ((c = getopt (argc, argv, ":b:s:f:m:")) != -1)
    switch (c)
    {
        case 'b':
            strcpy( haspotatoornot, optarg );
            number_of_setted_value++;
            break;
        case 's':
            strcpy( nameofsharedmemory, optarg );
            number_of_setted_value++;
            break;
        case 'f':
            strcpy( filewithfifonames, optarg );
            number_of_setted_value++;
            break;
        case 'm':
            strcpy( namedsemaphore, optarg );
            number_of_setted_value++;
            break;

        default:
            printf("ERROR: Undefined parameters dedected in command line arguments\n");
            show_usage();
            exit( 0 );
    }


    if( number_of_setted_value != 4 )
    {
    	show_usage();
    	exit( 0 );
    }
}

void
get_lines( char path[], char lines[][1000], int * size  )
{

	FILE* file;
	int line_len;

	file = fopen( path, "r");

	if( file == NULL )
	{
		printf("File NULL\n");
		exit(0);
	}

	int i = 0;
	while( fgets(lines[i], 1000, file) != NULL )
	{
		line_len = strlen(lines[i]);
		if( lines[i][line_len-1] == '\n' )
		{
			lines[i][line_len-1] = '\0';
		}
		i++;

	}

	*size = i;
	fclose(file);/**/

}

int
rand_index( int num_of_line, int line_index )
{
	int add_value;

	if( line_index == 0 ) add_value = +1;
	else add_value = -1;

	int result = rand() % num_of_line;
	if( result == line_index )
	{
		result += add_value;
	}
	return result;
}

void
handle_sigint(int sig)
{
	interrupt = 1;
}

void
terminate( int num_of_line )
{
	for( int i = 0; i < num_of_line; i++ )
	{
		if( fifo_index == i )
		{
			close ( read_fd );
		}
		else close( fds[i] );
	}	

	int temp = -1;
	for( int j = 0; j < num_of_line; j++ )
	{
		if( j != fifo_index )
		{
			write (fds[j], &temp, sizeof(int));
		}
		
	}	

	close( shm_fd );
	sem_close( sem );
	shm_unlink( nameofsharedmemory );
	sem_unlink( namedsemaphore );
	exit(0);
}

int
main( int argc, char **argv )
{
	signal(SIGINT, handle_sigint);
	srand(time(NULL));  

	take_arguments( argc, argv );

	int patato_id = getpid();
	int temp_pid;
	int hotness = atoi( haspotatoornot );
	int initial_hotness = hotness;
	int all_cool = 0;
	int current_patatoe_hot;
	int offset_pid;
	int number_of_cool;

	if( hotness > 0 )
	{
		current_patatoe_hot = 1;
	}
	else current_patatoe_hot = 0;

	char lines[100][1000];
	int num_of_line; //num of process


	get_lines( filewithfifonames, lines, &num_of_line );


	int is_taken;
	int write_fifo;
	int temp;


	
	sem = sem_open(  namedsemaphore, O_CREAT, S_IRUSR | S_IWUSR, 1 );	


	//shm_unlink( nameofsharedmemory );
	//sem_unlink( namedsemaphore );
	//exit(0);

	void* ptr;
	shm_fd = shm_open( nameofsharedmemory , O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd, 10000 );
	ptr = mmap(0, 10000, PROT_WRITE, MAP_SHARED, shm_fd, 0);



	sem_wait( sem );

	//every process selects a fifo file
	for( int i = 0; i < num_of_line; i++ ) 
	{
		memcpy( &is_taken, ptr + sizeof(int)*i , sizeof(int));
		if( is_taken == 0 )
		{
			temp = 1;
			memcpy( ptr + sizeof(int)*i , &temp, sizeof(int));
			fifo_index = i;

			//write patato_id, hotness and initial_hotness to shm 
			offset_pid = (100 + 3*i)*sizeof(int);
			memcpy( ptr + offset_pid , &patato_id, sizeof(int));
			memcpy( ptr + offset_pid +sizeof(int) , &hotness, sizeof(int));
			//initial hotness
			memcpy( ptr + offset_pid + 2*sizeof(int) , &hotness, sizeof(int));
			break;
		}
	}

	if( fifo_index == -1 )
	{
		printf("Error occured while searching fifo index\n");
		sem_close( sem );
		close( shm_fd );
		shm_unlink( nameofsharedmemory );
		sem_unlink( namedsemaphore );
		exit(0);
	}

	//create all fifo files
	if( fifo_index == 0 )
	{
		for( int i = 0; i < num_of_line; i++ )
		{
			mkfifo( lines[i], S_IRUSR | S_IWUSR );
		}
	}
	sem_post( sem );


	//open all fifo files.
	for( int i = 0; i < num_of_line; i++ )
	{
		if( fifo_index == i )
		{
			read_fd = open ( lines[fifo_index], O_APPEND);
			lseek( read_fd, 0, SEEK_SET );
		}
		else
		{
			fds[i] = open( lines[i], O_WRONLY);
		}
	}

	if( current_patatoe_hot == 0 )
	{
		current_patatoe_hot = 0;
		memcpy( &number_of_cool, ptr + sizeof(int)*400 , sizeof(int));
		number_of_cool++;
		memcpy( ptr + sizeof(int)*400 , &number_of_cool, sizeof(int));
	}


	while( all_cool == 0 )
	{
		if( interrupt )
		{
			terminate( num_of_line );
		}

		//send patatoe pid
		if( current_patatoe_hot == 1  ) 
		{
			write_fifo = rand_index( num_of_line, fifo_index );
			printf("pid=%d sending potato number %d to %s; this is switch number %d\n",
				getpid(), patato_id, lines[write_fifo], initial_hotness - hotness );
			write( fds[write_fifo], &patato_id, sizeof(int) );
		}

		//get patatoe pid
		read( read_fd, &patato_id, sizeof(int) );
		//all patatoes are cool.
		if( patato_id == -1 )
		{
			break;	
		}
		printf("pid=%d receiving potato number %d from %s\n", getpid(), patato_id, lines[fifo_index] );
		
		//receive patato always hot
		current_patatoe_hot = 1;

		//decrement value
		sem_wait( sem );
		for( int i = 0; i < num_of_line; i++ ) 
		{
			offset_pid = (100 + 3*i)*sizeof(int);
			memcpy( &temp_pid, ptr + offset_pid , sizeof(int));
			//printf("TEMP PID: %d\n", temp_pid );

			if( temp_pid == patato_id )
			{
				memcpy( &initial_hotness, ptr + offset_pid + 2*sizeof(int) , sizeof(int));
				memcpy( &hotness, ptr + offset_pid + sizeof(int) , sizeof(int));
				hotness--;
				memcpy(ptr + offset_pid + sizeof(int) , &hotness, sizeof(int));
				//printf("HOTNESS in loop: %d\n", hotness );
				if( hotness <= 0 )
				{
					printf("pid=%d; potato number %d has cooled down.\n", getpid(), patato_id );
					current_patatoe_hot = 0;
					memcpy( &number_of_cool, ptr + sizeof(int)*400 , sizeof(int));
					number_of_cool++;
					memcpy( ptr + sizeof(int)*400 , &number_of_cool, sizeof(int));
				}
				if( number_of_cool == num_of_line )
				{
					printf("All the potatoes are cold now.\n");
					//send all process -1. So they terminate itself.
					all_cool = 1;
					temp = -1;
					for( int j = 0; j < num_of_line; j++ )
					{
						if( j != fifo_index )
						{
							write (fds[j], &temp, sizeof(int));
						}
						
					}	
				}
				break;
			}
		}
		sem_post( sem );
	}


	terminate( num_of_line );
	return 0;

}
