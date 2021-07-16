void
reader( char query[], int fd )
{
	pthread_mutex_lock( &m );
	while ((AW + WW) > 0)
	{ 
		WR++;
		pthread_cond_wait( &ok_to_read , &m );
		WR--;
	}
	AR++;
	pthread_mutex_unlock( &m );
		respond( query, fd );
	pthread_mutex_lock( &m );
	AR--;
	if (AR == 0 && WW > 0)
	{
		pthread_cond_signal( &ok_to_write );
	}
	pthread_mutex_unlock( &m );
}

void
writer( char query[], int fd )
{
	pthread_mutex_lock( &m );
	while ((AW + AR) > 0)
	{ 
		WW++;
		pthread_cond_wait( &ok_to_write , &m );
		WW--;
	}
	AW++;
	pthread_mutex_unlock( &m );
		respond( query, fd );
	pthread_mutex_lock( &m );
	AW--;
	if (WW > 0)
	{
		pthread_cond_signal( &ok_to_write );
	}
	else if (WR > 0)
	{
		pthread_cond_broadcast( &ok_to_read );
	}
	pthread_mutex_unlock( &m );
}

void
reader_writer( char query[], int fd )
{
	if( is_reader( query ) )
	{
		reader( query, fd );
	}
	else
	{
		writer( query, fd );
	}
}