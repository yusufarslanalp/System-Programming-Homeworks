typedef struct
{
    char university[100];
    int quality;
    int speed;
    int monney;
    int available;
    int completed_hw;
    char current_hw;
}student;

student std_table[1000];
int std_table_size;
int remaining_money;

char queue[10000];
int head;
int tail;

sem_t sem_queue;
sem_t sem_has_hw;
sem_t sfh_sems[1000]; //semaphores for student_for_hire
sem_t hw_taken;
sem_t sem_available_std;

int wait_for_assignment;
int no_monney;
char hw_type;

char * hw_fpath;
char * student_fpath;

void
add( char hw );

char
poll();

static void *
student_h(void *arg);

static void *
student_for_hire(void *arg);

void
read_hws( char * fname );

void
get_students( char path[] );

int
find_suitable_std( char priority );

void
my_exit();

void
handle_sigint(int sig);