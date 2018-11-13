#include <stdlib.h>
#include "os_extended.h"
#include <os.h>

struct rbtNode* root = NULL;

void leftRotate(struct rbtNode *x){
  struct rbtNode *y;
  y = x->right;  x->right = y->left;
  if( y->left != NULL)
  {
    y->left->parent = x;
  }
  y->parent = x->parent;
  if( x->parent == NULL){
    root = y;
  }
  else if( (x->parent->left!=NULL) && (x->key == x->parent->left->key)){
    x->parent->left = y;
  }
  else
    x->parent->right = y;
  y->left = x; x->parent = y; return;
}

void rightRotate(struct rbtNode *y){
  struct rbtNode *x;
  x = y->left; y->left = x->right;
  if ( x->right != NULL){
    x->right->parent = y;
  }
  x->parent = y->parent;
  if( y->parent == NULL){
    root = x;
  }
  else if((y->parent->left!=NULL)&& (y->key == y->parent->left->key)){
    y->parent->left = x;
  }
  else
    y->parent->right = x;

  x->right = y; y->parent = x;
  return;
}

void color_insert(struct rbtNode *z){
  struct rbtNode *y=NULL;
  while ((z->parent != NULL) && (z->parent->color == 'r')){
    if ( (z->parent->parent->left != NULL) && (z->parent->key == z->parent->parent->left->key)){
      if(z->parent->parent->right!=NULL)
        y = z->parent->parent->right;
      if ((y!=NULL) && (y->color == 'r')){
        z->parent->color = 'b';
        y->color = 'b';
        z->parent->parent->color = 'r';
        if(z->parent->parent!=NULL)
          z = z->parent->parent;
      }
      else{
        if ((z->parent->right != NULL) && (z->key == z->parent->right->key)){
          z = z->parent;
          leftRotate(z);
        }
        z->parent->color = 'b';
        z->parent->parent->color = 'r';
        rightRotate(z->parent->parent);
      }
    }
    else{
      if(z->parent->parent->left!=NULL)
        y = z->parent->parent->left;
      if ((y!=NULL) && (y->color == 'r')){
        z->parent->color = 'b';
        y->color = 'b';
        z->parent->parent->color = 'r';
        if(z->parent->parent!=NULL)
          z = z->parent->parent;
      }
      else{
        if ((z->parent->left != NULL) && (z->key == z->parent->left->key)){
          z = z->parent;
          rightRotate(z);
        }
        z->parent->color = 'b';
        z->parent->parent->color = 'r';
        leftRotate(z->parent->parent);
      }
    }
  }
  root->color = 'b';
}

void insert(int val, OS_TCB *task, CPU_INT32U period, TaskInfo *taskInfo){
  struct rbtNode *z = searchRB(val);
  if(z!=NULL)
  {
    // This is the case where a node with the given time already exists
    // We have to add the new task to the list of the node with the given time.
    append(z->tasks, task, period, taskInfo); // Cast pointer to int - remember to cast back
    return;
  }
  struct rbtNode *x, *y;

  OS_ERR  err;
  z =(struct rbtNode*) OSMemGet(&CommMem2, &err);
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
  if(z==NULL){
    // printf("Error allocating z");
    exit(0);
  }
  z->key = val;
  z->tasks = create(task, period, taskInfo, NULL); // Cast pointer to int - remember to cast back
  z->left = NULL;
  z->right = NULL;
  z->color = 'r';
  x=root;

  if ( root == NULL )
  {
    root = z;
    root->color = 'b';
    return;
  }
  while ( x != NULL){
    y = x;
    if ( z->key < x->key){
      x = x->left;
    }
    else x = x->right;
  }
  z->parent = y; // set parent
  if ( y == NULL)
  {
    root = z;
  }
  else if( z->key < y->key ){ // set z-as the left or right child of y (parent)
    y->left = z;
  }
  else
    y->right = z;
  color_insert(z);
  return;
}

void inorderTree(struct rbtNode* root)

{          struct rbtNode* temp = root;

if (temp != NULL)

{          inorderTree(temp->left);

// printf(" %d--%c ",temp->key,temp->color);

inorderTree(temp->right);

}return;

}

void postorderTree(struct rbtNode* root)

{          struct rbtNode* temp = root;

if (temp != NULL)

{          postorderTree(temp->left);

postorderTree(temp->right);

// printf(" %d--%c ",temp->key,temp->color);

}return;

}

void traversal(struct rbtNode* root)

{          if (root != NULL)

{          // printf("root is %d-- %c",root->key,root->color);

// printf("\nInorder tree traversal\n");

inorderTree(root);

// printf("\npostorder tree traversal\n");

postorderTree(root);

}return;

}

struct rbtNode* searchRB(int val){
  struct rbtNode* temp = root;
  int diff;
  while (temp != NULL){
    diff = val - temp->key;
    if (diff > 0){
      temp = temp->right;
    }
    else if (diff < 0){
      temp = temp->left;
    }
    else{
      // printf("Search Element Found!!\n");
      return temp; // return node instead of just 1
    }
  }
  // printf("Given Data Not Found in RB Tree!!\n");
  return NULL;
}

struct rbtNode* RBFindMin(){
  return min(root);
}

struct rbtNode* min(struct rbtNode *x){
  if(x==NULL){
    return x;
  }
  while (x->left){
    x = x->left;
  }
  return x;
}

struct rbtNode* successor(struct rbtNode *x)

{          struct rbtNode *y;

if (x->right)

{          return min(x->right);

}          y = x->parent;

while (y && x == y->right)

{          x = y;

y = y->parent;

}return y;

}

void color_delete(struct rbtNode *x){
  while (x != root && x->color == 'b'){
    struct rbtNode *w = NULL;
    if ((x->parent->left!=NULL) && (x == x->parent->left)){
      w = x->parent->right;
      if ((w!=NULL) && (w->color == 'r')){
        w->color = 'b';
        x->parent->color = 'r';
        leftRotate(x->parent);
        w = x->parent->right;
      }

      if ((w!=NULL) && (w->right!=NULL) && (w->left!=NULL) && (w->left->color == 'b') && (w->right->color == 'b')){
        w->color = 'r';
        x = x->parent;
      }
      else if((w!=NULL) && (w->right->color == 'b')){
        w->left->color = 'b';
        w->color = 'r';
        rightRotate(w);
        w = x->parent->right;
      }

      if(w!=NULL){
        w->color = x->parent->color;
        x->parent->color = 'b';
        w->right->color = 'b';
        leftRotate(x->parent);
        x = root;
      }
    }
    else if(x->parent!=NULL){
      w = x->parent->left;
      if ((w!=NULL) && (w->color == 'r')){
        w->color = 'b';
        x->parent->color = 'r';
        leftRotate(x->parent);
        if(x->parent!=NULL)
          w = x->parent->left;
      }
      if ((w!=NULL) && (w->right!=NULL) && (w->left!=NULL) && (w->right->color == 'b') && (w->left->color == 'b')){
        x = x->parent;
      }
      else if((w!=NULL) && (w->right!=NULL) && (w->left!=NULL) && (w->left->color == 'b')){
        w->right->color = 'b';
        w->color = 'r';
        rightRotate(w);
        w = x->parent->left;
      }
      if(x->parent!=NULL){
        w->color = x->parent->color;
        x->parent->color = 'b';
      }

      if(w->left!=NULL)
        w->left->color = 'b';
      if(x->parent !=NULL)
        leftRotate(x->parent);
      x = root;
    }
  }
  x->color = 'b';
}

struct rbtNode* delete(int var){ //does this function take care of the fact that nodes contains a lists of tasks?? //Yep, it just returns the entire node.
  struct rbtNode *x = NULL, *y = NULL, *z;
  z=root;
  if((z->left ==NULL ) &&(z->right==NULL) && (z->key==var)){
    root=NULL;
    // printf("\nRBTREE is empty\n");
    return z;
  }

  while(z->key !=var && z!=NULL){
    if(var<z->key)
      z=z->left;
    else
      z=z->right;

    if(z== NULL)
     return NULL;
  }

  if((z->left==NULL)||(z->right==NULL)){
    y = z;
  }
  else{
    y = successor(z);
  }

  if (y->left!=NULL){
    x = y->left;
  }
  else{
    if(y->right !=NULL)
      x = y->right;
  }
  // setting the child of y's parent to y.
  if((x!=NULL)) // removed && y->parente != NULL
    x->parent = y->parent;

  if ((y !=NULL) && (x!=NULL) && (y->parent==NULL)){
      // x->parent = NULL;
      root=x; // root updated here when node->key = 0 exists
    // this one is skipped when node->key = 35000000
  }
  else if (y == y->parent->left){
    y->parent->left = x;
  }
  else{
    y->parent->right = x;
  }

  if (y != z){
    z->key = y->key;
  }

  if ((y!=NULL) && (x!=NULL) && (y->color == 'b')){
    color_delete(x);
  }
  return y;
}
