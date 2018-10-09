//#include <bits/stdc++.h>

#include <stdlib.h>
#include <os.h>
//#include "fatal.h"
 
//TCB information

/*
typedef struct TCBInfo {          
  CPU_CHAR      *p_name;
  OS_TASK_PTR    p_task;
  void          *p_arg;
  OS_PRIO        prio;
  CPU_STK       *p_stk_base;
  CPU_STK_SIZE   stk_limit;
  CPU_STK_SIZE   stk_size;
  OS_MSG_QTY     q_size;
  OS_TICK        time_quanta;
  void          *p_ext;
  OS_OPT         opt;
  OS_ERR        *p_err;
} TCBInfo; 
*/

typedef struct TCBInfo {          
  CPU_STK             *StkPtr;                            /* Pointer to current top of stack                        */
  void                *ExtPtr;                            /* Pointer to user definable data for TCB extension       */
  CPU_STK             *StkLimitPtr;                       /* Pointer used to set stack 'watermark' limit            */
  OS_TCB              *NextPtr;                           /* Pointer to next     TCB in the TCB list                */
  OS_TCB              *PrevPtr;                           /* Pointer to previous TCB in the TCB list                */
  OS_TCB              *TickNextPtr;
  OS_TCB              *TickPrevPtr;
  OS_TICK_SPOKE       *TickSpokePtr;                      /* Pointer to tick spoke if task is in the tick list      */
  CPU_CHAR            *NamePtr;                           /* Pointer to task name                                   */
  CPU_STK             *StkBasePtr;                        /* Pointer to base address of stack                       */
  OS_TASK_PTR          TaskEntryAddr;                     /* Pointer to task entry point address                    */
  void                *TaskEntryArg;                      /* Argument passed to task when it was created            */
  OS_PEND_DATA        *PendDataTblPtr;                    /* Pointer to list containing objects pended on           */
  OS_STATE             PendOn;                            /* Indicates what task is pending on                      */
  OS_STATUS            PendStatus;                        /* Pend status                                            */
  OS_STATE             TaskState;                         /* See OS_TASK_STATE_xxx                                  */
  OS_PRIO              Prio;                              /* Task priority (0 == highest)                           */
  CPU_STK_SIZE         StkSize;                           /* Size of task stack (in number of stack elements)       */
  //OS_OPT               Opt;                               /* Task options as passed by OSTaskCreate()               */
  //OS_OBJ_QTY           PendDataTblEntries;                /* Size of array of objects to pend on                    */
  //CPU_TS               TS;                                /* Timestamp                                              */
  //OS_SEM_CTR           SemCtr;                            /* Task specific semaphore counter                        */                                                          /* DELAY / TIMEOUT                                        */
  //OS_TICK              TickCtrPrev;                       /* Previous time when task was            ready           */
  //OS_TICK              TickCtrMatch;                      /* Absolute time when task is going to be ready           */
  //OS_TICK              TickRemain;                        /* Number of ticks remaining for a match (updated at ...  */                                                          /* ... run-time by OS_StatTask()                          */
  //OS_TICK              TimeQuanta;
  //OS_TICK              TimeQuantaCtr;
} TCBInfo; 

/*RBTree*/
#ifndef _RedBlack_H
#define _RedBlack_H

typedef int ElementType;
#define NegInfinity (-10000)

 
struct RedBlackNode;
typedef struct RedBlackNode *Position;
typedef struct RedBlackNode *RedBlackTree;
 
RedBlackTree MakeEmpty(RedBlackTree T);
Position Find(ElementType X, RedBlackTree T);
Position FindMin(RedBlackTree T);
Position FindMax(RedBlackTree T);
void Initialize(void); //return type changed to void from RedBlackTree
RedBlackTree Insert(ElementType Item, OS_TCB *task, CPU_INT32U period, TCBInfo* tcbInfo, RedBlackTree T);
RedBlackTree Remove(ElementType X, RedBlackTree T);
ElementType Retrieve(Position P);
void PrintTree(RedBlackTree T);
RedBlackTree retrieveTree(void); //added
 
#endif  /* _RedBlack_H */






#ifndef _LinkedList_H
#define _LinkedList_H
 
//typedef struct node node;
typedef struct node
{
    OS_TCB* data;
    CPU_INT32U period;
    TCBInfo* tcbInfo;
    struct node* next;
} node;
typedef void (*callback)(node* data);

node* create(OS_TCB* data, CPU_INT32U period, TCBInfo* tcbInfo, node* next);
node* prepend(node* head,OS_TCB* data, CPU_INT32U period, TCBInfo* tcbInfo);
node* append(node* head, OS_TCB* data, CPU_INT32U period, TCBInfo* tcbInfo);
node* insert_after(node *head, OS_TCB* data, CPU_INT32U period, TCBInfo* tcbInfo, node* prev);
node* insert_before(node *head, OS_TCB* data, CPU_INT32U period, TCBInfo* tcbInfo, node* nxt);
void traverse(node* head,callback f);
node* remove_front(node* head);
node* remove_back(node* head);
node* remove_any(node* head,node* nd);
void display(node* n);
node* search(node* head,OS_TCB* data);
void dispose(node *head);
int count(node *head);
node* insertion_sort(node* head);
node* reverse(node* head);

#endif  /* _LinkedList_H */




#ifndef _RedBlack2_H
#define _RedBlack2_H

//struct rbtNode;  
struct rbtNode {          
  int key; //this should be the releaseTime
  node *tasks; //list of tasks
  char color;
  struct rbtNode *left, *right,*parent;
};         
//struct rbtNode* root = NULL; //should maybe not be here

void leftRotate(struct rbtNode *x);
void rightRotate(struct rbtNode *y);
void color_insert(struct rbtNode *z);
void insert(int val, OS_TCB *task, CPU_INT32U period, TCBInfo* tcbInfo);
void inorderTree(struct rbtNode* root);
struct rbtNode* searchRB(int val);
struct rbtNode* min(struct rbtNode *x);
struct rbtNode* RBFindMin();
struct rbtNode* successor(struct rbtNode *x);
void color_delete(struct rbtNode *x);
struct rbtNode* delete(int var);

#endif  /* _RedBlack2_H */

/*OS_RecTaskServices*/
void OSTaskCreateRecursive(OS_TCB        *p_tcb,
                    CPU_CHAR      *p_name,
                    OS_TASK_PTR    p_task,
                    void          *p_arg,
                    OS_PRIO        prio,
                    CPU_STK       *p_stk_base,
                    CPU_STK_SIZE   stk_limit,
                    CPU_STK_SIZE   stk_size,
                    OS_MSG_QTY     q_size,
                    OS_TICK        time_quanta,
                    void          *p_ext,
                    OS_OPT         opt,
                    OS_ERR        *p_err);

void tickHandlerRecursion();
void releaseTask(node* taskNode);