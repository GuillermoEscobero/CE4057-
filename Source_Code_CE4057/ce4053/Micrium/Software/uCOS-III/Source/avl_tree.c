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

avlnode* avlTree = NULL;

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
avlnode* newNode(int key)
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

avlnode* avlInsert(avlnode* node, int key)
{
	/* 1. Perform the normal BST rotation */
	if (node == NULL)
		return(newNode(key));

	if (key < node->key)
		node->left = avlInsert(node->left, key);
	else if (key > node->key)
		node->right = avlInsert(node->right, key);
	else // Equal keys not allowed
		return node;

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

	/* loop down to find the leftmost leaf */
	while (current->left != NULL)
		current = current->left;

	return current;
}

// Recursive function to delete a node with given key
// from subtree with given root. It returns root of
// the modified subtree.
avlnode* avlDeleteNode(avlnode* root, int key)
{
	// STEP 1: PERFORM STANDARD BST DELETE

	if (root == NULL)
		return root;

	// If the key to be deleted is smaller than the
	// root's key, then it lies in left subtree
	if ( key < root->key )
		root->left = avlDeleteNode(root->left, key);

	// If the key to be deleted is greater than the
	// root's key, then it lies in right subtree
	else if( key > root->key )
		root->right = avlDeleteNode(root->right, key);

	// if key is same as root's key, then This is
	// the node to be deleted
	else
	{
		// node with only one child or no child
		if( (root->left == NULL) || (root->right == NULL) )
		{
			avlnode *temp = root->left ? root->left :
											root->right;

			// No child case
			if (temp == NULL)
			{
				temp = root;
				root = NULL;
			}
			else // One child case
			*root = *temp; // Copy the contents of
							// the non-empty child
			//free(temp); // TODO: OSMemGet, we have to decide if we should use different memory partitions
                        //Should we delete or return the node??
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
			avlnode* temp = minValueNode(root->right);

			// Copy the inorder successor's data to this node
			root->key = temp->key;

			// Delete the inorder successor
			root->right = avlDeleteNode(root->right, temp->key);
		}
	}

	// If the tree had only one node then return
	if (root == NULL)
	return root;

	// STEP 2: UPDATE HEIGHT OF THE CURRENT NODE
	root->height = 1 + max(height(root->left),
						height(root->right));

	// STEP 3: GET THE BALANCE FACTOR OF THIS NODE (to
	// check whether this node became unbalanced)
	int balance = getBalance(root);

	// If this node becomes unbalanced, then there are 4 cases

	// Left Left Case
	if (balance > 1 && getBalance(root->left) >= 0)
		return avlRightRotate(root);

	// Left Right Case
	if (balance > 1 && getBalance(root->left) < 0)
	{
		root->left = avlLeftRotate(root->left);
		return avlRightRotate(root);
	}

	// Right Right Case
	if (balance < -1 && getBalance(root->right) <= 0)
		return avlLeftRotate(root);

	// Right Left Case
	if (balance < -1 && getBalance(root->right) > 0)
	{
		root->right = avlRightRotate(root->right);
		return avlLeftRotate(root);
	}

	return root;
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
