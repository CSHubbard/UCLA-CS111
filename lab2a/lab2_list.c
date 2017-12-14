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
#include <string.h>
#include "SortedList.h"

//declare some global variables according to my implementation
long long counter = 0;
long long spec_iterations = 1;
long long spec_threads = 1;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
//counter, lock, mutex, INIZALIZE

#define NO_LOCK 'n'
#define MUTEX 'm'
#define SPIN_LOCK 's'
#define COMPARE_AND_SWAP 'c'
char which_lock = 'n';
long long my_spin_lock = 0;
int opt_yield = 0;
char* yield_opts = NULL;

//list initialization
char *junk = "123456789abcdefghijklmnopqrstuvABCDEFGHIJKLMNOPQRSTUVWXYZ";
int keylength=5;
SortedList_t *mlist;
SortedListElement_t **mlistitems;

//thread list operations
void* thread_function(void *arg) { 
    //declare parameters needed

    long long startpos = ((*(int *)arg)*spec_iterations);
    long long endpos = startpos+spec_iterations;
    
    long long  listlength = 0;
    
    switch (which_lock) 
    {
        case NO_LOCK:
        {
            for(long long i = startpos; i < endpos; i++) 
            {
                SortedList_insert(mlist, mlistitems[i]);
            }
            
            listlength = SortedList_length(mlist);
            if ((listlength < 0)||(listlength != spec_iterations))
            {
            	fprintf(stderr, "Error: List corrupted!\n");
    			exit(2);
            }
            
            SortedListElement_t* deleteMe;
            
            for(long long i = startpos; i < endpos; i++) 
            {
                deleteMe = SortedList_lookup(mlist,mlistitems[i]->key);
                if (deleteMe == NULL)
                {
            	    fprintf(stderr, "Error: List corrupted!\n");
    			    exit(2);
                }
                if ((SortedList_delete(deleteMe)))
                {
            	    fprintf(stderr, "Error: List corrupted!\n");
    			    exit(2);
                }
            }
            
            break;
        }
        case MUTEX:
        {
            pthread_mutex_lock(&lock);
            
            for(int i = startpos; i < endpos; i++) 
            {
	            SortedList_insert(mlist, mlistitems[i]);
	        }
            
            listlength = SortedList_length(mlist);
            if ((listlength < 0)||(listlength != spec_iterations))
            {
            	fprintf(stderr, "Error: List corrupted!\n");
    			exit(2);
            }
            
            SortedListElement_t* deleteMe;
            for(int i = startpos; i < endpos; i++) 
            {
                deleteMe = SortedList_lookup(mlist,mlistitems[i]->key);
                if (deleteMe == NULL)
                {
            	    fprintf(stderr, "Error: List corrupted!\n");
    			    exit(2);
                }
                if ((SortedList_delete(deleteMe)))
                {
            	    fprintf(stderr, "Error: List corrupted!\n");
    			    exit(2);
                }
            }
            
            pthread_mutex_unlock(&lock);
            break;
        }
        case SPIN_LOCK:
        {
            while(__sync_lock_test_and_set(&my_spin_lock, 1));
            
            for(int i = startpos; i < endpos; i++) 
            {
                SortedList_insert(mlist, mlistitems[i]);
            }
            
            listlength = SortedList_length(mlist);
            if ((listlength < 0)||(listlength != spec_iterations))
            {
            	fprintf(stderr, "Error: List corrupted!\n");
    			exit(2);
            }
            
            SortedListElement_t* deleteMe;
            for(int i = startpos; i < endpos; i++) 
            {
                deleteMe = SortedList_lookup(mlist,mlistitems[i]->key);
                if (deleteMe == NULL)
                {
            	    fprintf(stderr, "Error: List corrupted!\n");
    			    exit(2);
                }
                if ((SortedList_delete(deleteMe)))
                {
            	    fprintf(stderr, "Error: List corrupted!\n");
    			    exit(2);
                }
            }
            
           __sync_lock_release(&my_spin_lock);
            break;
        }
        default:
        {
            break;
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
    struct option long_options[] =
    {
        {"threads", required_argument, NULL, THREADS},
        {"iterations", required_argument, NULL, ITERATIONS},
        {"yield", required_argument, NULL, YIELD},
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
                yield_opts = optarg;
				for(unsigned int i = 0; i < strlen(optarg); i++)
				{
					if(optarg[i] == 'i')
						opt_yield |= INSERT_YIELD;
					else if(optarg[i] == 'd')
						opt_yield |= DELETE_YIELD;
					else if(optarg[i] == 'l')
						opt_yield |= LOOKUP_YIELD;
					else{
						fprintf(stderr, "Unrecognized yield argument, correct usage includes: --yield[idl]\n");
						exit(1);
					}
				}
				break;
            }
            case SYNC:
            {
                which_lock = optarg[0];
                if((which_lock != 'm') && (which_lock != 's'))
                {
                    fprintf(stderr, "Unrecognized lock argument, correct usage includes: --sync=m or s\n");
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
    //list creation
    long long total_elements = spec_threads*spec_iterations;
    
	mlist = malloc(sizeof(SortedList_t));
	mlist->prev = mlist;
	mlist->next = mlist;
	mlist->key = NULL;
	
	mlistitems = malloc(total_elements * sizeof(SortedListElement_t*));

	// initialize the list elements with random keys
	for (long long j = 0; j < total_elements; j++) {
		mlistitems[j] = malloc(sizeof(SortedListElement_t));
		mlistitems[j]->prev = NULL;
		mlistitems[j]->next = NULL;
		
		char* newkey = malloc(sizeof(char)*(keylength+1));
		for(int k = 0; k < keylength; k++){
			newkey[k] = junk[rand() % strlen(junk)];
		}
		newkey[keylength]='\0';
		mlistitems[j]->key = newkey;
	}
    
    //for thread creation
    pthread_t* tid = (pthread_t*)malloc(sizeof(pthread_t)*spec_threads);
    
    struct timespec starttime;
    clock_gettime(CLOCK_MONOTONIC, &starttime);
    
    //start threads
    for (int i = 0; i < spec_threads; i++) {
        long long spot = i;
        int ret = pthread_create(&tid[i], NULL, thread_function, &spot);
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
    free(mlist);
    
    if (SortedList_length(mlist) != 0)
    {
        fprintf(stderr, "Error: List corrupted!\n");
    	exit(2);
    }
    
    //calculate the elapsed time
    long long my_elapsed_time_in_ns = 
        (endtime.tv_sec - starttime.tv_sec)*1000000000;
    my_elapsed_time_in_ns += endtime.tv_nsec;
    my_elapsed_time_in_ns -= starttime.tv_nsec;
    
    //report data
    long long op_pref = spec_threads*spec_iterations*3;
    //long long my_elapsed_timeHOLDER = 5; //change this later so that correct tiems are output 
    long long avg_time_per_op = my_elapsed_time_in_ns/op_pref;
    printf("list");
    if(yield_opts == NULL)
        printf("-none");
    else
		printf("-%s", yield_opts);
	if(which_lock == 'n')
		printf("-none");
	else if(which_lock == 'm')
		printf("-m");
	else if(which_lock == 's')
		printf("-s");
	printf(",%lld,%lld,%d,%lld,%lld,%lld\n",spec_threads,spec_iterations,1,op_pref,my_elapsed_time_in_ns,avg_time_per_op);
    //exit
    exit(0);
}