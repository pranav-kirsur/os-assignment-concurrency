#define _POSIX_C_SOURCE 199309L //required for clock
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <limits.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <inttypes.h>
#include <math.h>

int *shareMem(size_t size)
{
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int *)shmat(shm_id, NULL, 0);
}

//swap function
void swap(int *a, int *b)
{
    int temp = *a;
    *a = *b;
    *b = temp;
    return;
}

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

//normal quicksort
void quicksort(int arr[], int l, int r)
{
    if (r < l)
    {
        return;
    }

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

    //recurse for quicksort
    int pivot_index = partition(arr, l, r);
    quicksort(arr, l, pivot_index - 1);
    quicksort(arr, pivot_index + 1, r);
    return;
}

//concurrent quicksort
void concurrent_quicksort(int arr[], int l, int r)
{
    if (r < l)
    {
        return;
    }

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

    return;
}

//struct for passing arguments to threaded quicksort
struct arg
{
    int l;
    int r;
    int *arr;
};

//threaded quicksort
void *threaded_quicksort(void *a)
{
    //typecast void pointer
    struct arg *args = (struct arg *)a;

    //get arguments
    int l = args->l;
    int r = args->r;
    int *arr = args->arr;

    if (r < l)
    {
        return NULL;
    }

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

    return NULL;
}

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

int main()
{
    int n;
    scanf("%d", &n);
    runsorts(n);
    return 0;
}