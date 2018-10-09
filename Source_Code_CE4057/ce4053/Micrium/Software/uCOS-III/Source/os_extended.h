//#include <bits/stdc++.h>

#include <stdlib.h>
#include <os.h>
//#include "fatal.h"

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

#ifndef _SKIPLIST_H
#define _SKIPLIST_H

typedef struct skiplist * Skiplist;
/* create an empty skiplist */
Skiplist skiplistCreate(void);

/* destroy a skiplist */
void skiplistDestroy(Skiplist s);

/* return maximum key less than or equal to key */
/* or INT_MIN if there is none */
int skiplistSearch(Skiplist s, int key);

/* insert a new key into s */
void skiplistInsert(Skiplist s, int key);

/* delete a key from s */
void skiplistDelete(Skiplist s, int key);

#endif /* _SKIPLIST_H */

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
