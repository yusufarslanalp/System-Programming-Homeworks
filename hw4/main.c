#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include "header.c"
#include<signal.h>

#define handle_error_en(en, msg) \
       do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
       do { perror(msg); exit(EXIT_FAILURE); } while (0)



int
main(int argc, char *argv[])
{
    if( argc != 4 )
    {
        printf("command line arguments are missing/invalid\n");
        printf("Sample usage: ./program homeworkFilePath studentsFilePath 10000\n");
        exit( 0 );
    }

    signal(SIGINT, handle_sigint);

    hw_fpath = argv[1];
    student_fpath = argv[2];
    remaining_money = atoi( argv[3] );

    //printf("hw_fpath: %s\n", hw_fpath );
    //printf("student_fpath: %s\n", student_fpath );
    //printf("remaining_money: %d\n", remaining_money );


    //sleep( 3 );
    head = 0;
    tail = 0;
    wait_for_assignment = 1;
    no_monney = 0;
    //printf("MAINNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNNN\n");



    get_students( student_fpath );

    //printf("mnoney2: %d\n", (std_table[2]).monney );


   int s;
   pthread_t t1;
   pthread_t sfh_threads[1000];
   void *res;

    if (sem_init(&sem_queue, 0, 1) == -1)
        perror( "sem_init" );
     //initially there is 0 hw in the queue
    if (sem_init(&sem_has_hw, 0, 0) == -1)
        perror( "sem_init" ); 
    if (sem_init(&sem_available_std, 0, std_table_size) == -1)
        perror( "sem_init" );
   

   int nums[1000];
   for( int i = 0; i < std_table_size; i++ )
   {
        if (sem_init(& (sfh_sems[i]) , 0, 0) == -1)
            perror( "sem_init" );    
        nums[i] = i;
        s = pthread_create( &(sfh_threads[i]) , NULL,
                          &student_for_hire,  &(nums[i]) );
        if (s != 0)
            handle_error_en(s, "pthread_create");
   }


   printf("%d students-for-hire threads have been created.\n", std_table_size );
   printf("Name Q S C\n");
   char * name;
   int Q, S, C;
   for( int i = 0; i < std_table_size; i++ )
   {
        name = (std_table[i]).university;
        Q = (std_table[i]).quality;
        S = (std_table[i]).speed;
        C = (std_table[i]).monney;
        printf( "%s %d %d %d\n", name, Q, S, C );
   }

   s = pthread_create( &t1 , NULL,
                      &student_h,  "I am child thread\n" );
   if (s != 0)
       handle_error_en(s, "pthread_create");


   //int val;
   //sem_getvalue( &sem_available_std, &val );
   //printf("sem_available_std: %d\n", val );

   char ch;
   int suit_index;
   int loop = 0;
   while( 1 )
   {
        ch = poll( );
        
        if( ch == 'D' )
        {
            //printf("All hws assigned\n");
            printf("No more homeworks left or coming in, closing.\n");
            break;
        }
        else
        {
            sem_wait( &sem_available_std );
            loop++;
            suit_index = find_suitable_std( ch );
            if( suit_index == -1 )
            {
                no_monney = 1;
                //printf("No monney left\n");
                printf("Money is over, closing.\n");
                break;
            }
            std_table[suit_index].available = 0;
            std_table[suit_index].current_hw = ch;
            remaining_money = remaining_money - std_table[suit_index].monney;
            sem_post( &(sfh_sems[suit_index]) );
            sem_wait( &hw_taken );
        }

   }

   wait_for_assignment = 0;
   for( int i = 0; i < std_table_size; i++ )
   {
        sem_post( &(sfh_sems[i]) );
   }

   for( int i = 0; i < std_table_size; i++ )
   {
       s = pthread_join( sfh_threads[i], &res);
       if (s != 0)
           handle_error_en(s, "pthread_join");
   }

   for( int i = 0; i < std_table_size; i++ )
   {
        sem_destroy( & (sfh_sems[ i ]));
   }

   s = pthread_join( t1, &res);
   if (s != 0)
       handle_error_en(s, "pthread_join");

   //printf("thred returned value is: %ld\n", (long) res);

   int comp_hw;
   int total_cost = 0;
   int total_comp_hw = 0;
   int monney;
   printf("Homeworks solved and money made by the students:\n");
   for( int i = 0; i < std_table_size; i++ )
   {
        name = (std_table[i]).university;
        monney = (std_table[i]).monney;
        comp_hw = (std_table[i]).completed_hw;
        printf( "%s %d %d\n", name, comp_hw, monney * comp_hw );
        total_comp_hw += comp_hw;
        total_cost += (monney * comp_hw);
   }
   printf( "Total cost for %d homeworks %d\n", total_comp_hw, total_cost );
   printf("Money left at H’s account: %dTL\n", remaining_money );



   //printf("destroy: %d\n", sem_destroy( &sem_queue ) );
   sem_destroy( &sem_has_hw );
   sem_destroy( &hw_taken );
   sem_destroy( &sem_available_std );   

   //printf("%d\n", remaining_money );

   exit(EXIT_SUCCESS);/**/
}

#include "functions.c"