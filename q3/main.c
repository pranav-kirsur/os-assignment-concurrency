#include <stdio.h>
#include <stdlib.h>
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

struct Cab
{
    int id;
    int state;
};

struct Rider
{
    int id;
};

//global variable that contains array of cab structs
struct Cab *cabs;

//returns random number in the given range(both inclusive)
int random(int lower_bound, int upper_bound)
{
    return (rand() % upper_bound - lower_bound + 1) + lower_bound;
}

void book_cab(int cab_type, int maxwaitime, int ride_time)
{

}

void *rider(void *args)
{
    //get arguments for rider
    struct Rider *rider_args = (struct Rider *)args;
    int id = rider_args->id;

    //generate random arguments for booking cab
    int cab_type = random(0, 1);
    int ride_time = random(1, 5);

    //book the cab
    book_cab(cab_type, MAX_WAIT_TIME, ride_time);

    //end the thread
    exit(1);
}

int main()
{
    int n, m, k;
    printf("Enter the number of cabs, the  number of riders, and the number of payment servers\n");
    scanf("%d%d%d", &n, &m, &k);

    //create array of cab structs
    cabs = (struct Cab *)malloc(n * sizeof(struct Cab));

    //initialise cabs
    for (int i = 0; i < n; i++)
    {
        cabs[i].id = i;
        cabs[i].state = STATE_WAIT;
    }

    //Store rider threads in an array
    pthread_t rider_threads[m];

    //create rider threads
    for (int i = 0; i < m; i++)
    {
        struct Rider args;
        args.id = i;
        pthread_create(&rider_threads[i], NULL, rider, &args)
    }

    //wait for threads to complete
    for (int i = 0; i < m; i++)
    {
        pthread_join(rider_threads[i], NULL);
    }
    return 0;
}