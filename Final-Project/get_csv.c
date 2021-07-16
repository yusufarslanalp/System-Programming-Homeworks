/*
int main(int argc, char **argv)
{


	int num_of_row;
	int num_of_column;


	char *** table = get_table( "test_csv.csv", &num_of_row, &num_of_column );

	printf( "num_of_row: %d\n", num_of_row );
	printf( "num_of_column: %d\n", num_of_column );




	// deallocate table
	for (int i = 0; i < num_of_row; i++)
	{
		for (int j = 0; j < num_of_column; j++) {
			free(table[i][j]);
		}
		free(table[i]);
	}
	free(table);

	return 0;
}
*/

char***
get_table( char path[], int * num_of_row, int * num_of_column )
{
	* num_of_row = count_lines( path );
	* num_of_column = count_columns( path );
	int cell_size = 1000;

	char*** table = alloc_table( *num_of_row, *num_of_column, cell_size );



	get_lines( path, *num_of_column, table );

	//print table
	/*for( int i = 0; i < *num_of_row; i++ )
	{
		for( int j = 0; j < *num_of_column; j++ )
		{
			printf("%s\n", table[i][j] );	
		}
		printf("\n\n" );
	}*/

	return table;
}

void
free_table()
{
	for (int i = 0; i < tnum_of_row; i++)
	{
		for (int j = 0; j < tnum_of_col; j++) {
			free(table[i][j]);
		}
		free(table[i]);
	}
	free(table);	
}

void
get_lines( char path[], int num_of_column, char *** table)
{
	int temp;

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
		if( line[ strlen( line ) -1 ] == '\n' )
		{
			line[ strlen( line ) -1 ] = '\0';
		}

		get_cells( line, table[i], &temp );
        i++;		
    }
    //*size = i;	
    fclose(file);/**/
}


int
count_lines( char path[] )
{
    FILE* file;
    char line[1000];
    int line_len;

    file = fopen( path, "r");

    if( file == NULL )
    {
        fprintf( log_file, "File NULL\n");
        exit(0);
    }

    int i = 0;
    while( fgets(line, 1000, file) != NULL )
    {
        if( strlen(line) > 1 )
        {
        	i++;
        }		
    }	
    fclose(file);/**/	
    return i;
}

int
count_columns( char path[] )
{
	char first_line[1000];
	int num_of_column = 0;

    FILE* file;

    file = fopen( path, "r");

    if( file == NULL )
    {
        printf("File NULL\n");
        exit(0);
    }

    int i = 0;
    if( fgets( first_line, 1000, file) == NULL )
    {
    	printf("ERROR in count_columns()\n");
    	exit( 0 );
    }
	if( first_line[ strlen( first_line ) -1 ] == '\n' )
	{
		first_line[ strlen( first_line ) -1 ] = '\0';
	}
	//printf("first_line: %s\n", first_line );

	int len = strlen( first_line );
	for( int i = 0; i < len; i++ )
	{
		if( first_line[i] == ',' )
		{
			num_of_column++;
		}
	}
	num_of_column++;

	return num_of_column;

}

void
get_cells( char line[], char ** cells, int * num_of_column )
{
	//printf("line:-->%s<--", line );
	//printf("HEREEEEEEEEEEEEE\n");
	int column_num = 0;
	//int i = 0;
	int j = 0;
	int line_len = strlen( line );
	for( int i = 0; i < line_len; i++ )  //while( column_num < num_of_column )
	{
		if( line[i] == ',' )
		{
			cells[ column_num ][0] = '\0';
		}
		else if( line[i] == '\0' )
		{
			cells[ column_num ][0] = '\0';
		}		
		else if( line[i] == '"' )
		{
			j = 0;
			i++;
			while( 1 )
			{
				if( line[i] == '"' )
				{
					//printf("111111111111111\n");
					cells[column_num][j] = '\0';
					if( line[i+1] == ',' ) i += 1;
					else if( line[i+1] == '\0' ) i += 1;
					else
					{
						//printf("line[i+1]: %d\n", line[i+1] );
						printf("ERROR in get_cells()\n");
					}
					break;
				}
				else
				{
					//printf("22222222222222222\n");
					cells[column_num][j] = line[i];
					j++;
					i++;
				}	
			}
		}
		else
		{
			j = 0;
			while( 1 )
			{
				if( (line[i] == ',') | line[i] == '\0' )
				{
					//printf("33333333333333333\n");
					cells[column_num][j] = '\0';
					break;
				}
				else
				{
					//printf("444444444444444444444\n");
					cells[column_num][j] = line[i]; ///////////////////////////////////////////////////
					//printf("444444444444444444444.111111111111\n");
					i++;
					j++;
				}	
			}
		}
		column_num++;
	}
	*num_of_column = column_num;
}

char***
alloc_table( int num_of_row, int num_of_column, int line_size )
{
	char*** table = (char***)malloc(num_of_row * sizeof(char**));

	if (table == NULL)
	{
		fprintf(stderr, "Out of memory");
		exit(0);
	}

	for (int i = 0; i < num_of_row; i++)
	{
		table[i] = (char**)malloc(num_of_column * sizeof(char*));

		if (table[i] == NULL)
		{
			fprintf(stderr, "Out of memory");
			exit(0);
		}

		for (int j = 0; j < num_of_column; j++)
		{
			table[i][j] = (char*)malloc(line_size * sizeof(char));
			if (table[i][j] == NULL)
			{
				fprintf(stderr, "Out of memory");
				exit(0);
			}
		}
	}
	return table;
}

/*
int
get_cells( char line[], char cells[][1000], int num_of_column )
{
	int column_num = 0;
	int line_len = strlen( line );
	for( int i = 0; i < line_len; i++ )
	{
		if( line[i] == ',' )
		{
			cells[ column_num ][0] = '\0';
		}
		else if( line[i] == '"' )
		{
			int j = 0;
			i++;
			while( 1 )
			{
				if( line[i] == '"' )
				{
					cells[column_num][j] = '\0';	
					break;
				}
				else()
				{
					cells[column_num][j] = line[i];
					i++;
				}	
			}
		}
		else
		{

		}
		column_num++;

	}

}
*/
