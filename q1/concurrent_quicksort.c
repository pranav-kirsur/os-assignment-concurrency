#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>



int * shareMem(size_t size){
    key_t mem_key = IPC_PRIVATE;
    int shm_id = shmget(mem_key, size, IPC_CREAT | 0666);
    return (int*)shmat(shm_id, NULL, 0);
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
        quicksort(arr, l, pivot_index - 1);
        _exit(1);
    }
    else
    {
        int pid2 = fork();
        if (pid2 == 0)
        {
            quicksort(arr, pivot_index + 1, r);
            _exit(1);
        }
        else
        {
            int status;
            //wait for both processes to finish
            waitpid(pid1,&status,0);
            waitpid(pid2,&status,0);
        }
        
    }

    return;
}

int main()
{
    int n;
    scanf("%d", &n);
    int *arr = shareMem(sizeof(int)*(n+1));
    for (int i = 0; i < n; i++)
    {
        scanf("%d", &arr[i]);
    }
    concurrent_quicksort(arr, 0, n - 1);
    for (int i = 0; i < n; i++)
    {
        printf("%d ", arr[i]);
    }

    //release shared memory
    shmdt(arr);
    return 0;
}