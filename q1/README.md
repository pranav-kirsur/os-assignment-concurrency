# Report - Concurrent Quicksort

# Compilation
Run the following:
```bash
gcc -pthread -D_GNU_SOURCE Q1.c
```

# Implementations

## Shared memory

To ensure sharing of memory we use `shmget` and `shmat`
```C
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int *)shmat(shm_id, NULL, 0);
``` 

## Partition function
This function gets array as an input and partitions it with a random pivot, returning the index of the pivot
```C
    //partition function for quicksort
    int partition(int arr[], int l, int r)
    {
        int random_index = l + (rand() % (r - l + 1));
        swap(&arr[random_index], &arr[l]);

        //now pivot is the first element
        int piv = arr[l];
        int i = l + 1;

        for (int j = l + 1; j <= r; j++)
        {
            if (arr[j] < piv)
            {
                swap(&arr[i], &arr[j]);
                i++;
            }
        }
        //move pivot to middle position
        swap(&arr[i - 1], &arr[l]);

        //return pivot position
        return i - 1;
    }
```

## Multiprocess quicksort

Under the function `concurrent_quicksort`\
If array size is 5 or less we do an insertion sort.
```C
    if (r - l <= 4)
    {
        //do insertion sort
        int key;
        for (int i = l + 1; i <= r; i++)
        {
            key = arr[i];
            int j = i - 1;
            while (j >= l && arr[j] > key)
            {
                arr[j + 1] = arr[j];
                j--;
            }
            arr[j + 1] = key;
        }
        return;
    }
```

Else we first part partition array into two parts using `partition` function(by picking a random pivot) and fork twice into two processes that each recursively quicksort the two halves in a multiprocess manner.\
The parent process meanwhile calls `wait` on the two processes to ensure it continues only after both of those processes are finished.

```C
    //recurse for quicksort
    int pivot_index = partition(arr, l, r);
    int pid1 = fork();
    if (pid1 == 0)
    {
        //sort left part
        concurrent_quicksort(arr, l, pivot_index - 1);
        _exit(1);
    }
    else
    {
        int pid2 = fork();
        if (pid2 == 0)
        {
            concurrent_quicksort(arr, pivot_index + 1, r);
            _exit(1);
        }
        else
        {
            int status;
            //wait for both processes to finish
            waitpid(pid1, &status, 0);
            waitpid(pid2, &status, 0);
        }
    }
```


## Multithreaded quicksort

Under the function `threaded_quicksort`.\
We pass arguments as a struct
```C
    //struct for passing arguments to threaded quicksort
    struct arg
    {
        int l;
        int r;
        int *arr;
    };
```
If array size is 5 or less we do an insertion sort.
```C
    if (r - l <= 4)
    {
        //do insertion sort
        int key;
        for (int i = l + 1; i <= r; i++)
        {
            key = arr[i];
            int j = i - 1;
            while (j >= l && arr[j] > key)
            {
                arr[j + 1] = arr[j];
                j--;
            }
            arr[j + 1] = key;
        }
        return NULL;
    }
```

Else we first part partition array into two parts using `partition` function(by picking a random pivot) and create two threads that each sort one half.\
The threads themselves run the same function, so recursively array is sorted.\
The parent process joins the two threads.\
Thus, the array is sorted.

```C
    //recurse for quicksort
    int pivot_index = partition(arr, l, r);

    //sort left side in one thread
    pthread_t tid1;

    //args for the thread
    struct arg args_left;
    args_left.l = l;
    args_left.r = pivot_index - 1;
    args_left.arr = arr;

    //create the thread
    pthread_create(&tid1, NULL, threaded_quicksort, &args_left);

    //sort right side in another thread
    pthread_t tid2;

    //args for the thread
    struct arg args_right;
    args_right.l = pivot_index + 1;
    args_right.r = r;
    args_right.arr = arr;

    //create thread
    pthread_create(&tid2, NULL, threaded_quicksort, &args_right);

    //wait for the two threads to complete
    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);
```
## Running the sorts
We run the sorts using `runsorts`, taking array as input.\
For timing we use `clock_gettime`

```C
void runsorts(int n)
{
    struct timespec ts;

    //assign shared memory
    int *arr = shareMem(sizeof(int) * (n + 1));

    //take input
    for (int i = 0; i < n; i++)
    {
        scanf("%d", &arr[i]);
    }

    //make two copied arrays for comparision
    int brr[n + 1];
    int crr[n + 1];
    for (int i = 0; i < n; i++)
    {
        brr[i] = arr[i];
        crr[i] = arr[i];
    }

    printf("Running concurrent_quicksort for n = %d\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double st = ts.tv_nsec / (1e9) + ts.tv_sec;

    //multiprocess quicksort
    concurrent_quicksort(arr, 0, n - 1);

    //print time
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    long double en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t1 = en - st;

    //multithreaded quicksort args
    pthread_t tid;
    struct arg a;
    a.l = 0;
    a.r = n - 1;
    a.arr = crr;

    printf("Running multithreaded concurrent quicksort for n = %d\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec / (1e9) + ts.tv_sec;

    //multithreaded quicksort
    pthread_create(&tid, NULL, threaded_quicksort, &a);
    pthread_join(tid, NULL);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t2 = en - st;

    printf("Running normal quicksort for n = %d\n", n);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    st = ts.tv_nsec / (1e9) + ts.tv_sec;

    // normal quicksort
    quicksort(brr, 0, n - 1);

    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    en = ts.tv_nsec / (1e9) + ts.tv_sec;
    printf("time = %Lf\n", en - st);
    long double t3 = en - st;

    printf("Normal quicksort ran %Lf times faster than multiprocess concurrent quicksort and %Lf times faster than multithreaded concurrent quicksort\n", t1 / t3, t1 / t2);

    //release shared memory
    shmdt(arr);
}
```

## Performance output

    Running concurrent_quicksort for n = 10
    time = 0.001671
    Running multithreaded concurrent quicksort for n = 10
    time = 0.000960
    Running normal quicksort for n = 10
    time = 0.000005
    Normal quicksort ran 330.043772 times faster than multiprocess concurrent quicksort and 1.740465 times faster than multithreaded concurrent quicksort

    Running concurrent_quicksort for n = 10000
    time = 0.425225
    Running multithreaded concurrent quicksort for n = 10000
    time = 0.129034
    Running normal quicksort for n = 10000
    time = 0.001039
    Normal quicksort ran 409.127068 times faster than multiprocess concurrent quicksort and 3.295462 times faster than multithreaded concurrent quicksort

We note that the normal quicksort is the fastest, likely because of overheads for threads and processes.





