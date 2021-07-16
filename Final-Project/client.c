#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#define PORT 20000

int id;
char address[100];
int port;
char path[4000];

void
get_queries( char path[], char queries[][1000], int * size, int id );

void
take_arguments( int argc, char **argv );

void
get_response( int fd );

//./client -i 10 -a 127.0.0.1 -p 500 -o pathToQueryFi
int main(int argc, char **argv)
{
	take_arguments( argc, argv );
	printf("id: %d\n", id );
	printf("address: %s\n", address );
	printf("port: %d\n", port );
	printf("path: %s\n", path );


	char queries[1000][1000];
	int queries_size;

	get_queries( "queries.txt", queries, &queries_size,  id );
	//printf("QUERY TAKEN\n");
	

	int sock = 0, valread;
	struct sockaddr_in serv_addr;
	char *hello = "Hello from client";
	
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("\n Socket creation error \n");
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	
	// Convert IPv4 and IPv6 addresses from text to binary form
	if(inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0)
	{
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	printf("Client-%d connecting to %s:%d\n", id, address, port );
	
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("\nConnection Failed \n");
		return -1;
	}

	

	for( int i = 0; i < queries_size; i++ )
	{
		printf( "Client-%d connected and sending query \'%s\'\n", id, queries[i] );
		//printf("%s\n", queries[i] );
	    write(sock , queries[i] , strlen( queries[i] ) +1 );
	    //printf("QUERY SENDED\n");
	    get_response( sock );
	}
	printf("A total of %d queries were executed, client is terminating.\n", queries_size );


	return 0;
}

void
get_response( int fd )
{
	printf("Server’s response to Client-%d is:\n", id );
	int size;
	char response[1024];
	while( 1 )
	{
		read( fd , &size, sizeof(int));
		//printf("size: %d\n", size );
		if( size == 0 )
		{
			break;
		}
		read( fd , response, size );
		printf( "%s\n", response );
	}
}


void
get_queries( char path[], char queries[][1000], int * size, int id )
{
	int id_in_line;

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
		sscanf( line, "%d", &id_in_line );
		if( id_in_line == id )
		{
			if( line[ strlen( line ) -1 ] == '\n' )
			{
				line[ strlen( line ) -1 ] = '\0';
			}
			strcpy( queries[i], line );
	        i++;			
		} 	
    }
    *size = i;	
    fclose(file);/**/
}



void
take_arguments( int argc, char **argv )
{
	//for( int i = 0; i < argc; i++ ) printf("%s\n", argv[i]);

	if( argc != 9 )
	{
		//show_usage();
		printf("Missing arg\n");
		exit(0);
	}



    int number_of_setted_value = 0;
    int c;
    while ((c = getopt (argc, argv, ":i:a:p:o:")) != -1)
    switch (c)
    {
        case 'i':
        	id = atoi( optarg );
            number_of_setted_value++;
            break;
        case 'a':
        	strcpy( address, optarg );
            number_of_setted_value++;
            break;
        case 'p':
        	port = atoi( optarg );
            number_of_setted_value++;
            break;
        case 'o':
        	strcpy( path, optarg );
            number_of_setted_value++;
            break;
        default:
            printf("ERROR: Undefined parameters dedected in command line arguments\n");
            //show_usage();
            exit( 0 );
    }

    if( number_of_setted_value != 4 )
    {
    	//show_usage();
    	printf("show usage\n");
    	exit( 0 );
    }
}



