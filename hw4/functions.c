static void *
student_h(void *arg)
{
    char * s = (char *) arg;
    
    read_hws( hw_fpath );

    //printf("%s\n", s );
    return (void *) strlen( s );
}

static void *
student_for_hire(void *arg)
{
    //char * s = (char *) arg;
    //printf("%s\n", s );
    
	int thread_num = *((int *)( arg));
	//printf("thread_num: %d\n", thread_num );
	int sleep_time;




	while( 1 )
	{
		printf("%s is waiting for a homework\n", (std_table[ thread_num ]).university );
		sem_wait( &(sfh_sems[ thread_num ]) );
		if( wait_for_assignment == 1 )  ///student_for_hire???
		{
			sem_post( &hw_taken );
			(std_table[ thread_num ]).completed_hw = (std_table[ thread_num ]).completed_hw + 1;
			printf("%s is solving homework %c for %d, H has %dTL left.\n",
					 (std_table[ thread_num ]).university,
					 (std_table[ thread_num ]).current_hw,
					 (std_table[ thread_num ]).monney, remaining_money);
			sleep_time = 6 - ((std_table[ thread_num ]).speed);
			sleep( sleep_time );
			printf("Student %s got the homework Sleep time: %d\n", (std_table[ thread_num ]).university, sleep_time );
			(std_table[ thread_num ]).available = 1;
			sem_post( &sem_available_std );
		}
		else
		{
			//sem_post( hw_taken );
			break;
		}



	}

    return NULL;
}



void
read_hws( char * fname )
{
    char chr;
    int num_of_byte;
    int fd = open( fname, O_RDONLY );

    if( fd == -1 )
    {
        perror( "read_hws" );
        exit( 0 );
    }

    while( 1 )
    {
        num_of_byte = read( fd, &chr, 1 );
        if( num_of_byte < 1 ) break;

        add( chr );

        //printf("H pushed %c to the queue\n", chr );
        if( no_monney == 1 )
        {
        	break;
        }
        printf("H has a new homework %c; remaining money is %dTL\n", chr, remaining_money );


    }

    add( 'D' ); //Done
    if( no_monney == 1 )
    {
    	printf("H has no more money for homeworks, terminating.\n" );
    }
    else
    {
    	printf("H has no other homeworks, terminating.\n");
    }
    //printf("All hws are readed. char D is added queue for termination.\n");
}

void
get_students( char path[] )
{

    FILE* file;
    char line[1000];
    int line_len;

    file = fopen( path, "r");

    if( file == NULL )
    {
        printf("File NULL\n");
        exit(0);
    }

    int i = 0;
    while( fgets(line, 1000, file) != NULL )
    {
        line_len = strlen(line);
        if( line_len >= 7 )
        {
            sscanf( line, "%s %d %d %d", (std_table[i]).university,
                                        &((std_table[i]).quality),
                                        &((std_table[i]).speed),
                                        &((std_table[i]).monney) );
        	(std_table[i]).available = 1;
        	(std_table[i]).completed_hw = 0;
        }
        //printf("%s", line );
        i++;
    }

    std_table_size = i;
    fclose(file);/**/

}

int
find_suitable_std( char priority )
{
	int suit_std_index = -1;
	int no_available = 1;
	for( int i = 0; i < std_table_size; i++ )
	{
		if( std_table[i].available == 1 )
		{
			no_available = 0;
			if( std_table[i].monney < remaining_money )
			{
				if( suit_std_index == -1 )
				{
					suit_std_index = i;
				}
				else
				{
					if( priority == 'Q' )
					{
						if( std_table[ i ].quality > 
							std_table[ suit_std_index ].quality)
						{ suit_std_index = i; }
					}
					else if ( priority == 'S' )
					{
						if( std_table[ i ].speed > 
							std_table[ suit_std_index ].speed)
						{ suit_std_index = i; }						
					}
					else if ( priority == 'C' )
					{
						if( std_table[ i ].monney < 
							std_table[ suit_std_index ].monney)
						{ suit_std_index = i; }						
					}
					else
					{
						printf("priority could not be other then Q, S or C characters.\n");
						my_exit();
					}
				}

			}
		}

	}
	if( no_available == 1 )
		printf("No ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::available\n");	
	return suit_std_index;
}


void
my_exit()
{
	exit(EXIT_SUCCESS);
}

void
add( char hw )
{
    if (sem_wait(&sem_queue) == -1)
    {
    	perror("sem_wait");
    }
            
    	queue[tail] = hw;
    	tail++;


    if (sem_post(&sem_queue) == -1)
        perror("sem_post");
    if (sem_post(&sem_has_hw) == -1)
        perror("sem_wait");
}

char
poll()
{
	char new_hw;

    if (sem_wait(&sem_has_hw) == -1)
        perror("sem_wait");	
    if (sem_wait(&sem_queue) == -1)
    {
    	perror("sem_wait");
    }
            
    	new_hw = queue[head];
    	head++;


    if (sem_post(&sem_queue) == -1)
        perror("sem_post");

    return new_hw;
}

void
handle_sigint(int sig)
{
	my_exit();
}