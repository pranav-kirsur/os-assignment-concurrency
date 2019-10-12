#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

//returns random number in the given range(both inclusive)
int getrandom(int lower_bound, int upper_bound)
{
    return (rand() % (upper_bound - lower_bound + 1)) + lower_bound;
}

//chefs,serving tables, student
int m, n, k;

int num_students_left;

//mutex lock
pthread_mutex_t lock;
pthread_mutex_t slot_lock;

//struct for chef thread arguments
struct Chef
{
    int id;
    int vessels;
    int capacity;
};

struct Chef *chefs;

struct ServingTable
{
    int id;
    int is_serving_container_occupied;
    int capacity;
    int numslots;
};

struct ServingTable *tables;

struct Student
{
    int id;
};

struct Student *students;

void biryani_ready(int id)
{
    while (chefs[id].vessels != 0 && num_students_left > 0)
    {
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
    }
    if (chefs[id].vessels == 0)
    {
        printf("All the vessels prepared by Chef %d are emptied. Resuming cooking now.\n", id);
    }
    return;
}

//function for chef thread
void *chef(void *chefargs)
{
    while (num_students_left > 0)
    {
        int id = ((struct Chef *)chefargs)->id;
        //sleep for random amount of time
        sleep(getrandom(2, 5));

        //prepare food
        chefs[id].vessels = getrandom(1, 10);
        chefs[id].capacity = getrandom(25, 50);

        printf("Chef %d has made %d biryani vessels of capacity %d\n", id, chefs[id].vessels, chefs[id].capacity);
        fflush(stdout);
        biryani_ready(id);
    }
    return NULL;
}

void ready_to_serve_table(int id, int number_of_slots)
{
    tables[id].numslots = number_of_slots;
    printf("Table %d is ready to serve and has been allocated %d slots\n", id, number_of_slots);
    while (num_students_left > 0 && tables[id].numslots > 0)
    {
        //wait
    }
    return;
}

//function for serving table thread
void *servingtable(void *tableargs)
{
    int id = ((struct ServingTable *)tableargs)->id;
    while (num_students_left > 0)
    {
        while (tables[id].is_serving_container_occupied == 0)
        {
            //do nothing
            if(num_students_left<=0)
            {
                return NULL;
            }
        }
        printf("Serving table %d entering serving phase\n", id);
        //serving mode
        int randomnum = getrandom(1, 10);
        ready_to_serve_table(id, randomnum);
    }

    return NULL;
}

//function that waits until a free slot is available
void wait_for_slot(int id)
{
    int i = 0;
    while (num_students_left > 0)
    {
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
        i = (i + 1) % n;
    }
}

void student_in_slot()
{
    return;
}

//function for students thread
void *student(void *studentargs)
{
    sleep(getrandom(1, 5));
    int id = ((struct Student *)studentargs)->id;
    printf("Student %d has arrived\n", id);
    printf("Student %d is waiting to be allocated a slot on the serving table\n", id);

    wait_for_slot(id);
    student_in_slot();
    return NULL;
}

int main()
{
    printf("Input the number of chefs, the number of serving tables, the number of students\n");
    scanf("%d%d%d", &m, &n, &k);

    //allocate memory for chefs,serving tables, students
    chefs = malloc(m * sizeof(struct Chef));
    tables = malloc(n * sizeof(struct ServingTable));
    students = malloc(k * sizeof(struct Student));

    printf("Beginning simulation\n");

    //create mutex lock
    pthread_mutex_init(&lock, NULL);
    pthread_mutex_init(&slot_lock, NULL);

    //number of students left to serve
    num_students_left = k;

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

    printf("Simulation end\n");
    return 0;
}