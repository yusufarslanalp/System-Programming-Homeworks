#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


void
get_lines( char path[], int num_of_column, char *** table); //, char queries[][1000], int * size, int id

void
get_cells( char line[], char ** cells, int * num_of_column );

int
count_lines( char path[] );

int
count_columns( char path[] );

char***
alloc_table( int num_of_row, int num_of_column, int line_size );

char***
get_table( char path[], int * num_of_row, int * num_of_column );

void
free_table();

