#include "os_extended.h"
//#include <stdio.h>
#include <os.h>
//#include "fatal.h"

static RedBlackTree T;
 
typedef enum ColorType {
    Red, Black
} ColorType;
 
/*struct Element {
  int releaseTime;
};*/

struct RedBlackNode {
    //ElementType Element;
  ElementType ReleaseTime; //ElementType is int
  node *tasks; //list of tasks
    RedBlackTree Left;
    RedBlackTree Right;
    ColorType Color;
};
 
static Position NullNode = NULL; /* Needs initialization */
 
/* Initialization procedure */
//RedBlackTree
void Initialize(void) {
    //RedBlackTree T;
 
    if (NullNode == NULL) {
        NullNode = malloc(sizeof ( struct RedBlackNode));
        if (NullNode == NULL){
            //FatalError("Out of space!!!");
          //what to do: no FatalError definition
          return;
        }
        //NullNode->Left = NullNode->Right = NullNode;
        NullNode->Left = NullNode->Right = NULL;
        NullNode->Color = Black;
        NullNode->ReleaseTime = 12345;
    }
 
    /* Create the header node */
    T = malloc(sizeof ( struct RedBlackNode));
    if (T == NULL){
        //FatalError("Out of space!!!");
      //what to do: no FatalError definition
      return;
    }
    T->ReleaseTime = NegInfinity;
    T->Left = T->Right = NullNode;
    T->Color = Black;
 
    //return T;
}

RedBlackTree retrieveTree(void){ //added
  return T;
}
 
/* END */
 
void
Output(ElementType Element) {
    //printf("%d\n", Element);
}
 
/* Print the tree, watch out for NullNode, */
 
/* and skip header */
 
static void
DoPrint(RedBlackTree T) {
    if (T != NullNode) {
        DoPrint(T->Left);
        Output(T->ReleaseTime);
        DoPrint(T->Right);
    }
}
 
void
PrintTree(RedBlackTree T) {
    DoPrint(T->Right);
}
 
/* END */
 
static RedBlackTree
MakeEmptyRec(RedBlackTree T) {
    if (T != NullNode) {
        MakeEmptyRec(T->Left);
        MakeEmptyRec(T->Right);
        free(T);
    }
    return NullNode;
}
 
RedBlackTree
MakeEmpty(RedBlackTree T) {
    T->Right = MakeEmptyRec(T->Right);
    return T;
}
 
Position
Find(ElementType X, RedBlackTree T) {
    if (T == NullNode)
        return NullNode;
    if (X < T->ReleaseTime)
        return Find(X, T->Left);
    else
        if (X > T->ReleaseTime)
        return Find(X, T->Right);
    else
        return T;
}
 
Position
FindMin(RedBlackTree T) {
    T = T->Right;
    while (T->Left != NullNode)
        T = T->Left;
 
    return T;
}
 
Position
FindMax(RedBlackTree T) {
    while (T->Right != NullNode)
        T = T->Right;
 
    return T;
}
 
/* This function can be called only if K2 has a left child */
/* Perform a rotate between a node (K2) and its left child */
 
/* Update heights, then return new root */
 
static Position
SingleRotateWithLeft(Position K2) {
    Position K1;
 
    K1 = K2->Left;
    K2->Left = K1->Right;
    K1->Right = K2;
 
    return K1; /* New root */
}
 
/* This function can be called only if K1 has a right child */
/* Perform a rotate between a node (K1) and its right child */
 
/* Update heights, then return new root */
 
static Position
SingleRotateWithRight(Position K1) {
    Position K2;
 
    K2 = K1->Right;
    K1->Right = K2->Left;
    K2->Left = K1;
 
    return K2; /* New root */
}
 
/* Perform a rotation at node X */
/* (whose parent is passed as a parameter) */
 
/* The child is deduced by examining Item */
 
static Position
Rotate(ElementType Item, Position Parent) {
 
    if (Item < Parent->ReleaseTime)
        return Parent->Left = Item < Parent->Left->ReleaseTime ?
            SingleRotateWithLeft(Parent->Left) :
        SingleRotateWithRight(Parent->Left);
    else
        return Parent->Right = Item < Parent->Right->ReleaseTime ?
            SingleRotateWithLeft(Parent->Right) :
        SingleRotateWithRight(Parent->Right);
}
 
static Position X, P, GP, GGP;
 
static
void HandleReorient(ElementType Item, RedBlackTree T) {
    X->Color = Red; /* Do the color flip */
    X->Left->Color = Black;
    X->Right->Color = Black;
 
    if (P->Color == Red) /* Have to rotate */ {
        GP->Color = Red;
        if ((Item < GP->ReleaseTime) != (Item < P->ReleaseTime))
            P = Rotate(Item, GP); /* Start double rotate */
        X = Rotate(Item, GGP);
        X->Color = Black;
    }
    T->Right->Color = Black; /* Make root black */
}
 
RedBlackTree
Insert(ElementType Item, OS_TCB *task, CPU_INT32U period, TaskInfo* taskInfo, RedBlackTree T) { //added OS_TCB (pointer or not?) and period
    X = P = GP = T;
    NullNode->ReleaseTime = Item;
    while (X->ReleaseTime != Item) /* Descend down the tree */ {
        GGP = GP;
        GP = P;
        P = X;
        if (Item < X->ReleaseTime)
            X = X->Left;
        else
            X = X->Right;
        if (X!=NullNode && (X->Left->Color == Red && X->Right->Color == Red))
            HandleReorient(Item, T);
    }
 
    if (X != NullNode)
        return NullNode; /* Duplicate */
        //I think this is the case where a node with the given time already exists
        //We have to add the new task to the list of the node with the given time.
        append(X->tasks, task, period, taskInfo); //cast pointer to int - remember to cast back
 
    X = malloc(sizeof ( struct RedBlackNode));
    if (X == NULL){
        //FatalError("Out of space!!!");
      //what to do: no FatalError definition
      return NULL;
    }
    //creates new node for given time with new list of tasks
    X->ReleaseTime = Item;
    X->tasks = create(task, period, taskInfo, NULL); //cast pointer to int - remember to cast back
    X->ReleaseTime = Item;
    X->Left = X->Right = NullNode;
 
    if (Item < P->ReleaseTime) /* Attach to its parent */
        P->Left = X;
    else
        P->Right = X;
    HandleReorient(Item, T); /* Color it red; maybe rotate */
 
    return T;
}
 
RedBlackTree
Remove(ElementType Item, RedBlackTree T) {
    //printf("Remove is unimplemented\n");
    if (Item)
        return T;
    return T;
}
 
ElementType
Retrieve(Position P) {
    return P->ReleaseTime;
}