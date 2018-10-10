//#include <stdio.h>
#include <stdlib.h>
#include <os.h>
#include "os_extended.h"
 

 
typedef void (*callback)(node* data);
 
/*
    create a new node
    initialize the data and next field
 
    return the newly created node
*/
node* create(OS_TCB* data, CPU_INT32U period, TaskInfo* taskInfo, node* next)
{
    //node* new_node = (node*)malloc(sizeof(node));
    //malloc does not work
    OS_ERR  err;
    node* new_node = (node*)OSMemGet(&CommMem2, &err);
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
    /*
    if(new_node == NULL)
    {
        //printf("Error creating a new node.\n");
        exit(0);
    }
*/
    new_node->data = data; //these four lines does not work as intended. Go through values in debugger!!!!
    new_node->period = period;
    new_node->next = next;
    new_node->taskInfo = taskInfo;
 
    return new_node;
}
 
/*
    add a new node at the beginning of the list
*/
node* prepend(node* head,OS_TCB* data, CPU_INT32U period, TaskInfo* taskInfo)
{
    node* new_node = create(data, period, taskInfo, head);
    head = new_node;
    return head;
}
 
/*
    add a new node at the end of the list
*/
node* append(node* head, OS_TCB* data, CPU_INT32U period, TaskInfo* taskInfo)
{
    if(head == NULL)
        return NULL;
    /* go to the last node */
    node *cursor = head;
    while(cursor->next != NULL)
        cursor = cursor->next;
 
    /* create a new node */
    node* new_node =  create(data, period, taskInfo, NULL);
    cursor->next = new_node;
 
    return head;
}
 
/*
    insert a new node after the prev node
*/
node* insert_after(node *head, OS_TCB* data, CPU_INT32U period, TaskInfo* taskInfo, node* prev)
{
    if(head == NULL || prev == NULL)
        return NULL;
    /* find the prev node, starting from the first node*/
    node *cursor = head;
    while(cursor != prev)
        cursor = cursor->next;
 
    if(cursor != NULL)
    {
        node* new_node = create(data, period, taskInfo, cursor->next);
        cursor->next = new_node;
        return head;
    }
    else
    {
        return NULL;
    }
}
 
/*
    insert a new node before the nxt node
*/
node* insert_before(node *head, OS_TCB* data, CPU_INT32U period, TaskInfo* taskInfo, node* nxt)
{
    if(nxt == NULL || head == NULL)
        return NULL;
 
    if(head == nxt)
    {
        head = prepend(head,data, period, taskInfo);
        return head;
    }
 
    /* find the prev node, starting from the first node*/
    node *cursor = head;
    while(cursor != NULL)
    {
        if(cursor->next == nxt)
            break;
        cursor = cursor->next;
    }
 
    if(cursor != NULL)
    {
        node* new_node = create(data, period, taskInfo, cursor->next);
        cursor->next = new_node;
        return head;
    }
    else
    {
        return NULL;
    }
}
 
/*
    traverse the linked list
*/
void traverse(node* head,callback f)
{
    node* cursor = head;
    while(cursor != NULL)
    {
        f(cursor);
        cursor = cursor->next;
    }
}
/*
    removes and returns node from the front of list
    updates head of the list
*/
node* remove_front(node** head)
{
    if(head == NULL)
        return NULL;
    node *front = *head;
    (*head) = (*head)->next;
    front->next = NULL;
    /* is this the last node in the list */
    if(front == *head)
        head = NULL;
    /*
    OS_ERR err;
    OSMemPut(&CommMem2, (void *)front, &err);

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
    */
    
    //free(front);
    return front;
}
 
/*
    remove node from the back of the list
*/
node* remove_back(node* head)
{
    if(head == NULL)
        return NULL;
 
    node *cursor = head;
    node *back = NULL;
    while(cursor->next != NULL)
    {
        back = cursor;
        cursor = cursor->next;
    }
 
    if(back != NULL)
        back->next = NULL;
 
    /* if this is the last node in the list*/
    if(cursor == head)
        head = NULL;
 
    free(cursor);
 
    return head;
}
 
///*
//    remove a node from the list
//*/
//node* remove_any(node* head,node* nd)
//{
//    if(nd == NULL)
//        return NULL;
//    /* if the node is the first node */
//    if(nd == head)
//        return remove_front(head);
// 
//    /* if the node is the last node */
//    if(nd->next == NULL)
//        return remove_back(head);
// 
//    /* if the node is in the middle */
//    node* cursor = head;
//    while(cursor != NULL)
//    {
//        if(cursor->next == nd)
//            break;
//        cursor = cursor->next;
//    }
// 
//    if(cursor != NULL)
//    {
//        node* tmp = cursor->next;
//        cursor->next = tmp->next;
//        tmp->next = NULL;
//        free(tmp);
//    }
//    return head;
// 
//}
/*
    display a node
*/
void display(node* n)
{
  if(n != NULL){
        //printf("%d ", n->data);
  }
}
 
/*
    Search for a specific node with input data
 
    return the first matched node that stores the input data,
    otherwise return NULL
*/
node* search(node* head,OS_TCB* data)
{
 
    node *cursor = head;
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
void dispose(node *head)
{
  OS_ERR  err; //memory error handling
    node *cursor, *tmp;
 
    if(head != NULL)
    {
        cursor = head->next;
        head->next = NULL;
        while(cursor != NULL)
        {
            tmp = cursor->next;
            //free(cursor); //we should use OSMemPut here instead
            OSMemPut(&CommMem2,cursor,&err); //commMem2 is the memory partition
            cursor = tmp;
        }
    }
}
/*
    return the number of elements in the list
*/
int count(node *head)
{
    node *cursor = head;
    int c = 0;
    while(cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}
/*
    sort the linked list using insertion sort
*/
node* insertion_sort(node* head)
{
    node *x, *y, *e;
 
    x = head;
    head = NULL;
 
    while(x != NULL)
    {
        e = x;
        x = x->next;
        if (head != NULL)
        {
            if(e->data > head->data)
            {
                y = head;
                while ((y->next != NULL) && (e->data> y->next->data))
                {
                    y = y->next;
                }
                e->next = y->next;
                y->next = e;
            }
            else
            {
                e->next = head;
                head = e ;
            }
        }
        else
        {
            e->next = NULL;
            head = e ;
        }
    }
    return head;
}
 
/*
    reverse the linked list
*/
node* reverse(node* head)
{
    node* prev    = NULL;
    node* current = head;
    node* next;
    while (current != NULL)
    {
        next  = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    head = prev;
    return head;
}

node* getHead(node* head){
  return head;
}