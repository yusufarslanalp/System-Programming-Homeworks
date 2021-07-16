#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<signal.h>

char search_dir[256] = "";
char target_fname[256] = "";
int target_fsize;
char target_ftype;
char target_fperm[256];
int target_num_of_link;
int file_found = 0;

int search_path_setted = 0;
int fname_setted = 0;
int fsize_setted = 0;
int ftype_setted = 0;
int fperm_setted = 0;
int num_of_link_setted = 0;


typedef struct
{
    char fname[256];
    int fsize;
    char ftype;
    char fperm[256];
    int num_of_link;
}file_property;

typedef struct dir_entry_s{
    char fname[256];
    int size;
    int capacity;
    struct dir_entry_s * sub_entries;

}dir_entry;

dir_entry root;

void
print_output_tree( dir_entry parent, int depth );

void
corvvert_to_list( char * path, char list[][256], int * size );

void
add_to_tree( dir_entry * root, char path[][256], int path_size );

void
add_entry( dir_entry * entry, char * new_entry_name );

int
get_index( dir_entry * parent, char * entry_name );

void
print_fproperty( file_property property );

int
is_target_file( file_property property );

int
is_match( char syntax[], char exp[] );

char
convert_meaningful_char( unsigned char d_type );

void
find_property( struct stat file_stat, char permissions[], int * num_of_link, int * size );

int
walk_in_dir(const char *path);

void
take_arguments( int argc, char **argv );

void
free_tree( dir_entry parent );

void
show_usage();

void
handle_sigint(int sig);

void
make_lover_case( char mix[], char lower[] );

int
main( int argc, char **argv )
{
    signal(SIGINT, handle_sigint);
    root.sub_entries = NULL;

    take_arguments( argc, argv );

    walk_in_dir( search_dir );
    strcpy( root.fname, search_dir );
    print_output_tree( root, 0 );
    free_tree( root );

    return 0;
}

void
make_lover_case( char mix[], char lower[] )
{
    int len = strlen( mix );

    int i;
    for( i = 0; i < len; i++ )
    {
        if( (mix[i] >= 'A') & (mix[i] <= 'Z') )
        {
            lower[i] = mix[i] + ('a' - 'A');
        }
        else
        {
            lower[i] = mix[i];
        }

    }
    lower[i] = '\0';

}

void
show_usage()
{
    /*printf( "The search criteria can be any combination of the following" );
    printf( " (at least one of them must beemployed):\n" );
    printf( "-f : filename (case insensitive), supporting the following regular expression: +\n" );
    printf( "-b : file size (in bytes)\n" );
    printf( "-t : file type\n" );
    printf( "-p : permissions, as 9 characters (e.g. ‘rwxr-xr--’)\n" );
    printf( "-l: number of links\n\n" );*/
    
    printf( "Example usage: ./a.out -w /home/yusuf/Desktop/System/HW1/test_dir -f 'f3'\n" );
}

void
take_arguments( int argc, char **argv )
{
    int number_of_setted_value = 0;
    int c;
    while ((c = getopt (argc, argv, ":w:f:b:t:p:l:")) != -1)
    switch (c)
    {
        case 'w':
            strcpy( search_dir, optarg );
            search_path_setted = 1;
            number_of_setted_value++;
            break;
        case 'f':
            fname_setted = 1;
            strcpy( target_fname, optarg );
            number_of_setted_value++;
            break;
        case 'b':
            fsize_setted = 1;
            sscanf( optarg, "%d", &target_fsize);
            number_of_setted_value++;
            break;
        case 't':
            ftype_setted = 1;
            target_ftype = optarg[0];
            number_of_setted_value++;
            break;
        case 'p':
            fperm_setted = 1;
            strcpy( target_fperm, optarg );
            number_of_setted_value++;
            break;
        case 'l':
            num_of_link_setted = 1;
            sscanf( optarg, "%d", &target_num_of_link);
            number_of_setted_value++;
            break;

        default:
            fprintf(stderr, "Undefined parameters dedected\n");
            show_usage();
            exit( 0 );
    }

    if( search_path_setted == 0 )
    {
        fprintf(stderr, "You must specify a path with -w parameter.\n");
        show_usage();
        exit( 0 );
    }
    if( number_of_setted_value < 2 )
    {
        fprintf(stderr, "At least one search criteria must be employed\n");
        show_usage();
        exit( 0 );
    }

}

void
handle_sigint(int sig)
{
    printf( "Ctrl + z pressed\n" );
    free_tree( root );
    printf( "all resources returned to the system" );
    exit( 0 );
}

void
print_output_tree( dir_entry parent, int depth )
{
    if( file_found == 0 )
    {
        printf( "No file found\n" );
        return;
    }
    dir_entry child_entry;

    for( int i = 0; i < depth; i++ ) printf( "--" );
    printf( "%s\n", parent.fname );

    for( int i = 0; i < parent.size; i++ )
    {
        child_entry = (parent.sub_entries)[i];
        print_output_tree( child_entry, depth+1 );
    }
}

void
free_tree( dir_entry parent )
{
    dir_entry child_entry;

    for( int i = 0; i < parent.size; i++ )
    {
        child_entry = (parent.sub_entries)[i];
        free_tree( child_entry );
    }
    free( parent.sub_entries );
}

void
corvvert_to_list( char * path, char list[][256], int * size )
{
    char bare_path[4096];
    int j = 0;
    int index = 0;
    int word_len = 0;
    * size = 0;
    int i = 0;
    for( i = strlen( search_dir ); i <= strlen( path ); i++ )
    {
        bare_path[j] = path[i];
        j++;
    }
    bare_path[i] = '\0';

    if( bare_path[0] == '/' ) index++;

    for( int i = index; i < strlen( bare_path ); i++ )
    {
        if( bare_path[i] == '/' )
        {
            //printf( "iiii: %d\n", i );
            list[*size][word_len] = '\0';
            word_len = 0;
            * size = (*size) + 1;
        }
        else
        {
            list[*size][word_len] = bare_path[i];
            word_len++;

        }
    }
    list[*size][word_len] = '\0';
    * size = (*size) + 1;

    if( *size > 100 )
    {
        fprintf(stderr, "ERROR: there are more than 100 nested file.\n");
        exit( 0 );
    }
}

void
add_to_tree( dir_entry * root, char path[][256], int path_size )
{
    int index;
    dir_entry * current_entry;
    current_entry = root;

    for( int i = 0; i < path_size; i++ ) //////////////////
    {
        index = get_index( current_entry, path[i] );
        if( index == -1 )
        {
            add_entry( current_entry, path[i] );
            index = current_entry->size -1;
        }
        current_entry = &((current_entry->sub_entries)[index]);
    }
}

void
add_entry( dir_entry * parent, char * new_child )
{
    dir_entry * new_enry;
    int alloc_size;

    if( parent->sub_entries == NULL )
    {
        parent->capacity = 20;
        parent->size = 0;
        parent->sub_entries = ( dir_entry * )malloc( 20 * sizeof( dir_entry ) );
        if( parent->sub_entries == NULL )
        {
            fprintf(stderr, "ERROR in memory allocation.\n");
        }
    }

    if( parent->capacity > parent->size )
    {
        new_enry = &((parent->sub_entries)[parent->size]);
        strcpy( new_enry->fname, new_child );
        parent->size = parent->size + 1;
        new_enry->sub_entries = NULL;
        new_enry->size = 0;
        //printf( "ADD ENTRY----2 parent size: %d\n", parent->size );
    }
    else
    {
        alloc_size = (parent->size + 20) * sizeof( dir_entry );
        parent->sub_entries = ( dir_entry * )realloc( parent->sub_entries,  alloc_size );
        if( parent->sub_entries == NULL )
        {
            fprintf(stderr, "ERROR in memory allocation.\n");
        }
        new_enry = &((parent->sub_entries)[parent->size]);
        strcpy( new_enry->fname, new_child );
        parent->size = parent->size + 1;
        new_enry->sub_entries = NULL;
        new_enry->size = 0;
    }
}

int
get_index( dir_entry * parent, char * child_name )
{
    if( parent->sub_entries == NULL ) return -1;
    for( int i = 0; i < parent->size; i++ )
    {
        if( strcmp( ((parent->sub_entries)[i]).fname, child_name ) == 0 )
        {
            return i;
        }
    }
    return -1;
}

void
print_fproperty( file_property property )
{
    printf( "file name: %s\n", property.fname );
    printf( "file size: %d\n", property.fsize );
    printf( "file type: %c\n", property.ftype );
    printf( "file permission: %s\n", property.fperm );
    printf( "file num_of_link: %d\n\n", property.num_of_link );
}

int
is_target_file( file_property property )
{
    if( fname_setted )
    {
        //printf( "NAME\n" );
        if( is_match( target_fname , property.fname ) );
        else return 0;
    }
    if( fsize_setted )
    {
        if( target_fsize == property.fsize );
        else return 0;
    }
    if( ftype_setted )
    {
        //printf( "TYPE\n" );
        if( target_ftype == property.ftype );
        else return 0;
    }
    if( fperm_setted )
    {
        //printf( "PERM\n" );
        if( strcmp( target_fperm, property.fperm ) == 0 );
        else return 0;
    }
    if( num_of_link_setted )
    {
        //printf( "LINK\n" );
        if( target_num_of_link == property.num_of_link );
        else return 0;
    }
    return 1;
}

//a boolean function
int
is_match( char syntax[], char exp[] )
{   
    char lower_syntax[256];
    char lower_exp[256];

    make_lover_case( syntax, lower_syntax );
    make_lover_case( exp, lower_exp );

    int s_index = 0; //syntax index
    int s_size = strlen( lower_syntax );

    int e_index = 0; //expression index
    int e_size = strlen( lower_exp );

    while( s_index < s_size )
    {
        if( lower_syntax[ s_index ] == lower_exp[ e_index ] )
        {
            s_index++;
            e_index++;
        }
        else if( lower_syntax[s_index] == '+' )
        {
            if( lower_syntax[ s_index -1 ] == lower_exp[ e_index ] )
            {
                e_index++;
            }
            else
            {
                s_index++;
            }
        }
        else return 0;
    }
    if( e_index == e_size ) return 1;
    else return 0;
}

char
convert_meaningful_char( unsigned char d_type )
{
    if( d_type == DT_UNKNOWN ) return '?';
    else if( d_type == DT_REG ) return 'f';
    else if( d_type == DT_DIR ) return 'd';
    else if( d_type == DT_FIFO ) return 'p';
    else if( d_type == DT_SOCK ) return 's';
    else if( d_type == DT_CHR ) return 'c';
    else if( d_type == DT_BLK ) return 'b';
    else if( d_type == DT_LNK ) return 'l';
    return '?';
}

void
find_property( struct stat file_stat, char permissions[], int * num_of_link, int * size )
{
    permissions[0] = (file_stat.st_mode & S_IRUSR) ? 'r' : '-';
    permissions[1] = (file_stat.st_mode & S_IWUSR) ? 'w' : '-';
    permissions[2] = (file_stat.st_mode & S_IXUSR) ? 'x' : '-';
    permissions[3] = (file_stat.st_mode & S_IRGRP) ? 'r' : '-';
    permissions[4] = (file_stat.st_mode & S_IWGRP) ? 'w' : '-';
    permissions[5] = (file_stat.st_mode & S_IXGRP) ? 'x' : '-';
    permissions[6] = (file_stat.st_mode & S_IROTH) ? 'r' : '-';
    permissions[7] = (file_stat.st_mode & S_IWOTH) ? 'w' : '-';
    permissions[8] = (file_stat.st_mode & S_IXOTH) ? 'x' : '-';    
    permissions[9] = '\0';

    *num_of_link = file_stat.st_nlink;
    *size = file_stat.st_size;
}

int walk_in_dir(const char *path) 
{
    struct dirent *entry;
    DIR *dp;
    char fpath[4096];
    struct stat f_stat;
    int return_of_stat = 0;
    char permissions[10];
    int num_of_link;
    int size;
    file_property property;
    char path_as_list[100][256];
    int path_size = 0;
    
    dp = opendir(path);
    if (dp == NULL) 
    {
        //perror("opendir");
        return -1;
    }

    while((entry = readdir(dp))){
        strcpy( property.fname, entry->d_name );

        if( (property.fname)[0] != '.' )
        {
            property.ftype = convert_meaningful_char( entry -> d_type );

            strcpy( fpath, "" );
            strcat( fpath, path );
            if(  path[strlen( path ) -1] != '/' )
            {
               strcat( fpath, "/" ); 
            }
            strcat( fpath, property.fname );
            return_of_stat = stat( fpath, &f_stat);
            if( return_of_stat == -1 )
            {
                fprintf(stderr, "stat syscall failed in walk_in_dir() fınction.\n");
            }
            find_property( f_stat, permissions, &num_of_link, &size );
            strcpy( property.fperm, permissions );
            property.num_of_link = num_of_link;
            property.fsize = size;

            if( is_target_file( property ) )
            {
                file_found = 1;
                corvvert_to_list( fpath, path_as_list, &path_size );
                add_to_tree( &root, path_as_list, path_size );
            }

            if( property.ftype == 'd' )
            {
                walk_in_dir( fpath );
            }
        }


    }
    closedir(dp);
    return 0;
}