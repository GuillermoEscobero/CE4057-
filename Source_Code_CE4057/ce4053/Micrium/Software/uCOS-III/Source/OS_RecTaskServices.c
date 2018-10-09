#include "os_extended.h"
#include <os.h>
/**
  Selected copy paste from OSTaskCreate function
*/
     
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
                    OS_ERR        *p_err)
{
  
  //Maybe we may let this function call the OSTaskCreate function
  //Before doing that we will however need to ensure that we put the task in the datastructure
  //keeping track of recursion
  //How do we know when to put the task back in the recursion structure?
  //Should it always be there such that we only update the time at which it is ready: that sounds like a good idea!!
  
  
      CPU_STK_SIZE   i;
      CPU_INT32U period = (CPU_INT32U) p_ext; //added

#if OS_CFG_TASK_REG_TBL_SIZE > 0u
    OS_OBJ_QTY     reg_nbr;
#endif

    CPU_STK       *p_sp;
    CPU_STK       *p_stk_limit;
    CPU_SR_ALLOC();



#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#ifdef OS_SAFETY_CRITICAL_IEC61508
    if (OSSafetyCriticalStartFlag == DEF_TRUE) {
       *p_err = OS_ERR_ILLEGAL_CREATE_RUN_TIME;
        return;
    }
#endif

#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u
    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {              /* ---------- CANNOT CREATE A TASK FROM AN ISR ---------- */
        *p_err = OS_ERR_TASK_CREATE_ISR;
        return;
    }
#endif

#if OS_CFG_ARG_CHK_EN > 0u                                  /* ---------------- VALIDATE ARGUMENTS ------------------ */
    if (p_tcb == (OS_TCB *)0) {                             /* User must supply a valid OS_TCB                        */
        *p_err = OS_ERR_TCB_INVALID;
        return;
    }
    if (p_task == (OS_TASK_PTR)0) {                         /* User must supply a valid task                          */
        *p_err = OS_ERR_TASK_INVALID;
        return;
    }
    if (p_stk_base == (CPU_STK *)0) {                       /* User must supply a valid stack base address            */
        *p_err = OS_ERR_STK_INVALID;
        return;
    }
    if (stk_size < OSCfg_StkSizeMin) {                      /* User must supply a valid minimum stack size            */
        *p_err = OS_ERR_STK_SIZE_INVALID;
        return;
    }
    if (stk_limit >= stk_size) {                            /* User must supply a valid stack limit                   */
        *p_err = OS_ERR_STK_LIMIT_INVALID;
        return;
    }
    if (prio >= OS_CFG_PRIO_MAX) {                          /* Priority must be within 0 and OS_CFG_PRIO_MAX-1        */
        *p_err = OS_ERR_PRIO_INVALID;
        return;
    }
#endif

#if OS_CFG_ISR_POST_DEFERRED_EN > 0u
    if (prio == (OS_PRIO)0) {
        if (p_tcb != &OSIntQTaskTCB) {
            *p_err = OS_ERR_PRIO_INVALID;                   /* Not allowed to use priority 0                          */
            return;
        }
    }
#endif

    if (prio == (OS_CFG_PRIO_MAX - 1u)) {
        if (p_tcb != &OSIdleTaskTCB) {
            *p_err = OS_ERR_PRIO_INVALID;                   /* Not allowed to use same priority as idle task          */
            return;
        }
    }

    OS_TaskInitTCB(p_tcb);                                  /* Initialize the TCB to default values                   */

    *p_err = OS_ERR_NONE;
                                                            /* --------------- CLEAR THE TASK'S STACK --------------- */
    if ((opt & OS_OPT_TASK_STK_CHK) != (OS_OPT)0) {         /* See if stack checking has been enabled                 */
        if ((opt & OS_OPT_TASK_STK_CLR) != (OS_OPT)0) {     /* See if stack needs to be cleared                       */
            p_sp = p_stk_base;
            for (i = 0u; i < stk_size; i++) {               /* Stack grows from HIGH to LOW memory                    */
                *p_sp = (CPU_STK)0;                         /* Clear from bottom of stack and up!                     */
                p_sp++;
            }
        }
    }
                                                            /* ------- INITIALIZE THE STACK FRAME OF THE TASK ------- */
#if (CPU_CFG_STK_GROWTH == CPU_STK_GROWTH_HI_TO_LO)
    p_stk_limit = p_stk_base + stk_limit;
#else
    p_stk_limit = p_stk_base + (stk_size - 1u) - stk_limit;
#endif

    p_sp = OSTaskStkInit(p_task,
                         p_arg,
                         p_stk_base,
                         p_stk_limit,
                         stk_size,
                         opt);

                                                            /* -------------- INITIALIZE THE TCB FIELDS ------------- */
    p_tcb->TaskEntryAddr = p_task;                          /* Save task entry point address                          */
    p_tcb->TaskEntryArg  = p_arg;                           /* Save task entry argument                               */

    p_tcb->NamePtr       = p_name;                          /* Save task name                                         */

    p_tcb->Prio          = prio;                            /* Save the task's priority                               */

    p_tcb->StkPtr        = p_sp;                            /* Save the new top-of-stack pointer                      */
    p_tcb->StkLimitPtr   = p_stk_limit;                     /* Save the stack limit pointer                           */

    p_tcb->TimeQuanta    = time_quanta;                     /* Save the #ticks for time slice (0 means not sliced)    */
#if OS_CFG_SCHED_ROUND_ROBIN_EN > 0u
    if (time_quanta == (OS_TICK)0) {
        p_tcb->TimeQuantaCtr = OSSchedRoundRobinDfltTimeQuanta;
    } else {
        p_tcb->TimeQuantaCtr = time_quanta;
    }
#endif
    p_tcb->ExtPtr        = p_ext;                           /* Save pointer to TCB extension                          */
    p_tcb->StkBasePtr    = p_stk_base;                      /* Save pointer to the base address of the stack          */
    p_tcb->StkSize       = stk_size;                        /* Save the stack size (in number of CPU_STK elements)    */
    p_tcb->Opt           = opt;                             /* Save task options                                      */

#if OS_CFG_TASK_REG_TBL_SIZE > 0u
    for (reg_nbr = 0u; reg_nbr < OS_CFG_TASK_REG_TBL_SIZE; reg_nbr++) {
        p_tcb->RegTbl[reg_nbr] = (OS_REG)0;
    }
#endif

#if OS_CFG_TASK_Q_EN > 0u
    OS_MsgQInit(&p_tcb->MsgQ,                               /* Initialize the task's message queue                    */
                q_size);
#endif

    OSTaskCreateHook(p_tcb);                                /* Call user defined hook                                 */

    //only for restoring TCB, when releasing
    OS_ERR  err;
    TCBInfo* tcbInfo = (TCBInfo*) OSMemGet(&CommMem2, &err);
    /*
    tcbInfo->p_name = p_name;
    tcbInfo->p_task = p_task;
    tcbInfo->p_arg = p_arg;
    tcbInfo->prio = prio;
    tcbInfo->p_stk_base = p_stk_base;
    tcbInfo->stk_limit = stk_limit;
    tcbInfo->stk_size = stk_size;
    tcbInfo->q_size = q_size;
    tcbInfo->time_quanta = time_quanta;
    tcbInfo->p_ext = p_ext;
    tcbInfo->opt = opt;
    */
    
    tcbInfo->StkPtr=p_tcb->StkPtr;
    tcbInfo->ExtPtr=p_tcb->ExtPtr;
    tcbInfo->StkLimitPtr=p_tcb->StkLimitPtr;
    tcbInfo->NextPtr=p_tcb->NextPtr;
    tcbInfo->PrevPtr=p_tcb->PrevPtr;
    tcbInfo->TickNextPtr=p_tcb->TickNextPtr;
    tcbInfo->TickPrevPtr=p_tcb->TickPrevPtr;
    tcbInfo->TickSpokePtr=p_tcb->TickSpokePtr;
    tcbInfo->NamePtr=p_tcb->NamePtr;
    tcbInfo->StkBasePtr=p_tcb->StkBasePtr;
    tcbInfo->TaskEntryAddr=p_tcb->TaskEntryAddr;
    tcbInfo->TaskEntryArg=p_tcb->TaskEntryArg;
    tcbInfo->PendDataTblPtr=p_tcb->PendDataTblPtr;
    tcbInfo->PendOn=p_tcb->PendOn;
    tcbInfo->PendStatus=p_tcb->PendStatus;
    tcbInfo->TaskState=p_tcb->TaskState;
    tcbInfo->Prio=p_tcb->Prio;
    tcbInfo->StkSize=p_tcb->StkSize;
//    tcbInfo->Opt=p_tcb->Opt;
//    tcbInfo->PendDataTblEntries=p_tcb->PendDataTblEntries;
//    tcbInfo->TS=p_tcb->TS;
//    tcbInfo->SemCtr=p_tcb->SemCtr;
//    tcbInfo->TickCtrPrev=p_tcb->TickCtrPrev;
//    tcbInfo->TickCtrMatch=p_tcb->TickCtrMatch;
//    tcbInfo->TickRemain=p_tcb->TickRemain;
//    tcbInfo->TimeQuanta=p_tcb->TimeQuanta;
//    tcbInfo->TimeQuantaCtr=p_tcb->TimeQuantaCtr;
    
    
    
                                                              /* --------------- ADD TASK TO READY LIST --------------- */
    OS_CRITICAL_ENTER();
    OS_PrioInsert(p_tcb->Prio);
    //RedBlackTree T = retrieveTree(); //retrieve the red-black tree for recursion //no longer needed for new RB-tree
    //Insert(0, p_tcb, T); //we want the task to run at time 0 //no longer needed for new RB-tree
    insert(0, p_tcb, period, tcbInfo); //we want the task to run at time 0
    //OS_RdyListInsertTail(p_tcb); //We will have to to call this function at the right times (when task should be repeated)

#if OS_CFG_DBG_EN > 0u
    OS_TaskDbgListAdd(p_tcb);
#endif

    OSTaskQty++;                                            /* Increment the #tasks counter                           */

    if (OSRunning != OS_STATE_OS_RUNNING) {                 /* Return if multitasking has not started                 */
        OS_CRITICAL_EXIT();
        return;
    }

    OS_CRITICAL_EXIT_NO_SCHED();

    OSSched();
}




void tickHandlerRecursion(){
  //OSTickCtr; //the current OSTick I think.
  
  struct rbtNode *minNode = RBFindMin();
  int minTime = minNode->key;
  int x = OSTickCtr;
  while(minTime <= OSTickCtr){
    //traverse the list of the node and make the tasks in the list ready
    //reinsert each task into the redblack tree at its new time
    //and free the redblacknode, the linked list and all the nodes of the linked list
    traverse(minNode->tasks, releaseTask); //f should be the function that does something for each node in the list of tasks.
    dispose(minNode->tasks); //remove all elements of the list
    //TODO: remove the list itself
    //TODO: remove the rbtNode
    delete(minTime); //remove rbtNode from tree (does not free memory)
    minNode = RBFindMin(); //THE problems seem to occur here as we do not retrieve our the correct node.
    if(minNode == NULL){
      break;
    }
    minTime = minNode->key;
  }
}

void releaseTask(node* taskNode){
  OS_TCB *p_tcb = (OS_TCB*) taskNode->data;
  TCBInfo* tcbInfo = taskNode->tcbInfo;
  p_tcb->TaskState = 0;
  /*
  //TCBInfo* tcbInfo = (TCBInfo*) OSMemGet();
  p_tcb->NamePtr = tcbInfo->p_name;
  //p_tcb->?? = tcbInfo->p_task;
  //p_tcb->?? = tcbInfo->p_arg;
  p_tcb->Prio = tcbInfo->prio;
  p_tcb->StkBasePtr = tcbInfo->p_stk_base;
  p_tcb->StkLimitPtr = tcbInfo->stk_limit;
  p_tcb->StkSize = tcbInfo->stk_size;
  //p_tcb->?? = tcbInfo->q_size;
  //p_tcb->?? = tcbInfo->time_quanta;
  //p_tcb->?? = tcbInfo->p_ext; //period
  //p_tcb->?? = tcbInfo->opt;
  */
  
  p_tcb->StkPtr=tcbInfo->StkPtr;
  p_tcb->ExtPtr=tcbInfo->ExtPtr;
  p_tcb->StkLimitPtr=tcbInfo->StkLimitPtr;
  p_tcb->NextPtr=tcbInfo->NextPtr;
  p_tcb->PrevPtr=tcbInfo->PrevPtr;
  p_tcb->TickNextPtr=tcbInfo->TickNextPtr;
  p_tcb->TickPrevPtr=tcbInfo->TickPrevPtr;
  p_tcb->TickSpokePtr=tcbInfo->TickSpokePtr;
  p_tcb->NamePtr=tcbInfo->NamePtr;
  p_tcb->StkBasePtr=tcbInfo->StkBasePtr;
  p_tcb->TaskEntryAddr=tcbInfo->TaskEntryAddr;
  p_tcb->TaskEntryArg=tcbInfo->TaskEntryArg;
  p_tcb->PendDataTblPtr=tcbInfo->PendDataTblPtr;
  p_tcb->PendOn=tcbInfo->PendOn;
  p_tcb->PendStatus=tcbInfo->PendStatus;
  p_tcb->TaskState=tcbInfo->TaskState;
  p_tcb->Prio=tcbInfo->Prio;
  p_tcb->StkSize=tcbInfo->StkSize;
//  p_tcb->Opt=tcbInfo->Opt;
//  p_tcb->PendDataTblEntries=tcbInfo->PendDataTblEntries;
//  p_tcb->TS=tcbInfo->TS;
//  p_tcb->SemCtr=tcbInfo->SemCtr;
//  p_tcb->TickCtrPrev=tcbInfo->TickCtrPrev;
//  p_tcb->TickCtrMatch=tcbInfo->TickCtrMatch;
//  p_tcb->TickRemain=tcbInfo->TickRemain;
//  p_tcb->TimeQuanta=tcbInfo->TimeQuanta;
//  p_tcb->TimeQuantaCtr=tcbInfo->TimeQuantaCtr;
  
  
  
  OS_RdyListInsertTail(p_tcb); //make task ready to run
  CPU_INT32U newRelease = OSTickCtr+taskNode->period;
  insert(newRelease, p_tcb, taskNode->period, tcbInfo); //reinsert into RB-tree
}