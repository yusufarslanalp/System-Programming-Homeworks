#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

// Dynamically allocate memory for 3D Array
int main()
{
	int num_of_row = 2;
	int num_of_column = 3;

	char*** table = alloc_table( 2, 3, 400 );

	// assign values to the allocated memory
	for (int i = 0; i < num_of_row; i++)
	{
		for (int j = 0; j < num_of_column; j++)
		{
			table[i][j] = strcpy( table[i][j], "asd" );
		}
	}

	// print the 3D array
	for (int i = 0; i < num_of_row; i++)
	{
		for (int j = 0; j < num_of_column; j++)
		{
			printf("%s   ", table[i][j] );
		}
		printf("\n");
	}

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