//NAME: Cody Hubbard
//EMAIL: CodySpHubbard@Gmail.com
//ID: 004843389
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <sched.h>
#include <string.h>
#include "SortedList.h"

//counter, lock, mutex, INIZALIZE
long long counter = 0;
long long spec_iterations = 1;
long long spec_threads = 1;
pthread_mutex_t* locks;
long long* my_spin_locks;

#define NO_LOCK 'n'
#define MUTEX 'm'
#define SPIN_LOCK 's'
#define COMPARE_AND_SWAP 'c'

char which_lock = 'n';
int opt_yield = 0;
char* yield_opts = NULL;
int lists = 1;

//list initialization
char *junk = "123456789abcdefghijklmnopqrstuvABCDEFGHIJKLMNOPQRSTUVWXYZ";
int keylength=5;
SortedList_t *mlist;
SortedListElement_t **mlistitems;
SortedList_t **msublists;


//used for lock wait clac
long long thread_wait_times[32] = {0};

// int hash(const char *str)
// {
    // int total = 0;
    // for (int i; i < keylength; i++)
    // {
        // total += str[i];
    // }
	// int temp = total%lists;
	// printf("%d",temp);
    // return total%lists;
// }

//thread list operations
void* thread_function(void *arg) { 
    //declare parameters needed

    long long startpos = ((*(int *)arg)*spec_iterations);
    long long endpos = startpos+spec_iterations;
    
    long long  sublistlength = 0;
    long long  listlength = 0;
    
    long long wait_for_lock_thread = 0;
    
    int which_list = 0;
    struct timespec startwaittimeM;
    struct timespec endwaittimeM;

//INSERT STUFF
    for(int i = startpos; i < endpos; i++) 
    {
        //which_list = hash(mlistitems[i]->key);
		which_list = ((long long)(mlistitems[i]->key))%lists;
        
        clock_gettime(CLOCK_MONOTONIC, &startwaittimeM);
        if (which_lock == 'm')
		{
            pthread_mutex_lock(&locks[which_list]);
		}
        else if (which_lock == 's')
		{
            while(__sync_lock_test_and_set(&(my_spin_locks[which_list]), 1));
        }
		clock_gettime(CLOCK_MONOTONIC, &endwaittimeM);

        SortedList_insert(msublists[which_list], mlistitems[i]);
        
        if (which_lock == 'm')
		{
            pthread_mutex_unlock(&locks[which_list]);
        }
		else if (which_lock == 's')
        {
			__sync_lock_release(&my_spin_locks[which_list]);
        }
		wait_for_lock_thread 	 = 	endwaittimeM.tv_nsec - startwaittimeM.tv_nsec;
		wait_for_lock_thread 	+= 	1000000000 * (endwaittimeM.tv_sec - startwaittimeM.tv_sec);
		thread_wait_times[*(int *)arg] += wait_for_lock_thread;
   }
    
//LIST LENGTHS            
    clock_gettime(CLOCK_MONOTONIC, &startwaittimeM);
    if (which_lock == 'm')
    {
        for(int k = 0; k < lists; k++)
        {
            pthread_mutex_lock(&locks[k]);
        }
    }
    else if (which_lock == 's')
    {
        for(int k = 0; k < lists; k++)
        {
    	    while(__sync_lock_test_and_set(&(my_spin_locks[k]), 1));
        }   
    }
    clock_gettime(CLOCK_MONOTONIC, &endwaittimeM);

    for (int k = 0; k < lists; k++)
    {
        sublistlength = 0;
        sublistlength = SortedList_length(msublists[k]);
        if (sublistlength < 0)
        {
    	    fprintf(stderr, "Error: List corrupted!\n");
    	    exit(2);
        }
        listlength += sublistlength;
    }
    
    if (which_lock == 'm')
    {
        for(int k = 0; k < lists; k++)
        {
            pthread_mutex_unlock(&locks[k]);
        }
    }
    else if (which_lock == 's')
    {
        for(int k = 0; k < lists; k++)
        {
			__sync_lock_release(&my_spin_locks[k]);
        }   
    }
    
	wait_for_lock_thread 	 = 	endwaittimeM.tv_nsec - startwaittimeM.tv_nsec;
	wait_for_lock_thread 	+= 	1000000000 * (endwaittimeM.tv_sec - startwaittimeM.tv_sec); 
	thread_wait_times[*(int *)arg] += wait_for_lock_thread;
    
//DELETE ELEMENTS
    SortedListElement_t* deleteMe;
    for(long long i = startpos; i < endpos; i++) 
    {
        which_list = ((long long)(mlistitems[i]->key))%lists;   
        clock_gettime(CLOCK_MONOTONIC, &startwaittimeM);
        if (which_lock == 'm')
		{
			pthread_mutex_lock(&locks[which_list]);
        }
		else if (which_lock == 's')
		{        
			while(__sync_lock_test_and_set(&(my_spin_locks[which_list]), 1));
        }
		clock_gettime(CLOCK_MONOTONIC, &endwaittimeM);

        deleteMe = SortedList_lookup(msublists[which_list], mlistitems[i]->key);
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
    	    
        if (which_lock == 'm')
        {
			pthread_mutex_unlock(&locks[which_list]);
        }
		else if (which_lock == 's')
        {
			__sync_lock_release(&my_spin_locks[which_list]);
        }	
		wait_for_lock_thread 	 = 	endwaittimeM.tv_nsec - startwaittimeM.tv_nsec;
		wait_for_lock_thread 	+= 	1000000000 * (endwaittimeM.tv_sec - startwaittimeM.tv_sec);
		thread_wait_times[*(int *)arg] += wait_for_lock_thread;
    }
    
    return NULL;
}


#define THREADS 't'
#define ITERATIONS 'i'
#define YIELD 'y'
#define SYNC 's'
#define LISTS 'l'

int main(int argc, char **argv) 
{
    struct option long_options[] =
    {
        {"threads", required_argument, NULL, THREADS},
        {"iterations", required_argument, NULL, ITERATIONS},
        {"yield", required_argument, NULL, YIELD},
        {"sync", required_argument, NULL, SYNC},
        {"lists", required_argument, NULL, LISTS},
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
            case LISTS:
            {
                lists = atoi(optarg);
                break;
            }
            default:
            {
                fprintf(stderr, "Unrecognized argument\n");
                exit(1);
            }
			
        }
    }
    //thread waiting time variables
    long long wait_for_lock_total = 0;
    
    //lock iniatalization
	if(which_lock == 'm')
    {
        locks = malloc(sizeof(pthread_mutex_t)*lists);
        for (int i = 0; i < lists; i++)
        {
            locks[i] = (pthread_mutex_t) PTHREAD_MUTEX_INITIALIZER;
        }
    }
	if(which_lock == 's')
	{
		my_spin_locks = malloc(sizeof(long long)*lists);
		for (int i = 0; i < lists; i++)
		{
            my_spin_locks[i] = 0;
		}
	}
	
    //main list creation
    long long total_elements = spec_threads*spec_iterations;
    
	mlist = malloc(sizeof(SortedList_t*));
	mlist->prev = mlist;
	mlist->next = mlist;
	mlist->key = NULL;
	
	//sublist creation & initalization
	msublists = malloc(lists * sizeof(SortedList_t*));
    for (int i = 0; i < lists; i++)
    {
        msublists[i] = malloc(sizeof(SortedList_t));
	    msublists[i]->prev = msublists[i];
	    msublists[i]->next = msublists[i];
	    msublists[i]->key = NULL;
    }
    
	// initialize the list elements with random keys
	mlistitems = malloc(total_elements * sizeof(SortedListElement_t*));
	for (long long j = 0; j < total_elements; j++) {
		mlistitems[j] = malloc(sizeof(SortedListElement_t*));
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
    free(msublists);
    
    if (SortedList_length(mlist) != 0)
    {
        fprintf(stderr, "Error: List corrupted!\n");
    	exit(2);
    }
	
    //calculate the elapsed time waiting for locks 
    for (int k=0; k<spec_threads; k++)
    {
        wait_for_lock_total += thread_wait_times[k];
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
	printf(",%lld,%lld,%d,%lld,%lld,%lld,%lld\n",spec_threads,spec_iterations,lists,op_pref,my_elapsed_time_in_ns,avg_time_per_op,wait_for_lock_total);
    exit(0);
}