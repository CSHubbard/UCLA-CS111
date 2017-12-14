//NAME: Cody Hubbard
//EMAIL: CodySpHubbard@Gmail.com
//ID: 004843389
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <time.h>
#include <sched.h>

//declare some global variables according to my implementation
long long counter = 0;
long long spec_iterations = 1;
long long spec_threads = 1;
int opt_yield = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
//counter, lock, mutex, INIZALIZE

void add(long long *pointer, long long value) 
{
    long long sum = *pointer + value;
    if (opt_yield)
        sched_yield();
    *pointer = sum;
}

#define NO_LOCK 'n'
#define MUTEX 'm'
#define SPIN_LOCK 's'
#define COMPARE_AND_SWAP 'c'
char which_lock = 'n';
long long my_spin_lock = 0;

//preforms the additions (with locking)
void* thread_function(void *arg) { 
    //declare parameters needed
    //first preform num_of_iterations times +1 
    
    //junk code to remove warning
    int* warning_remover= arg;
    warning_remover++;
    
    
    int addval = 0;
    for (int k = 0; k < 2; k++)
    {
        if (k == 0)
            addval = 1;
        else if (k == 1)
            addval = -1;
        for (int i = 0; i < spec_iterations; ++i)
        {
            switch (which_lock) 
            {
                case NO_LOCK:
                {
                    add(&counter, addval);
                    break;
                }
                case MUTEX:
                {
                    pthread_mutex_lock(&lock);
                    add(&counter, addval);
                    pthread_mutex_unlock(&lock);
                    break;
                }
                case SPIN_LOCK:
                {
                    while(__sync_lock_test_and_set(&my_spin_lock, 1));
                    add(&counter, addval);
                    __sync_lock_release(&my_spin_lock);
                    break;
                }
                case COMPARE_AND_SWAP:
                {
                    long long temp = 0;
                    do 
                    {
    		            temp = counter;
                    }while(__sync_val_compare_and_swap(&counter, temp, temp+addval) != temp);
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    return NULL;
}

#define THREADS 't'
#define ITERATIONS 'i'
#define YIELD 'y'
#define SYNC 's'

int main(int argc, char **argv) 
{
    //using while loop and getop_long() to process options
    //initalize some variables in case needed
    //collect start times
    struct option long_options[] =
    {
        {"threads", required_argument, NULL, THREADS},
        {"iterations", required_argument, NULL, ITERATIONS},
        {"yield", no_argument, NULL, YIELD},
        {"sync", required_argument, NULL, SYNC},
        {0,0,0,0}
    };
    while(1)
    {
        int ret = getopt_long(argc, argv, "", long_options, NULL);
        if (ret == -1) 
        {
            break;
        }
        
        switch (ret) 
        {
            case THREADS:
            {
                spec_threads = atoi(optarg);
                break;
            }
            case ITERATIONS:
            {
                spec_iterations = atoi(optarg);
                break;
            }
            case YIELD:
            {
                opt_yield =1;
                break;
            }
            case SYNC:
            {
                which_lock = optarg[0];
                if((which_lock != 'm') && (which_lock != 's') && (which_lock != 'c'))
                {
                    fprintf(stderr, "Unrecognized lock argument, correct usage includes: --sync=m, s, or c\n");
                    exit(1);  
                }
                break;
            }
            default:
            {
                fprintf(stderr, "Unrecognized argument\n");
                exit(1);
            }
			
        }
    }
    //for thread creation
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*spec_threads);
    
    struct timespec starttime;
    clock_gettime(CLOCK_MONOTONIC, &starttime);
    
    //start threads
    for (int i = 0; i < spec_threads; ++i) {
        int ret = pthread_create(&tid[i], NULL, thread_function, NULL);
        if(ret != 0)
        {
          fprintf(stderr, "Error: Thread create failure\n");
          exit(1);
        }
        
    }
    
    //wait for all threads to exit
    for (int i = 0; i < spec_threads; ++i) {
        int ret = pthread_join(tid[i], NULL);
        if(ret != 0)
        {
          fprintf(stderr, "Error: Thread join failure\n");
          exit(1);
        }
    }
    
    //collect end timespe
    struct timespec endtime;
    clock_gettime(CLOCK_MONOTONIC, &endtime);
    free(tid);
    
    //calculate the elapsed time
    long long my_elapsed_time_in_ns = 
        (endtime.tv_sec - starttime.tv_sec)*1000000000;
    my_elapsed_time_in_ns += endtime.tv_nsec;
    my_elapsed_time_in_ns -= starttime.tv_nsec;
    
    //report data
    long long op_pref = spec_threads*spec_iterations*2;
//    long long my_elapsed_timeHOLDER = 5; 
    long long avg_time_per_op = my_elapsed_time_in_ns/op_pref;
    printf("add");
    if(opt_yield)
		printf("-yield");
	if(which_lock == 'n')
		printf("-none");
	else if(which_lock == 'm')
		printf("-m");
	else if(which_lock == 's')
		printf("-s");
	else if(which_lock == 'c')
		printf("-c");
	printf(",%lld,%lld,%lld,%lld,%lld,%lld\n",spec_threads,spec_iterations,op_pref,my_elapsed_time_in_ns,avg_time_per_op,counter);
    //exit
    exit(0);
}