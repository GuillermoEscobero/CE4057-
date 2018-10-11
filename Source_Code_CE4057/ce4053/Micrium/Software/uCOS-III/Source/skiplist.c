#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
//#include <string.h>

#include "os_extended.h"

#define MAX_HEIGHT (32)

Skiplist readyQueue = NULL;

/* choose a height according to a geometric distribution */
static int
chooseHeight(void)
{
    int i;

    for(i = 1; i < MAX_HEIGHT && rand() % 2 == 0; i++);

    return i;
}

/* create a skiplist node with the given key and height */
/* does not fill in next pointers */
static Skiplist
skiplistCreateNode(CPU_INT32U key, int height, OS_TCB *p_tcb, CPU_INT32U period)
{
  Skiplist s = NULL;
  if(readyQueue != NULL){
    s = skiplistSearch(readyQueue, key);
    if(s!=NULL){
      append(s->tasks, p_tcb, period, NULL);
      //insert_after(s->tasks, p_tcb, period, NULL, NULL)
      return NULL;
    }
  }

    // assert(height > 0);
    if (height <= 0) {
      exit(0);
    }

    // assert(height <= MAX_HEIGHT);
    if (height > MAX_HEIGHT) {
      exit(0);
    }

    OS_ERR  err;
    s = (Skiplist) OSMemGet(&CommMem2, &err);
    // s = malloc(sizeof(struct skiplist) + sizeof(struct skiplist *) * (height - 1));

    switch(err){
      case OS_ERR_NONE:
        break;
      case OS_ERR_MEM_INVALID_P_MEM:
        exit(0);
        break;
      case OS_ERR_MEM_NO_FREE_BLKS:
        exit(0);
        break;
      case OS_ERR_OBJ_TYPE:
        exit(0);
        break;
    }

    // assert(s);
    
    if(readyQueue != NULL){
      s->tasks = create(p_tcb, period, NULL, NULL);
    }
    s->key = key;
    s->height = height;

    return s;
}

/* create an empty skiplist */
void //Skiplist
skiplistCreate(void) //remember to call this function at some point in time!! Initializes readyQueue!!
{
    //Skiplist s;
    //readyQueue = skiplistCreate(); //This is a recursive call?
    int i;

    /* s is a dummy head element */
    readyQueue = skiplistCreateNode(INT_MIN, MAX_HEIGHT, NULL, NULL);//no TCB and no period for head

    /* this tracks the maximum height of any node */
    readyQueue->height = 1;

    for(i = 0; i < MAX_HEIGHT; i++) {
        readyQueue->next[i] = 0;
    }

    //return s;
}

/* free a skiplist */
void
skiplistDestroy(Skiplist s)
{
    Skiplist next;

    OS_ERR err;

    while(s) {
        next = s->next[0];
        
        memset((void *) s, 0, (size_t) BLK_SIZE*sizeof(CPU_INT32U));
        OSMemPut(&CommMem2, (void *)s, &err);
        // free(s);

        switch(err){
          case OS_ERR_NONE:
            break;
          case OS_ERR_MEM_FULL:
            exit(0);
            break;
          case OS_ERR_MEM_INVALID_P_BLK:
            exit(0);
            break;
          case OS_ERR_MEM_INVALID_P_MEM:
            exit(0);
            break;
          case OS_ERR_OBJ_TYPE:
            exit(0);
            break;
        }

        s = next;
    }
}

/* return maximum key less than or equal to key */
/* or INT_MIN if there is none */
Skiplist skiplistSearch(Skiplist s, int key)
{
    int level;

    for(level = s->height - 1; level >= 0; level--) {
        while(s->next[level] && s->next[level]->key <= key) {
            s = s->next[level];
        }
    }
    if(s->key == key)
      return s;
    else
      return NULL;
}

/* insert a new key into s */
void
skiplistInsert(Skiplist s, int key, OS_TCB *p_tcb, CPU_INT32U period)
{
    int level;
    Skiplist elt;

    elt = skiplistCreateNode(key, chooseHeight(), p_tcb, period);
    if(elt == NULL){
      return;
    }
    // assert(elt);

    if(elt->height > s->height) {
        s->height = elt->height;
    }

    /* search through levels taller than elt */
    for(level = s->height - 1; level >= elt->height; level--) {
        while(s->next[level] && s->next[level]->key < key) {
            s = s->next[level];
        }
    }

    /* now level is elt->height - 1, we can start inserting */
    for(; level >= 0; level--) {
        while(s->next[level] && s->next[level]->key < key) {
            s = s->next[level];
        }

        /* s is last entry on this level < new element */
        /* do list insert */
        elt->next[level] = s->next[level];
        s->next[level] = elt;
    }
}

/* delete a key from s */
void
skiplistDelete(Skiplist s, int key)
{
    int level;
    Skiplist target;

    /* first we have to find leftmost instance of key */
    target = s;

    for(level = s->height - 1; level >= 0; level--) {
        while(target->next[level] && target->next[level]->key < key) {
            target = target->next[level];
        }
    }

    /* take one extra step at bottom */
    target = target->next[0];

    if(target == 0 || target->key != key) {
        return;
    }

    /* now we found target, splice it out */
    for(level = s->height - 1; level >= 0; level--) {
        while(s->next[level] && s->next[level]->key < key) {
            s = s->next[level];
        }

        if(s->next[level] == target) {
            s->next[level] = target->next[level];
        }
    }
    OS_ERR err;
    memset((void *) target, 0, (size_t) BLK_SIZE*sizeof(CPU_INT32U));
    OSMemPut(&CommMem2, (void *)target, &err);

    switch(err){
      case OS_ERR_NONE:
        break;
      case OS_ERR_MEM_FULL:
        exit(0);
        break;
      case OS_ERR_MEM_INVALID_P_BLK:
        exit(0);
        break;
      case OS_ERR_MEM_INVALID_P_MEM:
        exit(0);
        break;
      case OS_ERR_OBJ_TYPE:
        exit(0);
        break;
    }

    // free(target);
}

struct skiplist* getMinKeyNode2(struct skiplist* list){
  return list->next[0]; //get the first element of the first level (level 0)
}
