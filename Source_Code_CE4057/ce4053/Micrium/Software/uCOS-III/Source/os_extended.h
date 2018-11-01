#include <stdlib.h>
#include <os.h>

//TCB information

typedef struct TaskInfo {
  OS_TCB        *p_tcb;
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
} TaskInfo;

#ifndef _MEM_H
#define _MEM_H

#define N_BLKS 48
#define BLK_SIZE 16

extern OS_MEM CommMem2; // memory heap

#endif  /* _MEM_H */


#ifndef _LinkedList_H
#define _LinkedList_H

//typedef struct node node;
typedef struct node
{
    OS_TCB* data;
    CPU_INT32U period;
    TaskInfo* taskInfo;
    struct node* next;
} node;
typedef void (*callback)(node* data);

node* create(OS_TCB* data, CPU_INT32U period, TaskInfo* taskInfo, node* next);
node* prepend(node* head,OS_TCB* data, CPU_INT32U period, TaskInfo* taskInfo);
node* append(node* head, OS_TCB* data, CPU_INT32U period, TaskInfo* taskInfo);
node* insert_after(node *head, OS_TCB* data, CPU_INT32U period, TaskInfo* taskInfo, node* prev);
node* insert_before(node *head, OS_TCB* data, CPU_INT32U period, TaskInfo* taskInfo, node* nxt);
void traverse(node* head,callback f);
node* remove_front(node** head);
node* remove_back(node** head);
node* remove_any(node** head,OS_TCB* p_tcb);
void display(node* n);
node* search(node* head,OS_TCB* data);
void dispose(node *head);
int count(node *head);
node* insertion_sort(node* head);
node* reverse(node* head);

#endif  /* _LinkedList_H */


#ifndef _RBTree_H
#define _RBTree_H

struct rbtNode {
  int key; //this should be the releaseTime
  node *tasks; //list of tasks
  char color;
  struct rbtNode *left, *right,*parent;
};

void leftRotate(struct rbtNode *x);
void rightRotate(struct rbtNode *y);
void color_insert(struct rbtNode *z);
void insert(int val, OS_TCB *task, CPU_INT32U period, TaskInfo* taskInfo);
void inorderTree(struct rbtNode* root);
struct rbtNode* searchRB(int val);
struct rbtNode* min(struct rbtNode *x);
struct rbtNode* RBFindMin();
struct rbtNode* successor(struct rbtNode *x);
void color_delete(struct rbtNode *x);
struct rbtNode* delete(int var);

#endif  /* _RBTree_H */

#ifndef _SKIPLIST_H
#define _SKIPLIST_H

struct skiplist {
    int key;                   /* period */
    int height;                /* number of next pointers */
    node* tasks;               /* list of tasks with this period */
    struct skiplist *next[1];  /* first of many */
};

typedef struct skiplist * Skiplist;

extern Skiplist readyQueue;

/* create an empty skiplist */
void skiplistCreate(void);

/* destroy a skiplist */
void skiplistDestroy(Skiplist s);

/* return maximum key less than or equal to key */
/* or INT_MIN if there is none */
Skiplist skiplistSearch(Skiplist s, int key);

/* insert a new key into s */
void skiplistInsert(Skiplist s, int key, OS_TCB *p_tcb, CPU_INT32U period);

/* delete a key from s */
void skiplistDelete(Skiplist s, int key, OS_TCB* p_tcb);

struct skiplist* getMinKeyNode(struct skiplist* list);

#endif /* _SKIPLIST_H */

/************* OS_RecTaskServices *************/

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

void OSTaskDelRecursive (OS_TCB  *p_tcb,
                          OS_ERR  *p_err);

void tickHandlerRecursion();
void releaseTask(node* taskNode);
OS_TCB* RMSched();


#ifndef _AVLTREE_H
#define _AVLTREE_H



// An AVL tree node
typedef struct Node
{
	int key; //priority
        node* tasks; //tasks waiting for a mutex at this priority
	struct Node *left;
	struct Node *right;
	int height;
} avlnode;

extern avlnode* waitQueue; //Global avlTree

// A utility function to get maximum of two integers
int max(int a, int b);

// A utility function to get height of the tree
int height(avlnode *N);

/* Helper function that allocates a new node with the given key and
	NULL left and right pointers. */
avlnode* newNode(int key);

// A utility function to right rotate subtree rooted with y
// See the diagram given above.
avlnode *avlRightRotate(avlnode *y);

// A utility function to left rotate subtree rooted with x
// See the diagram given above.
avlnode*avlLeftRotate(avlnode *x);

// Get Balance factor of node N
int getBalance(avlnode *N);

avlnode* avlInsert(avlnode* node, int key, OS_TCB* p_tcb);

/* Given a non-empty binary search tree, return the
node with minimum key value found in that tree.
Note that the entire tree does not need to be
searched. */
avlnode * minValueNode(avlnode* node);

// Recursive function to delete a node with given key
// from subtree with given root. It returns root of
// the modified subtree.
OS_TCB* avlDeleteNode(avlnode** root, int key, OS_TCB* p_tcb);

// A utility function to print preorder traversal of
// the tree.
// The function also prints height of every node
void preOrder(avlnode *root);

#endif /* _AVLTREE_H */


/*
 * Author:  zentut.com
 * Purpose: linked stack header file
 */
#ifndef LINKEDSTACK_H_INCLUDED
#define LINKEDSTACK_H_INCLUDED

typedef struct stackNode{
    int data;
    struct stackNode* next;
} stacknode;

extern stacknode* ceilingStack;
 
int empty(struct node *s);
stacknode* push(stacknode *s,int data);
stacknode* pop(stacknode *s,int *data);
void init(stacknode* s);
CPU_INT32U peek(stacknode* head);
//void display(stacknode* head);
#endif // LINKEDSTACK_H_INCLUDED


#ifndef EXT_MUTEX_H
#define EXT_MUTEX_H

struct  ext_mutex {
    OS_OBJ_TYPE          Type;                              /* Mutual Exclusion Semaphore                             */
    CPU_CHAR            *NamePtr;                           /* Should be set to OS_OBJ_TYPE_MUTEX                     */
    OS_PEND_LIST         PendList;                          /* Pointer to Mutex Name (NUL terminated ASCII)           */
#if OS_CFG_DBG_EN > 0u
    OS_MUTEX            *DbgPrevPtr;
    OS_MUTEX            *DbgNextPtr;
    CPU_CHAR            *DbgNamePtr;
#endif
    OS_TCB              *OwnerTCBPtr;                       /* List of tasks waiting on event flag group              */
    OS_PRIO              OwnerOriginalPrio;
    OS_NESTING_CTR       OwnerNestingCtr;                   /* Mutex is available when the counter is 0               */
    CPU_TS               TS;
    
    int                  resourceCeiling; //added
};

typedef  struct  ext_mutex            EXT_MUTEX;


#endif // EXT_MUTEX_H