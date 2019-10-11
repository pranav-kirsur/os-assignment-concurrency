#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>

//states
#define STATE_WAIT 0
#define STATE_ON_RIDE_PREMIER 1
#define STATE_ON_RIDE_POOL_FULL 2
#define STATE_ON_RIDE_POOL_ONE 3

//Cab types
#define TYPE_PREMIER 0
#define TYPE_POOL 1

//Max waiting time for rider
#define MAX_WAIT_TIME 5

//Number of cabs, number of riders, number of payment servers
int n, m, k;

struct Cab
{
    int id;
    int state;
};

struct Rider
{
    int id;
};

//mutex lock for the threads
pthread_mutex_t lock;

//global variable that contains array of cab structs
struct Cab *cabs_array;

//returns random number in the given range(both inclusive)
int getrandom(int lower_bound, int upper_bound)
{
    return (rand() % (upper_bound - lower_bound + 1)) + lower_bound;
}

void book_cab(int cab_type, int maxwaitime, int ride_time, int id)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    time_t inittime = ts.tv_sec;
    //while time is less than maxwaittime
    while (clock_gettime(CLOCK_REALTIME, &ts), ts.tv_sec - inittime < maxwaitime)
    {
        //flag to check whether a cab has been assigned
        int is_cab_assigned = 0;
        //lock and then look for cabs
        pthread_mutex_lock(&lock);
        if (cab_type == TYPE_PREMIER)
        {
            for (int i = 0; i < n; i++)
            {
                if (cabs_array[i].state == STATE_WAIT)
                {
                    cabs_array[i].state = STATE_ON_RIDE_PREMIER;
                    is_cab_assigned = 1;
                    printf("Rider %d has been assigned cab number %d \n", id, cabs_array[i].id);
                    fflush(stdout);
                    break;
                }
            }
        }
        else
        {
            for (int i = 0; i < n; i++)
            {
                if (cabs_array[i].state == STATE_WAIT)
                {
                    cabs_array[i].state = STATE_ON_RIDE_POOL_ONE;
                    is_cab_assigned = 1;
                    printf("Rider %d has been assigned cab number %d \n", id, cabs_array[i].id);
                    break;
                }
                else if (cabs_array[i].state == STATE_ON_RIDE_POOL_ONE)
                {
                    cabs_array[i].state = STATE_ON_RIDE_POOL_FULL;
                    is_cab_assigned = 1;
                    printf("Rider %d has been assigned cab number %d \n", id, cabs_array[i].id);
                    break;
                }
            }
        }
        //unlock mutex
        pthread_mutex_unlock(&lock);
        if(is_cab_assigned)
        {
            return;
        }
    }
    //print timeout message
    printf("Rider %d timed out\n",id);
    fflush(stdout);
    return;
}

void *rider(void *args)
{
    //get arguments for rider
    struct Rider *rider_args = (struct Rider *)args;
    int id = rider_args->id;

    //generate random arguments for booking cab
    int cab_type = getrandom(0, 1);
    int ride_time = getrandom(1, 5);

    //print request
    printf("Rider %d has requested a cab of type %s with max waiting time as %d and ride time as %d\n", id, cab_type == TYPE_PREMIER ? "premier" : "pool", MAX_WAIT_TIME, ride_time);
    fflush(stdout);

    //book the cab
    book_cab(cab_type, MAX_WAIT_TIME, ride_time, id);

    //end the thread
    return NULL;
}

int main()
{
    printf("Enter the number of cabs, the  number of riders, and the number of payment servers\n");
    fflush(stdout);
    scanf("%d%d%d", &n, &m, &k);

    //create array of cab structs
    cabs_array = (struct Cab *)malloc(n * sizeof(struct Cab));

    //initialise cabs
    for (int i = 0; i < n; i++)
    {
        cabs_array[i].id = i;
        cabs_array[i].state = STATE_WAIT;
    }

    //seed random
    srand(time(NULL));

    //create mutex lock
    pthread_mutex_init(&lock, NULL);

    //Store rider threads in an array
    pthread_t rider_threads[m];
    struct Rider args[m];

    //create rider threads
    for (int i = 0; i < m; i++)
    {
        args[i].id = i;
        pthread_create(&rider_threads[i], NULL, rider, &args[i]);
    }

    //wait for threads to complete
    for (int i = 0; i < m; i++)
    {
        pthread_join(rider_threads[i], NULL);
    }
    return 0;
}