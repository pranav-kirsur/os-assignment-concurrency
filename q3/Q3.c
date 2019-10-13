#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

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

//mutex lock for the payment server
pthread_mutex_t payment_lock;

//number of riders currently in simulation
int num_in_sim;

//global variable that contains array of cab structs
struct Cab *cabs_array;

//global variable that contains the array that contains for whether or not rider is waiting for payment server
int *is_waiting_for_payment_server;

//semaphore for people waiting for payment server
sem_t semaphore_waiting_for_payment_server;

//returns random number in the given range(both inclusive)
int getrandom(int lower_bound, int upper_bound)
{
    return (rand() % (upper_bound - lower_bound + 1)) + lower_bound;
}

int book_cab(int cab_type, int maxwaitime, int ride_time, int id)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    time_t inittime = ts.tv_sec;
    //while time is less than maxwaittime
    while (clock_gettime(CLOCK_REALTIME, &ts), ts.tv_sec - inittime <= maxwaitime)
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

                    //unlock the mutex
                    pthread_mutex_unlock(&lock);

                    //Wait till ride time
                    sleep(ride_time);

                    //print that ride has ended
                    printf("Rider %d has finished the ride and is waiting for a payment server\n", id);

                    //modify status of cab the rider was in inside of a mutex lock
                    pthread_mutex_lock(&lock);
                    cabs_array[i].state = STATE_WAIT;
                    pthread_mutex_unlock(&lock);

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

                    //unlock the mutex
                    pthread_mutex_unlock(&lock);

                    //Wait till ride time
                    sleep(ride_time);

                    //print that ride has ended
                    printf("Rider %d has finished the ride and is waiting for a payment server\n", id);

                    //modify status of cab the rider was in inside of a mutex lock
                    pthread_mutex_lock(&lock);
                    if (cabs_array[i].state == STATE_ON_RIDE_POOL_FULL)
                    {
                        cabs_array[i].state = STATE_ON_RIDE_POOL_ONE;
                    }
                    else if (cabs_array[i].state == STATE_ON_RIDE_POOL_ONE)
                    {
                        cabs_array[i].state = STATE_WAIT;
                    }
                    pthread_mutex_unlock(&lock);

                    break;
                }
                else if (cabs_array[i].state == STATE_ON_RIDE_POOL_ONE)
                {
                    cabs_array[i].state = STATE_ON_RIDE_POOL_FULL;
                    is_cab_assigned = 1;
                    printf("Rider %d has been assigned cab number %d \n", id, cabs_array[i].id);

                    //unlock the mutex
                    pthread_mutex_unlock(&lock);

                    //Wait till ride time
                    sleep(ride_time);

                    //print that ride has ended
                    printf("Rider %d has finished the ride and is waiting for a payment server\n", id);

                    //modify status of cab the rider was in inside of a mutex lock
                    pthread_mutex_lock(&lock);
                    if (cabs_array[i].state == STATE_ON_RIDE_POOL_FULL)
                    {
                        cabs_array[i].state = STATE_ON_RIDE_POOL_ONE;
                    }
                    else if (cabs_array[i].state == STATE_ON_RIDE_POOL_ONE)
                    {
                        cabs_array[i].state = STATE_WAIT;
                    }
                    pthread_mutex_unlock(&lock);

                    break;
                }
            }
        }
        if (is_cab_assigned)
        {
            return 1;
        }
        else
        {
            //release the mutex lock
            pthread_mutex_unlock(&lock);
        }
    }
    //print timeout message
    printf("Rider %d timed out\n", id);
    num_in_sim--;
    fflush(stdout);
    return -1;
}

void *rider(void *args)
{
    //get arguments for rider
    struct Rider *rider_args = (struct Rider *)args;
    int id = rider_args->id;

    //generate random arguments for booking cab
    int cab_type = getrandom(0, 1);
    int ride_time = getrandom(1, 5);
    int max_wait_time = getrandom(1,5);

    //sleep for some random amount of time
    sleep(getrandom(1,5));

    //print request
    printf("Rider %d has requested a cab of type %s with max waiting time as %d and ride time as %d\n", id, cab_type == TYPE_PREMIER ? "premier" : "pool", max_wait_time, ride_time);
    fflush(stdout);

    //book the cab
    int cab_assigned = book_cab(cab_type, max_wait_time, ride_time, id);

    //if rider didn't timeout then set waiting for payment server and increment the semaphore
    if (cab_assigned != -1)
    {
        is_waiting_for_payment_server[id] = 1;
        sem_post(&semaphore_waiting_for_payment_server);
    }

    //end the thread
    return NULL;
}

struct PaymentServer
{
    int id;
};

void *payment_server(void *payment_args)
{
    //get id from the arguments
    int id = ((struct PaymentServer *)payment_args)->id;

    while (num_in_sim > 0)
    {

        //wait for semaphore
        sem_wait(&semaphore_waiting_for_payment_server);

        //lock mutex and look for rider
        pthread_mutex_lock(&payment_lock);
        int rider = 0;
        for (rider = 0; rider < m; rider++)
        {
            if (is_waiting_for_payment_server[rider])
            {
                break;
            }
        }
        if (rider < m)
        {
            is_waiting_for_payment_server[rider] = 0;
        }

        //release lock
        pthread_mutex_unlock(&payment_lock);

        //in case no rider is waiting
        if (rider == m)
        {
            continue;
        }

        printf("Rider %d has been assigned payment server %d\n", rider, id);

        //sleep for 2 s
        sleep(2);

        printf("Rider %d has finished payment\n", rider);
        num_in_sim--;
    }
    return NULL;
}

int main()
{
    printf("Enter the number of cabs, the  number of riders, and the number of payment servers\n");
    fflush(stdout);
    scanf("%d%d%d", &n, &m, &k);

    //create array of cab structs
    cabs_array = (struct Cab *)malloc(n * sizeof(struct Cab));

    //initialise number of riders in simulation
    num_in_sim = m;

    //create array of flags for payment server
    is_waiting_for_payment_server = (int *)malloc(m * sizeof(int));

    //create semaphore for payment servers
    sem_init(&semaphore_waiting_for_payment_server, 0, 0);

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
    pthread_mutex_init(&payment_lock, NULL);

    //Store rider threads in an array
    pthread_t rider_threads[m];
    struct Rider args[m];

    printf("Simulation started\n");

    //create rider threads
    for (int i = 0; i < m; i++)
    {
        args[i].id = i;
        pthread_create(&rider_threads[i], NULL, rider, &args[i]);
    }

    //store payment server threads in an array
    pthread_t payment_server_threads[k];

    //payment server thread args
    struct PaymentServer payment_server_args[k];

    //create payment server threads
    for (int i = 0; i < k; i++)
    {
        payment_server_args[i].id = i;
        pthread_create(&payment_server_threads[i], NULL, payment_server, &payment_server_args[i]);
    }

    //wait for threads to complete
    for (int i = 0; i < m; i++)
    {
        pthread_join(rider_threads[i], NULL);
    }

    //in case threads are waiting we shall increment semaphore k times to allow threads to escape out of semaphore out of wait
    while (1)
    {
        if (num_in_sim == 0)
        {
            for (int i = 0; i < k; i++)
            {
                sem_post(&semaphore_waiting_for_payment_server);
            }
            break;
        }
    }

    for (int i = 0; i < k; i++)
    {
        pthread_join(payment_server_threads[i], NULL);
    }

    //destroy semaphore
    sem_destroy(&semaphore_waiting_for_payment_server);

    printf("Simulation ended\n");

    return 0;
}