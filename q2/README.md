# Automatic Biryani Serving

# Overview
We perform a simulation of the situation decribed in the question using threads.\
We use mutex locks to protect critical sections.\
The output is printed to the console.

# Helper functions
`getrandom` returns a random number in the given range.

# Mutex locks
We use two mutex locks: `lock` and `slot_lock`. \
Whenever we are doing a critical operation we first lock a mutex and then unlock it later.

# Structs
We have three structs :\
 `Chef` , `ServingTable` , `Student`

# Global arrays
We have three arrays of structs for each of chef , table, student

# Global variables
n, m, k which were from the question.\
We also have `num_students_left` that maintains a count of the students left.

# Threads
Each chef, student, table is a thread which calls its own functions as defined in the question to execute its job.

# Functions
Functions mostly perform the functions as defined in the question.

# Compilation
Run the following:\
`gcc -pthread Q2.c`

# Description of code
We create threads for each of chefs, serving tables, and students:
```C
    //create chef threads
    pthread_t chefthreads[m];

    for (int i = 0; i < m; i++)
    {
        chefs[i].id = i;
        chefs[i].vessels = 0;
        chefs[i].capacity = 0;
        pthread_create(&chefthreads[i], NULL, chef, &chefs[i]);
    }

    //create serving table threads
    pthread_t serving_table_threads[n];

    for (int i = 0; i < n; i++)
    {
        tables[i].id = i;
        tables[i].is_serving_container_occupied = 0;
        tables[i].capacity = 0;
        tables[i].numslots = 0;
        pthread_create(&serving_table_threads[i], NULL, servingtable, &tables[i]);
    }

    //create student threads
    pthread_t student_threads[k];

    for (int i = 0; i < k; i++)
    {
        students[i].id = i;
        pthread_create(&student_threads[i], NULL, student, &students[i]);
    }
```

The chef threads run the function `chef` which prepares biryani and then calls `biryani_ready`.

In `biryani_ready`, to ensure concurrency, we use the mutex lock `lock` .

```C
        pthread_mutex_lock(&lock);
        for (int i = 0; i < n; i++)
        {
            if (tables[i].is_serving_container_occupied == 0)
            {
                if (chefs[id].vessels > 0)
                {
                    chefs[id].vessels--;
                    tables[i].is_serving_container_occupied = 1;
                    tables[i].capacity = chefs[id].capacity;
                    printf("Chef %d has served biryani on table %d\n", id, i);
                }
            }
        }
        pthread_mutex_unlock(&lock);
```


The serving table threads run `servingtable` which performs functions as descibed in the question.

The student threads run `student` . They utilise the mutex lock `slot_lock` to ensure that only one of the students grabs a slot.

```C
        pthread_mutex_lock(&slot_lock);
        if (tables[i].numslots >= 1)
        {
            printf("Student %d has been assigned to table %d\n", id, i);
            tables[i].numslots--;
            tables[i].capacity--;
            if (tables[i].capacity == 0)
            {
                printf("Serving Container of table %d is empty, waiting for refill\n", i);
                tables[i].is_serving_container_occupied = 0;
            }
            num_students_left--;
            printf("Student %d on table %d has been served\n", id, i);
            pthread_mutex_unlock(&slot_lock);
            return;
        }
        pthread_mutex_unlock(&slot_lock);
```

Finally, in `main` , we join the threads

```C
    //join the threads
    for (int i = 0; i < m; i++)
    {
        pthread_join(chefthreads[i], NULL);
    }

    for (int i = 0; i < n; i++)
    {
        pthread_join(serving_table_threads[i], NULL);
    }

    for (int i = 0; i < k; i++)
    {
        pthread_join(student_threads[i], NULL);
    }
```
and end the simulation

```C
    printf("Simulation end\n");
```





