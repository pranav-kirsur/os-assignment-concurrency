#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//states
#define STATE_WAIT 0
#define STATE_ON_RIDE_PREMIER 1
#define STATE_ON_RIDE_POOL_FULL 2
#define STATE_ON_RIDE_POOL_ONE 3

struct Cab
{
    int id;
    int state;
};

struct Rider
{
    int id;
};

struct Cab *cabs;

void *rider(void *args)
{
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
    return 0;
}