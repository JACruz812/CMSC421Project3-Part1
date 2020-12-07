
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>

#define NUMBER_THREADS 10 
#define RANDOM_MAX 50

/**
 * You need to use this to increment the sums
 */
#define INCREMENT(what, by_how_much) do {\
    uint64_t curr = what;\
    if (by_how_much < RANDOM_MAX / 2) sched_yield();\
    what = curr + by_how_much;\
} while(0)

/**
 * Aggregate sum calculated by all the threads
 */
static uint64_t total_sum = 0;

//mutex lock used to exclude other threads
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

/**
 * Individual sums for each thread
 */
static uint64_t local_sums[NUMBER_THREADS];

/**
 * This is the code each of the threads will execute
 */
void* thread_function(void* arg)
{
    int my_id = *(int*)arg;
    local_sums[my_id] = 0;

    for (size_t i = 0; i < 500; i++)
    {
        // Random number in [0, RANDOM_MAX]
        uint32_t number = (rand() % (RANDOM_MAX + 1));
        INCREMENT(local_sums[my_id], number);
	pthread_mutex_lock(&lock);
        INCREMENT(total_sum, number);
	pthread_mutex_unlock(&lock);
    }

    printf("Thread %d finished and calculated: %ld\n", my_id, local_sums[my_id]);
    return NULL;
}




int main(int argc, char** argv)
{
    time_t t;
    srand(time(&t));

    pthread_t thread_ids[NUMBER_THREADS];
    int thread_args[NUMBER_THREADS];

    // Create all the threads
    for (size_t i = 0; i < NUMBER_THREADS; i++)
    {
        thread_args[i] = i;
        // The thread starts working after this function call finishes
        pthread_create(&thread_ids[i], NULL, thread_function, &thread_args[i]);
    }
    
    // We wait for each of the threads to finish here
    for (size_t i = 0; i < NUMBER_THREADS; i++)
    {
        pthread_join(thread_ids[i], NULL);
    }

    // Manually sum the individual numbers. This is the correct total sum
    uint64_t real_sum = 0;
    for (size_t i = 0; i < NUMBER_THREADS; i++)
    {
        real_sum += local_sums[i];
    }
    
    // Print Color
    int ret;
    if (real_sum == total_sum) 
    {
        // Will make the text green
        printf("\033[1;32m");
        ret = 0;
    }
    else
    {
        // Will make the text red
        printf("\033[1;31m");
        ret = 1;
    }

    printf("All threads are done. Total sum is %ld, expected %ld\n", total_sum, real_sum);

    // Will reset the color
    printf("\033[0m");

    return ret;
}
