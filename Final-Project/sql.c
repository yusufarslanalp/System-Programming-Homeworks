void
parse( char str[], char delim, char parsed[][100], int * word_count )
{
	*word_count = 0;
	int i = 0;
	int j = 0;

	while( str[i] != '\0' )
	{

		if( str[i] == delim )
		{
			parsed[ *word_count ][j] = '\0';
			*word_count = (*word_count) +1;
			j = 0;
			i++;
		}
		else
		{
			parsed[ *word_count ][j] = str[i];
			j++;
			i++;
		}	
	}	
	parsed[ *word_count ][j] = '\0';
	*word_count = (*word_count) +1;
}


void
clear_comma( char words[][100], int w_size )
{
	int current_len;
	for( int i = 0; i < w_size; i++ )
	{
		current_len = strlen( words[i] );
		if( words[i][ current_len-1 ] == ',' )
		{
			words[i][ current_len-1 ] = '\0';	
		}

	}
}

void
get_type( char first_word[], char second_word[], char type[] )
{
	if( strcmp( first_word, "UPDATE" ) == 0 )
	{
		strcpy( type, "update" );
		return;
	}
	else if( strcmp( first_word, "SELECT" ) == 0 )
	{
		if( strcmp( second_word, "*" ) == 0 )
		{
			strcpy( type, "select_star" );
		}
		else if( strcmp( second_word, "DISTINCT" ) == 0 )
		{
			strcpy( type, "select_distinct" );
		}
		else
		{
			strcpy( type, "select_column" );
		}					
	}
	else
	{
		printf("Undefined Type\n");
		exit( 0 );
	}
}

void
get_colname_value( char str[], char col_name[] , char value[] )
{
	int i = 0;
	int j = 0;
	while( str[i] != '=' )
	{
		col_name[j] = str[i];
		i++;
		j++;
	}
	col_name[j] = '\0';

	while( str[i] != '\'' )
	{
		i++;
	}
	i++;


	j = 0;
	while( str[i] != '\'' )
	{
		value[j] = str[i];
		j++;
		i++;
	}
	value[j] = '\0';

}

int
contains( char str[], char ch )
{
	int len = strlen( str );
	for( int i = 0; i < len; i++ )
	{
		if( str[i] == ch ) return 1;
	}
	return 0;
}


void
select_column( char *** table, int t_rsize, int t_csize, char query[], int fd )
{
	char column_names[30][100];
	int column_size;
	int column_indexes[30];
	char parsed[30][100];
	int parsed_size;
	parse( query, ' ',  parsed, &parsed_size );
	clear_comma( parsed, parsed_size );

	int response_len;
	char line[1024];
	line[0] = '\0';
	char space[2] = " ";
	int zero = 0;	

	for (int i = 2; i < parsed_size-2; ++i)
	{
		strcpy( column_names[i-2], parsed[i] );
	}
	column_size = parsed_size-4;

	int k = 0;
	for (int i = 0; i < column_size; ++i)
	{
		for( int j = 0; j < t_csize; j++ )
		{
			if( strcmp( column_names[i], table[0][j] ) == 0 )
			{
				column_indexes[k] = j;
				k++;
				break;
			}	
		}
		//printf("col[i]: %s\n", column_names[i] );
	}
	//printf("k: %d\n", k );
	if( k != column_size )
	{
		fprintf( log_file, "ERROR\n");
		exit(0);
	}

	for( int i = 0; i < t_rsize; i++ )
	{
		line[0] = '\0';
		for (int j = 0; j < column_size; ++j)
		{
			//printf( "%s    ", table[i][ column_indexes[j] ] );	
            strcat(line, table[i][ column_indexes[j] ] );
            strcat(line,  space );				
		}
		//printf( "\n" );
        response_len = strlen( line ) + 1;
    	write( fd , &response_len , sizeof( int ) );
        write( fd , line, response_len );		
	}
	write( fd , &zero, sizeof( int ) ); //response completely sended
	fprintf( log_file, "%d records have been returned.\n", (tnum_of_row-1) );
}

int
is_distinct_rows( char*** table, int col_indexes[], int ci_size, int r_index1, int r_index2 )
{
	for( int i = 0; i < ci_size; i++ )
	{
		if( strcmp( table[r_index1][ col_indexes[i] ],
					table[r_index2][ col_indexes[i] ] ) != 0 )
		{
			return 1;
		}	
	}
	return 0;
}

int
is_distinct( char*** table, int col_indexes[], int ci_size, int r_index )
{
	for( int i = 1; i < r_index; i++ )
	{
		if( is_distinct_rows( table, col_indexes, ci_size, r_index, i ) == 0 )
		{
			return 0;
		}	
	}
	return 1;
}

void
select_distinct( char *** table, int t_rsize, int t_csize, char query[], int fd )
{
	char column_names[30][100];
	int column_size;
	int column_indexes[30];
	char parsed[30][100];
	int parsed_size;

	int response_len;
	char line[1024];
	line[0] = '\0';
	char space[2] = " ";
	int zero = 0;

	int nof_returned_line = 0;

	parse( query, ' ',  parsed, &parsed_size );
	clear_comma( parsed, parsed_size );

	for (int i = 3; i < parsed_size-2; ++i)
	{
		strcpy( column_names[i-3], parsed[i] );
	}
	column_size = parsed_size-5;

	int k = 0;
	for (int i = 0; i < column_size; ++i)
	{
		for( int j = 0; j < t_csize; j++ )
		{
			if( strcmp( column_names[i], table[0][j] ) == 0 )
			{
				column_indexes[k] = j;
				k++;
				break;
			}	
		}
		//printf("col[i]: %s\n", column_names[i] );
	}
	//printf("k: %d\n", k );
	if( k != column_size )
	{
		printf("ERROR\n");
		exit(0);
	}



	for( int i = 0; i < t_rsize; i++ )
	{
		if( is_distinct( table, column_indexes, column_size, i ) )
		{
			line[0] = '\0';
			for (int j = 0; j < column_size; ++j)
			{
				//printf( "%s    ", table[i][ column_indexes[j] ] );	
	            strcat(line, table[i][ column_indexes[j] ]  );
	            strcat(line,  space );				
			}
			//printf( "\n" );
	        response_len = strlen( line ) + 1;
	    	write( fd , &response_len , sizeof( int ) );
	        write( fd , line, response_len );
	        nof_returned_line++;				
		}	
	}
	write( fd , &zero, sizeof( int ) ); //response completely sended
	fprintf( log_file, "%d records have been returned.\n", (nof_returned_line-1) );
}

void
update( char *** table, int t_rsize, int t_csize, char query[], int fd )
{
	char column_names[30][100];
	char values[30][1000];	
	int column_size = 0; //size of column_names and values
	int column_indexes[30];
	char parsed[30][100];
	int parsed_size;

	int response_len;
	char line[1024];
	line[0] = '\0';
	char space[2] = " ";
	int zero = 0;

	int nof_returned_line = 0;

	parse( query, ' ',  parsed, &parsed_size );
	//clear_comma( parsed, parsed_size );

	for (int i = 0; i < parsed_size; ++i)
	{
		//printf("parsed[i]: %s\n", parsed[i] );
		if( contains( parsed[i], '=' ) )
		{
			get_colname_value( parsed[i], column_names[ column_size ] , values[ column_size ] );
			column_size++;
		}	
	}

	/*for (int i = 0; i < column_size; ++i)
	{
		printf("col: %-20s   values: %s\n", column_names[i], values[i] );
	}*/

	int k = 0;
	for (int i = 0; i < column_size; ++i)
	{
		for( int j = 0; j < t_csize; j++ )
		{
			if( strcmp( column_names[i], table[0][j] ) == 0 )
			{
				column_indexes[k] = j;
				k++;
				break;
			}	
		}
		//printf("col[i]: %s\n", column_names[i] );
	}
	//printf("k: %d\n", k );
	if( k != column_size )
	{
		printf("ERROR\n");
		exit(0);
	}

	//send column names(first row)
	line[0] = '\0';			
	for( int j = 0; j < t_csize; j++ )
	{
        strcat(line, table[0][j]);
        strcat(line,  space );					
	}
    response_len = strlen( line ) + 1;
	write( fd , &response_len , sizeof( int ) );
    write( fd , line, response_len );

	int search_col = column_indexes[ column_size-1 ];
	for (int i = 1; i < t_rsize; ++i)
	{
		if( strcmp( table[i][ search_col ], values[ column_size-1 ] ) == 0 )
		{
			for( int j = 0; j < column_size-1; j++ )
			{
				strcpy( table[i][ column_indexes[j] ], values[j] );
			}


			line[0] = '\0';			
			for( int j = 0; j < t_csize; j++ )
			{
	            strcat(line, table[i][j]);
	            strcat(line,  space );					
			}
	        response_len = strlen( line ) + 1;
	    	write( fd , &response_len , sizeof( int ) );
	        write( fd , line, response_len );
	        nof_returned_line++;	
		}
	}
	write( fd , &zero, sizeof( int ) ); //response completely sended
	fprintf( log_file, "%d records have been returned.\n", (nof_returned_line-1) );
}

int
is_reader( char query[] )
{
	char parsed[30][100];
	int parsed_size;
	parse( query, ' ',  parsed, &parsed_size );
	char query_type[30];
	get_type( parsed[1], parsed[2], query_type );

	if( strcmp( query_type, "update" ) == 0 )
	{
		return 0;
	}
	return 1;
}

void
select_star( char *** table, int t_rsize, int t_csize, char query[], int fd )
{
	int response_len;
	char line[1024];
	line[0] = '\0';
	char space[2] = " ";
	int zero = 0;
	for (int i = 0; i < t_rsize; ++i)
	{
		line[0] = '\0';
		for ( int j = 0; j < t_csize; ++j )
		{
			//printf("%s ", table[i][j] );
            strcat(line, table[i][j]);
            strcat(line,  space );			
		}
		//printf("\n");
		//printf( "%s\n", line );
        response_len = strlen( line ) + 1;
    	write( fd , &response_len , sizeof( int ) );
        write( fd , line, response_len );

	}
	write( fd , &zero, sizeof( int ) ); //response completely sended
	fprintf( log_file, "%d records have been returned.\n", (tnum_of_row-1) );
}

void
respond( char query[], int fd )
{
	//printf("tnum_of_row: %d\n", tnum_of_row );
	//exit( 0 );

	char parsed[30][100];
	int parsed_size;
	parse( query, ' ',  parsed, &parsed_size );

	char query_type[30];
	get_type( parsed[1], parsed[2], query_type );
	//printf("TYPE: %s\n", query_type );

	if( strcmp( query_type, "update" ) == 0 )
	{
		update( table, tnum_of_row, tnum_of_col, query, fd );
	}
	else if( strcmp( query_type, "select_star" ) == 0 )
	{
		select_star( table, tnum_of_row, tnum_of_col, query, fd );
	}
	else if( strcmp( query_type, "select_distinct" ) == 0 )
	{
		select_distinct( table, tnum_of_row, tnum_of_col, query, fd );
	}
	else if( strcmp( query_type, "select_column" ) == 0 )
	{
		select_column( table, tnum_of_row, tnum_of_col, query, fd );
	}
	else
	{
		printf("ERROR\n");
		exit( 0 );
	}				
}



/*
int main(int argc, char **argv)
{
	char type[100];
	char first_word[100];
	char second_word[100];

	char * q1 = "SELECT * FROM TABLE;";
	char * q2 = "SELECT columnName1, columnName2, columnName3 FROM TABLE;";
	char * q3 = "UPDATE TABLE SET columnName1=’value1’, columnName2=’value2’ WHERE columnName=’valueX’";
	char * q4 = "SELECT DISTINCT columnName1,columnName2 FROM TABLE;";

	//get_two_word( q4, first_word, second_word );
	//get_type( first_word, second_word, type );

	//printf( "type: %s\n", type );

	char parsed[30][100];
	int word_count;
	parse( q3, ' ', parsed, &word_count );
	clear_comma( parsed, word_count );
	for( int i = 0; i < word_count; i++ )
	{
		printf("%s\n", parsed[i] );

	}	

	printf("\n\n\n");
	char col_name[100];
	char value[100];
	get_colname_value( "columnName1='value1'", col_name, value );
	printf("%s\n", col_name );
	printf("%s\n", value );


	char * qn1 = "1 SELECT * FROM TABLE;";
	char * qn2 = "1 SELECT Description, Period FROM TABLE;";
	char * qn3 = "1 UPDATE TABLE SET Description='value1', Revised='value2' WHERE Revised='1184'";
	//char * qn4 = "1 SELECT DISTINCT Description, Period FROM TABLE;";
	char * qn4 = "1 SELECT DISTINCT Period FROM TABLE;";

	printf("\n\n\n");
	printf("is_reader: %d\n", is_reader( qn4 ) );


	int num_of_row;
	int num_of_column;


	char *** table = get_table( "test_csv.csv", &num_of_row, &num_of_column );

	printf( "num_of_row: %d\n", num_of_row );
	printf( "num_of_column: %d\n", num_of_column );	


	printf("\n\n\n");
	//select_column( table, 79, 5, qn2, 0 );
	//select_distinct( table, 79, 5, qn4, 0 );

	update( table, 79, 5, qn3, 0 );

	printf("\n\n\n");
	//print table
	for( int i = 0; i < 79; i++ )
	{
		for( int j = 0; j < 5; j++ )
		{
			printf("%s\n", table[i][j] );
		}
		printf("\n" );
	}


}
*/


