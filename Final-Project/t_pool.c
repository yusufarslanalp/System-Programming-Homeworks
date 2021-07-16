
void
push( int num )
{
	stack[ stack_size ] = num;
	stack_size++;
}

int
pop(  )
{
	if( stack_size != 0 )
	{
		stack_size--;
		return stack[ stack_size ];
	}
	else
	{
		printf("ERROR: attempting to pop from an empty stack\n");
		exit( 0 );
	}
}

static void *
thread_of_pool(void *arg)
{
    int t_id = *((int *)arg);
    int temp;
    int res;
    int socket;
    char query[1024];
    char response[1024] = "server response";
    int response_len;
    int zero = 0;
 
    fprintf( log_file, "Thread #%d: waiting for connection\n", t_id );

    while( 1 )
    {
        pthread_mutex_lock( &lock );
        pthread_cond_wait( &cond , &lock ); //automatically relase the lock and goes to sleep. after woken locks the mutex again
        	socket = pop();
	        fprintf( log_file, "A connection has been delegated to thread id #%d\n", t_id );
        pthread_mutex_unlock( &lock );

        if( terminate == 1 )
        {
            break;
        }

        while( read( socket , query, 1024) != 0 )
        {
        	fprintf( log_file, "Thread #%d: received query \'%s\'\n", t_id, query );
            usleep( 500000 );
            fprintf( log_file, "Thread #%d: query completed, ", t_id );
            reader_writer( query, socket );
            //response_len = strlen( response );
        	//write( socket , &response_len , sizeof( int ) );
            //write( socket , response, response_len );
            //write( socket , &zero, sizeof( int ) );
        }
        //break;
        pthread_mutex_lock( &lock );
            num_of_busy--;
            pthread_cond_signal( &cond_server );            
        pthread_mutex_unlock( &lock );           	

		
    }	

  
	//printf("id: %d\n", t_id ); //for printing id properly comment line 18 (printing tem value)
  	//exit( 0 );
}