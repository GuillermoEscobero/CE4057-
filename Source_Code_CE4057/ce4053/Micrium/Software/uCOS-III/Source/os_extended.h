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
