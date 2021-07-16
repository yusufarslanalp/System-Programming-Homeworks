void
parse( char str[], char delim, char parsed[][100], int * word_count );

void
clear_comma( char words[][100], int w_size );

void
get_type( char first_word[], char second_word[], char type[] );

void
get_colname_value( char str[], char col_name[] , char value[] );

int
contains( char str[], char ch );

void
select_column( char *** table, int t_rsize, int t_csize, char query[], int fd );

int
is_distinct_rows( char*** table, int col_indexes[], int ci_size, int r_index1, int r_index2 );

int
is_distinct( char*** table, int col_indexes[], int ci_size, int r_index );

void
select_distinct( char *** table, int t_rsize, int t_csize, char query[], int fd );

void
update( char *** table, int t_rsize, int t_csize, char query[], int fd );

int
is_reader( char query[] );

void
select_star( char *** table, int t_rsize, int t_csize, char query[], int fd );

void
respond( char query[], int fd );

