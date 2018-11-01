/*
 * File:    stack.c
 * Author:  zentut.com
 * Purpose: linked stack implementation
 */
 
#include <stdio.h>

stacknode* ceilingStack = NULL;
 
/*
    init the stack
*/
void init(stacknode* head)
{
  //I feel like there is a crucial mistake here
  //I think that the assignment is only valid for the local variable head wihtin this function call!!!!
    head = NULL; 
}
//maybe
//void init(stacknode** p_head){
//      *head = NULL;
//}
 
/*
    push an element into stack
*/
stacknode* push(stacknode* head,int data)
{
//    stacknode* tmp = (stacknode*)malloc(sizeof(stacknode));
//    if(tmp == NULL)
//    {
//        exit(0);
//    }
  
      OS_ERR  err;
      node =(avlnode*) OSMemGet(&CommMem2, &err);
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
        
        
    tmp->data = data;
    tmp->next = head;
    head = tmp;
    return head;
}
/*
    pop an element from the stack
*/
stacknode* pop(stacknode *head,int *element)
{
    stacknode* tmp = head;
    *element = head->data;
    head = head->next;
    //free(tmp);
    //Should we free the memory or return the node instead?
    OS_ERR err;
    OSMemPut(&CommMem2,tmp,&err); //should we check if temp is NULL?
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
    return head;
}

//returns the value at the top of the stack
CPU_INT32U peek(stacknode* head){
  return head->data;
}


/*
    returns 1 if the stack is empty, otherwise returns 0
*/
int empty(stacknode* head)
{
    return head == NULL ? 1 : 0;
}