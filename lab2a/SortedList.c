//NAME: Cody Hubbard
//EMAIL: CodySpHubbard@Gmail.com
//ID: 004843389
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include "SortedList.h"

/**
 * SortedList (and SortedListElement)
 *
 *	A doubly linked list, kept sorted by a specified key.
 *	This structure is used for a list head, and each element
 *	of the list begins with this structure.
 *
 *	The list head is in the list, and an empty list contains
 *	only a list head.  The next pointer in the head points at
 *      the first (lowest valued) elment in the list.  The prev
 *      pointer in the list head points at the last (highest valued)
 *      element in the list.
 *
 *      The list head is also recognizable by its NULL key pointer.
 *
 * NOTE: This header file is an interface specification, and you
 *       are not allowed to make any changes to it.
 */
 /*
struct SortedListElement {
	struct SortedListElement *prev;
	struct SortedListElement *next;
	const char *key;
};
typedef struct SortedListElement SortedList_t;
typedef struct SortedListElement SortedListElement_t;
*/


/**
 * SortedList_insert ... insert an element into a sorted list
 *
 *	The specified element will be inserted in to
 *	the specified list, which will be kept sorted
 *	in ascending order based on associated keys
 *
 * @param SortedList_t *list ... header for the list
 * @param SortedListElement_t *element ... element to be added to the list
 */
void SortedList_insert(SortedList_t *list, SortedListElement_t *element) 
{
	SortedList_t *root = list;
	SortedList_t *temp = list->next;

	while (temp != root) 
	{
		if (strcmp(element->key, temp->key) <= 0) 
		{
			break;
		}
		
	    if (opt_yield & INSERT_YIELD) 
	    {
	    	sched_yield();
    	}
		
		temp = temp->next;
	}

	element->prev = temp->prev;
	element->next = temp;
	temp->prev->next = element;
	temp->prev = element;
}


/**
 * SortedList_delete ... remove an element from a sorted list
 *
 *	The specified element will be removed from whatever
 *	list it is currently in.
 *
 *	Before doing the deletion, we check to make sure that
 *	next->prev and prev->next both point to this node
 *
 * @param SortedListElement_t *element ... element to be removed
 *
 * @return 0: element deleted successfully, 1: corrtuped prev/next pointers
 *
 */
int SortedList_delete( SortedListElement_t *element)
{
    SortedListElement_t *temp = element->next;
    if (temp->prev!=element)
        return 1;
        
    temp=element->prev;
    if (temp->next!=element)
        return 1;
    
    if (opt_yield & DELETE_YIELD)
        sched_yield();
      
    element->next->prev = element->prev;
    element->prev->next = element->next;
    return 0;
    
}

/**
 * SortedList_lookup ... search sorted list for a key
 *
 *	The specified list will be searched for an
 *	element with the specified key.
 *
 * @param SortedList_t *list ... header for the list
 * @param const char * key ... the desired key
 *
 * @return pointer to matching element, or NULL if none is found
 */
SortedListElement_t *SortedList_lookup(SortedList_t *list, const char *key)
{
    SortedListElement_t *temp = list;
    while ((strcmp(temp->next->key, key) != 0)&&(temp->next->key!=NULL))
    {
        temp=temp->next;
            
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
            
    }
    if (strcmp(temp->next->key, key) == 0)
        return temp->next;
    else 
        return NULL;
}

/**
 * SortedList_length ... count elements in a sorted list
 *	While enumeratign list, it checks all prev/next pointers
 *
 * @param SortedList_t *list ... header for the list
 *
 * @return int number of elements in list (excluding head)
 *	   -1 if the list is corrupted
 */
int SortedList_length(SortedList_t *list)
{
    int count =0;
    SortedListElement_t *temp = list;
    SortedListElement_t *tempprev = NULL;
    SortedListElement_t *tempnext = temp->next;
    
    while(temp->next->key!=NULL)
    {
        count++;
        
        if((tempprev!=NULL)&&(tempprev->next!=temp))
            return -1;
        if((tempnext!=NULL)&&(tempnext->prev!=temp))
            return -1;
        
        if (opt_yield & LOOKUP_YIELD)
            sched_yield();
        
        temp=temp->next;
		tempprev = temp->prev;   
		tempnext = temp->next;
        
    }
    return count;
}

/**
 * variable to enable diagnositc yield calls
 */
 /*
extern int opt_yield;
#define	INSERT_YIELD	0x01	// yield in insert critical section
#define	DELETE_YIELD	0x02	// yield in delete critical section
#define	LOOKUP_YIELD	0x04	// yield in lookup/length critical esction
*/
