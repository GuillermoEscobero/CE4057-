#include<stdio.h>
#include<stdlib.h>
#include <os.h>
#include "os_extended.h"

// An AVL tree node
//already defined in os_extended.h
//typedef struct Node
//{
//  //We will probably need some data here
//	int key;
//	struct Node *left;
//	struct Node *right;
//	int height;
//} avlnode;

avlnode* waitQueue = NULL;

// A utility function to get maximum of two integers
int max(int a, int b);

// A utility function to get height of the tree
int height(avlnode *N)
{
	if (N == NULL)
		return 0;
	return N->height;
}

// A utility function to get maximum of two integers
int max(int a, int b)
{
	return (a > b)? a : b;
}

/* Helper function that allocates a new node with the given key and
	NULL left and right pointers. */
avlnode* newNode(int key, OS_TCB* tcb, EXT_MUTEX* mutex)
{
	//avlnode* node = (avlnode*) malloc(sizeof(avlnode));
        
        OS_ERR  err;
        avlnode* node =(avlnode*) OSMemGet(&CommMem2, &err);
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
        
	node->key = key;
	node->left = NULL;
	node->right = NULL;
        node->tasks = NULL; //To make sure the last element of the list always points to NULL
        node->tasks = prependQ(node->tasks,tcb, mutex);
	node->height = 1; // new node is initially added at leaf
	return(node);
}

// A utility function to right rotate subtree rooted with y
// See the diagram given above.
avlnode *avlRightRotate(avlnode *y)
{
	avlnode *x = y->left;
	avlnode *T2 = x->right;

	// Perform rotation
	x->right = y;
	y->left = T2;

	// Update heights
	y->height = max(height(y->left), height(y->right))+1;
	x->height = max(height(x->left), height(x->right))+1;

	// Return new root
	return x;
}

// A utility function to left rotate subtree rooted with x
// See the diagram given above.
avlnode *avlLeftRotate(avlnode *x)
{
	avlnode *y = x->right;
	avlnode *T2 = y->left;

	// Perform rotation
	y->left = x;
	x->right = T2;

	// Update heights
	x->height = max(height(x->left), height(x->right))+1;
	y->height = max(height(y->left), height(y->right))+1;

	// Return new root
	return y;
}

// Get Balance factor of node N
int getBalance(avlnode *N)
{
	if (N == NULL)
		return 0;
	return height(N->left) - height(N->right);
}

//Returns the root of the new tree
avlnode* avlInsert(avlnode* node, int key, OS_TCB* p_tcb, EXT_MUTEX* mutex) //last argument is the mutex, the task is waiting for
{
	/* 1. Perform the normal BST rotation */
	if (node == NULL)
		return(newNode(key, p_tcb, mutex));

	if (key < node->key)
		node->left = avlInsert(node->left, key, p_tcb, mutex);
	else if (key > node->key)
		node->right = avlInsert(node->right, key, p_tcb, mutex);
	else{ // Equal keys: append new tcb to list of tasks
                appendQ(node->tasks, p_tcb, mutex); //Beaware of the cast here; is it correct?
		return node;
        }

	/* 2. Update height of this ancestor node */
	node->height = 1 + max(height(node->left),
						height(node->right));

	/* 3. Get the balance factor of this ancestor
		node to check whether this node became
		unbalanced */
	int balance = getBalance(node);

	// If this node becomes unbalanced, then there are 4 cases

	// Left Left Case
	if (balance > 1 && key < node->left->key)
		return avlRightRotate(node);

	// Right Right Case
	if (balance < -1 && key > node->right->key)
		return avlLeftRotate(node);

	// Left Right Case
	if (balance > 1 && key > node->left->key)
	{
		node->left = avlLeftRotate(node->left);
		return avlRightRotate(node);
	}

	// Right Left Case
	if (balance < -1 && key < node->right->key)
	{
		node->right = avlRightRotate(node->right);
		return avlLeftRotate(node);
	}

	/* return the (unchanged) node pointer */
	return node;
}

/* Given a non-empty binary search tree, return the
node with minimum key value found in that tree.
Note that the entire tree does not need to be
searched. */
avlnode * minValueNode(avlnode* node)
{
	avlnode* current = node;
        if(current==NULL){
          return NULL;
        }
	/* loop down to find the leftmost leaf */
	while (current->left != NULL)
		current = current->left;

	return current;
}

// Recursive function to delete a node with given key
// from subtree with given root. It returns root of
// the modified subtree.

//Changes made: it takes a pointer to the root pointer
//it changes the root and returns the TCB of the task that can be released
//p_tcb is used to specify a certain tcb, that should be removed.
//the key should be the same as p_tcb->prio.
//if p_tcb==NULL, then any tcb may removed and returned from the node with the given key.
//TODO: This one does not update correctly the three
OS_TCB* avlDeleteNode(avlnode** root, int key, OS_TCB *p_tcb, EXT_MUTEX** mutex) //The last argument is an extra return value to get the mutex
{
  OS_TCB* p_tcb_ret; //the task to return
  //Be carefull as we have two different TCBs in this function
	// STEP 1: PERFORM STANDARD BST DELETE

	if (*root == NULL)
		return NULL;

	// If the key to be deleted is smaller than the
	// root's key, then it lies in left subtree
	if ( key < (*root)->key ){
          //as the function shoul modify the root itself, it should be necessary to return it
		//(*root)->left = avlDeleteNode(&((*root)->left), key, p_tcb);
                p_tcb_ret = avlDeleteNode(&((*root)->left), key, p_tcb, mutex);
        }

	// If the key to be deleted is greater than the
	// root's key, then it lies in right subtree
	else if( key > (*root)->key ){
          //as the function shoul modify the root itself, it should be necessary to return it
		//(*root)->right = avlDeleteNode(&((*root)->right), key, p_tcb);
                p_tcb_ret = avlDeleteNode(&((*root)->right), key, p_tcb, mutex);
        }

	// if key is same as root's key, then This is
	// the node to be deleted
	else
	{
          if(p_tcb == NULL){
            //first remove a tcb from the list of the node
            listNodeQ* tasknode = remove_frontQ(&((*root)->tasks));
            p_tcb_ret = tasknode->data;
            *mutex = tasknode->waitMu;
            //TODO: free the memory of the node
            //if list is not empty, then return without further changes
            if((*root)->tasks != NULL){
              return p_tcb_ret;
            }
          }
          else{ //we want to remove a given tcb
            listNodeQ* tasknode = remove_anyQ(&((*root)->tasks), p_tcb);
            if(tasknode == NULL){
              return NULL;
            }
            else{
              //OS_TCB* tcb_ret = tasknode->data; //I think htis line is wrong
              p_tcb_ret = tasknode->data;
              *mutex = tasknode->waitMu;
              //TODO: free the memory of the node
              //if list is not empty, then return without further changes
              if((*root)->tasks != NULL){
                return p_tcb_ret;
              }
            }
          }
          //list is empty and we have to remove the node
		// node with only one child or no child
		if( ((*root)->left == NULL) || ((*root)->right == NULL) )
		{
			avlnode *temp = (*root)->left ? (*root)->left :
											(*root)->right;

			// No child case
			if (temp == NULL)
			{
				temp = (*root);
				(*root) = NULL;
			}
			else // One child case
			**root = *temp; // Copy the contents of
							// the non-empty child
			//free(temp); // TODO: OSMemGet, we have to decide if we should use different memory partitions
                        //Should we delete or return the node??
                        //is it the correct data that we free??
                        OS_ERR err;
                        OSMemPut(&CommMem2,temp,&err); //should we check if temp is NULL?
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
		}
		else
		{
			// node with two children: Get the inorder
			// successor (smallest in the right subtree)
			avlnode* temp = minValueNode((*root)->right);

			// Copy the inorder successor's data to this node
			(*root)->key = temp->key;

			// Delete the inorder successor
			//(*root)->right = avlDeleteNode(&((*root)->right), temp->key);
                        //The root should be updated by the call below
                        p_tcb_ret = avlDeleteNode(&((*root)->right), temp->key, p_tcb, mutex);
		}
	}

	// If the tree had only one node then return
	if (*root == NULL){
          //return root;
          return p_tcb_ret;
        }

	// STEP 2: UPDATE HEIGHT OF THE CURRENT NODE
	(*root)->height = 1 + max(height((*root)->left),
						height((*root)->right));

	// STEP 3: GET THE BALANCE FACTOR OF THIS NODE (to
	// check whether this node became unbalanced)
	int balance = getBalance((*root));

	// If this node becomes unbalanced, then there are 4 cases

	// Left Left Case
	if (balance > 1 && getBalance((*root)->left) >= 0){
		//return avlRightRotate(*root);
          *root = avlRightRotate(*root);
          return p_tcb;
        }

	// Left Right Case
	if (balance > 1 && getBalance((*root)->left) < 0)
	{
		(*root)->left = avlLeftRotate((*root)->left);
		//return avlRightRotate(*root);
                (*root) = avlRightRotate(*root);
                return p_tcb;
	}

	// Right Right Case
	if (balance < -1 && getBalance((*root)->right) <= 0){
		//return avlLeftRotate(*root);
                *root = avlLeftRotate(*root);
                return p_tcb;
        }

	// Right Left Case
	if (balance < -1 && getBalance((*root)->right) > 0)
	{
		(*root)->right = avlRightRotate((*root)->right);
		//return avlLeftRotate(*root);
                *root = avlLeftRotate(*root);
                return p_tcb;
	}

	//return root; ?? its a TCB
        return p_tcb;
}

// A utility function to print preorder traversal of
// the tree.
// The function also prints height of every node
void preOrder(avlnode *root)
{
	if(root != NULL)
	{
		printf("%d ", root->key);
		preOrder(root->left);
		preOrder(root->right);
	}
}

// /* Driver program to test above function */
// int main()
// {
// avlnode *root = NULL;
//
// /* Constructing tree given in the above figure */
// 	root = avlInsert(root, 9);
// 	root = avlInsert(root, 5);
// 	root = avlInsert(root, 10);
// 	root = avlInsert(root, 0);
// 	root = avlInsert(root, 6);
// 	root = avlInsert(root, 11);
// 	root = avlInsert(root, -1);
// 	root = avlInsert(root, 1);
// 	root = avlInsert(root, 2);
//
// 	/* The constructed AVL Tree would be
// 			9
// 		/ \
// 		1 10
// 		/ \	 \
// 	0 5	 11
// 	/ / \
// 	-1 2 6
// 	*/
//
// 	printf("Preorder traversal of the constructed AVL "
// 		"tree is \n");
// 	preOrder(root);
//
// 	root = deleteNode(root, 10);
//
// 	/* The AVL Tree after deletion of 10
// 			1
// 		/ \
// 		0 9
// 		/	 / \
// 	-1 5	 11
// 		/ \
// 		2 6
// 	*/
//
// 	printf("\nPreorder traversal after deletion of 10 \n");
// 	preOrder(root);
//
// 	return 0;
// }
