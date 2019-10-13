# Ober Cab Services

# Overview
This is a simulation of the situation described in the question.\
We use threading, mutex locks, and semaphores.

# Modelling
We model the cabs as a struct and rider and payment server as threads.

# Mutex Locks
We use two mutex locks which are useful for critical sections:

```C

    //mutex lock for the threads
    pthread_mutex_t lock;

    //mutex lock for the payment server
    pthread_mutex_t payment_lock;

```

# Semaphores
We use one semaphore that is used to check if riders are waiting for a payment server

```C
    //semaphore for people waiting for payment server
    sem_t semaphore_waiting_for_payment_server;
```

# Helper functions
`getrandom` returns a random number in the given range.

# Compilation
Run this command:\
`gcc -pthread Q3.c`

# Timing
For timing purposes we use `clock_gettime` function.

# Cabs Array
We store the cabs as a global array of structs whose state attribute is constantly modified throughout the program.

# Structs
We use structs for the Cab and the Rider

# Explanation of code

We give a brief explanation of important parts of the code:

We first create the rider threads:
```C
    //create rider threads
    for (int i = 0; i < m; i++)
    {
        args[i].id = i;
        pthread_create(&rider_threads[i], NULL, rider, &args[i]);
    }
```

The riders then print the required info and then call `book_cab`

```C
    //book the cab
    int cab_assigned = book_cab(cab_type, max_wait_time, ride_time, id);
```

This function tries to assign the rider to a cab until time exceeds `max_wait_time`, at which point it returns -1 and prints a timeout meassage.

Whenever cab assignment is done here, we use the mutex lock `lock`.

The cab states in the cab array struct are modified accordingly.

The payment server threads are created as follows:
```C
    //create payment server threads
    for (int i = 0; i < k; i++)
    {
        payment_server_args[i].id = i;
        pthread_create(&payment_server_threads[i], NULL, payment_server, &payment_server_args[i]);
    }
```

The payment server thread waits on the semaphore `semaphore_waiting_for_payment_server`

```C
        //wait for semaphore
        sem_wait(&semaphore_waiting_for_payment_server);
```

We then mutex lock `payment_lock` and look for riders that are waiting for a payment server.

```C
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
```

We finally release the lock.

This function is in a `while` loop that continues while there are some riders in the simulation.

Finally to avoid deadlocks, after all riders are done, we post on the semaphore `semaphore_waiting_for_payment_server` k times to ensure all payment server threads terminate.

```C
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
```

We then join the threads and end the simulation.
```C
    //wait for threads to complete
    for (int i = 0; i < m; i++)
    {
        pthread_join(rider_threads[i], NULL);
    }




    for (int i = 0; i < k; i++)
    {
        pthread_join(payment_server_threads[i], NULL);
    }

    //destroy semaphore
    sem_destroy(&semaphore_waiting_for_payment_server);

    printf("Simulation ended\n");
```










# Concurrency
The usage of mutexes to protect critical sections and semaphores as a signalling mechanism ensures that threads run concurrently and that data operations that are required to be atomic are indeed so.



