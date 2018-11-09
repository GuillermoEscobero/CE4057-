#include <stdlib.h>
#include <os.h>
#include "os_extended.h"


typedef void (*callback)(node* data);

/*
    create a new node
    initialize the data and next field

    return the newly created node
*/
listNodeQ* createQ(OS_TCB* data, EXT_MUTEX* mutex, listNodeQ* next)
{
    OS_ERR  err;
    listNodeQ* new_node = (listNodeQ*)OSMemGet(&CommMem2, &err);
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

    new_node->data = data;
    new_node->next = next;
    new_node->waitMu = mutex;

    return new_node;
}

/*
    add a new node at the beginning of the list
*/
listNodeQ* prependQ(listNodeQ* head,OS_TCB* data, EXT_MUTEX* mutex)
{
    listNodeQ* new_node = createQ(data, mutex, head);
    head = new_node;
    return head;
}

/*
    add a new node at the end of the list
*/
listNodeQ* appendQ(listNodeQ* head, OS_TCB* data, EXT_MUTEX* mutex)
{
    if(head == NULL)
        return NULL;
    /* go to the last node */
    listNodeQ *cursor = head;
    while(cursor->next != NULL)
        cursor = cursor->next;

    /* create a new node */
    listNodeQ* new_node =  createQ(data, mutex, NULL);
    cursor->next = new_node;

    return head;
}

/*
    removes and returns node from the front of list
    updates head of the list
*/
listNodeQ* remove_frontQ(listNodeQ** head)
{
    if(head == NULL)
        return NULL;
    listNodeQ *front = *head;
    (*head) = (*head)->next;
    front->next = NULL;
    /* is this the last node in the list */
    if(front == *head)
        head = NULL; //TODO: I think we should at a * to head

    // Warning: Memory is NOT freed

    return front;
}

/*
    remove node from the back of the list
    returns the removed back and updates
    head if necessary
*/
listNodeQ* remove_backQ(listNodeQ** head)
{
    if(*head == NULL)
        return NULL;

    listNodeQ *cursor = *head;
    listNodeQ *back = NULL;
    while(cursor->next != NULL)
    {
        back = cursor;
        cursor = cursor->next;
    }

    if(back != NULL)
        back->next = NULL;

    /* if this is the last node in the list*/
    if(cursor == *head)
        *head = NULL;

    // Warning: Memory is NOT freed

    return cursor;
}

/*
    remove a node from the list
    returns the removed node and updates
    head if necessary
*/
listNodeQ* remove_anyQ(listNodeQ** head,OS_TCB* p_tcb)
{
    if(p_tcb == NULL)
        return NULL;
    /* if the node is the first node */
    if(p_tcb == (*head)->data)
        return remove_frontQ(head);

    /* if the node is the last node */
    listNodeQ* cursor2 = *head;
    while(cursor2->next != NULL){
      cursor2 = cursor2->next;
    }
    if(p_tcb == cursor2->data)
        return remove_backQ(head);

    /* if the node is in the middle */
    listNodeQ* cursor = *head;
    while(cursor != NULL)
    {
        if(cursor->next->data == p_tcb) //does the next node contain the tcb?
            break;
        cursor = cursor->next;
    }

    if(cursor != NULL)
    {
        listNodeQ* tmp = cursor->next;
        cursor->next = tmp->next;
        tmp->next = NULL;
        // Warning: Memory is NOT freed
        return tmp;
    }
    //return head;
    return NULL; //the node was not found

}

/*
    Search for a specific node with input data

    return the first matched node that stores the input data,
    otherwise return NULL
*/
listNodeQ* searchQ(listNodeQ* head,OS_TCB* data)
{

    listNodeQ *cursor = head;
    while(cursor!=NULL)
    {
        if(cursor->data == data)
            return cursor;
        cursor = cursor->next;
    }
    return NULL;
}

/*
    remove all element of the list
*/
void disposeQ(listNodeQ *head)
{
    OS_ERR err;
    listNodeQ *cursor, *tmp;

    if(head != NULL)
    {
        cursor = head->next;
        head->next = NULL;
        OSMemPut(&CommMem2,head,&err);
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
        while(cursor != NULL)
        {
            tmp = cursor->next;

            OSMemPut(&CommMem2,cursor,&err);
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
            cursor = tmp;
        }
    }
}

listNodeQ* getHeadQ(listNodeQ* head){
  return head;
}
