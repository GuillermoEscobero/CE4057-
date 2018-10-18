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

      OS_ERR  err;
      TaskInfo* taskInfo = (TaskInfo*) OSMemGet(&CommMem2, &err);
      taskInfo->p_tcb = p_tcb;
      taskInfo->p_name = p_name;
      taskInfo->p_task = p_task;
      taskInfo->p_arg = p_arg;
      taskInfo->prio = prio;
      taskInfo->p_stk_base = p_stk_base;
      taskInfo->stk_limit = stk_limit;
      taskInfo->stk_size = stk_size;
      taskInfo->q_size = q_size;
      taskInfo->time_quanta = time_quanta;
      taskInfo->p_ext = p_ext;
      taskInfo->opt = opt;



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
    //OS_ERR  err;
//    TCBInfo* tcbInfo = (TCBInfo*) OSMemGet(&CommMem2, &err);
//    /*
//    tcbInfo->p_name = p_name;
//    tcbInfo->p_task = p_task;
//    tcbInfo->p_arg = p_arg;
//    tcbInfo->prio = prio;
//    tcbInfo->p_stk_base = p_stk_base;
//    tcbInfo->stk_limit = stk_limit;
//    tcbInfo->stk_size = stk_size;
//    tcbInfo->q_size = q_size;
//    tcbInfo->time_quanta = time_quanta;
//    tcbInfo->p_ext = p_ext;
//    tcbInfo->opt = opt;
//    */
//
//    tcbInfo->StkPtr=p_tcb->StkPtr;
//    tcbInfo->ExtPtr=p_tcb->ExtPtr;
//    tcbInfo->StkLimitPtr=p_tcb->StkLimitPtr;
//    tcbInfo->NextPtr=p_tcb->NextPtr;
//    tcbInfo->PrevPtr=p_tcb->PrevPtr;
//    tcbInfo->TickNextPtr=p_tcb->TickNextPtr;
//    tcbInfo->TickPrevPtr=p_tcb->TickPrevPtr;
//    tcbInfo->TickSpokePtr=p_tcb->TickSpokePtr;
//    tcbInfo->NamePtr=p_tcb->NamePtr;
//    tcbInfo->StkBasePtr=p_tcb->StkBasePtr;
//    tcbInfo->TaskEntryAddr=p_tcb->TaskEntryAddr;
//    tcbInfo->TaskEntryArg=p_tcb->TaskEntryArg;
//    tcbInfo->PendDataTblPtr=p_tcb->PendDataTblPtr;
//    tcbInfo->PendOn=p_tcb->PendOn;
//    tcbInfo->PendStatus=p_tcb->PendStatus;
//    tcbInfo->TaskState=p_tcb->TaskState;
//    tcbInfo->Prio=p_tcb->Prio;
//    tcbInfo->StkSize=p_tcb->StkSize;
////    tcbInfo->Opt=p_tcb->Opt;
////    tcbInfo->PendDataTblEntries=p_tcb->PendDataTblEntries;
////    tcbInfo->TS=p_tcb->TS;
////    tcbInfo->SemCtr=p_tcb->SemCtr;
////    tcbInfo->TickCtrPrev=p_tcb->TickCtrPrev;
////    tcbInfo->TickCtrMatch=p_tcb->TickCtrMatch;
////    tcbInfo->TickRemain=p_tcb->TickRemain;
////    tcbInfo->TimeQuanta=p_tcb->TimeQuanta;
////    tcbInfo->TimeQuantaCtr=p_tcb->TimeQuantaCtr;



                                                              /* --------------- ADD TASK TO READY LIST --------------- */
    OS_CRITICAL_ENTER();
    //OS_PrioInsert(p_tcb->Prio);
    //RedBlackTree T = retrieveTree(); //retrieve the red-black tree for recursion //no longer needed for new RB-tree
    //Insert(0, p_tcb, T); //we want the task to run at time 0 //no longer needed for new RB-tree
    insert(50, p_tcb, period, taskInfo); //we want the task to run at time 0
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

    OSSched(); //old scheduler
    //RMSched(); //new scheduler

}

void OSTaskReCreateRecursive(OS_TCB        *p_tcb,
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
      OS_OPT blah = opt;


      OS_ERR  err;
//      TaskInfo* taskInfo = (TaskInfo*) OSMemGet(&CommMem2, &err);
//      taskInfo->p_tcb = p_tcb;
//      taskInfo->p_name = p_name;
//      taskInfo->p_task = p_task;
//      taskInfo->p_arg = p_arg;
//      taskInfo->prio = prio;
//      taskInfo->p_stk_base = p_stk_base;
//      taskInfo->stk_limit = stk_limit;
//      taskInfo->stk_size = stk_size;
//      taskInfo->q_size = q_size;
//      taskInfo->time_quanta = time_quanta;
//      taskInfo->p_ext = p_ext;
//      taskInfo->opt = opt;



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

    //OSTaskCreateHook(p_tcb);                                /* Call user defined hook                                 */

    //only for restoring TCB, when releasing
    //OS_ERR  err;
//    TCBInfo* tcbInfo = (TCBInfo*) OSMemGet(&CommMem2, &err);
//    /*
//    tcbInfo->p_name = p_name;
//    tcbInfo->p_task = p_task;
//    tcbInfo->p_arg = p_arg;
//    tcbInfo->prio = prio;
//    tcbInfo->p_stk_base = p_stk_base;
//    tcbInfo->stk_limit = stk_limit;
//    tcbInfo->stk_size = stk_size;
//    tcbInfo->q_size = q_size;
//    tcbInfo->time_quanta = time_quanta;
//    tcbInfo->p_ext = p_ext;
//    tcbInfo->opt = opt;
//    */
//
//    tcbInfo->StkPtr=p_tcb->StkPtr;
//    tcbInfo->ExtPtr=p_tcb->ExtPtr;
//    tcbInfo->StkLimitPtr=p_tcb->StkLimitPtr;
//    tcbInfo->NextPtr=p_tcb->NextPtr;
//    tcbInfo->PrevPtr=p_tcb->PrevPtr;
//    tcbInfo->TickNextPtr=p_tcb->TickNextPtr;
//    tcbInfo->TickPrevPtr=p_tcb->TickPrevPtr;
//    tcbInfo->TickSpokePtr=p_tcb->TickSpokePtr;
//    tcbInfo->NamePtr=p_tcb->NamePtr;
//    tcbInfo->StkBasePtr=p_tcb->StkBasePtr;
//    tcbInfo->TaskEntryAddr=p_tcb->TaskEntryAddr;
//    tcbInfo->TaskEntryArg=p_tcb->TaskEntryArg;
//    tcbInfo->PendDataTblPtr=p_tcb->PendDataTblPtr;
//    tcbInfo->PendOn=p_tcb->PendOn;
//    tcbInfo->PendStatus=p_tcb->PendStatus;
//    tcbInfo->TaskState=p_tcb->TaskState;
//    tcbInfo->Prio=p_tcb->Prio;
//    tcbInfo->StkSize=p_tcb->StkSize;
////    tcbInfo->Opt=p_tcb->Opt;
////    tcbInfo->PendDataTblEntries=p_tcb->PendDataTblEntries;
////    tcbInfo->TS=p_tcb->TS;
////    tcbInfo->SemCtr=p_tcb->SemCtr;
////    tcbInfo->TickCtrPrev=p_tcb->TickCtrPrev;
////    tcbInfo->TickCtrMatch=p_tcb->TickCtrMatch;
////    tcbInfo->TickRemain=p_tcb->TickRemain;
////    tcbInfo->TimeQuanta=p_tcb->TimeQuanta;
////    tcbInfo->TimeQuantaCtr=p_tcb->TimeQuantaCtr;



                                                              /* --------------- ADD TASK TO READY LIST --------------- */
    OS_CRITICAL_ENTER();
    //OS_PrioInsert(p_tcb->Prio);
    //RedBlackTree T = retrieveTree(); //retrieve the red-black tree for recursion //no longer needed for new RB-tree
    //Insert(0, p_tcb, T); //we want the task to run at time 0 //no longer needed for new RB-tree
    //insert(0, p_tcb, period, taskInfo); //we want the task to run at time 0
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

    OSSched(); //old scheduler //this one probably does not make sense, as have not put into the readyqueue yet.
    //RMSched(); //new scheduler
}



#if OS_CFG_TASK_DEL_EN > 0u
void  OSTaskDelRecursive (OS_TCB  *p_tcb,
                          OS_ERR  *p_err)
{
    CPU_SR_ALLOC();



#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u
    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {              /* See if trying to delete from ISR                       */
       *p_err = OS_ERR_TASK_DEL_ISR;
        return;
    }
#endif

    if (p_tcb == &OSIdleTaskTCB) {                          /* Not allowed to delete the idle task                    */
        *p_err = OS_ERR_TASK_DEL_IDLE;
        return;
    }

#if OS_CFG_ISR_POST_DEFERRED_EN > 0u
    if (p_tcb == &OSIntQTaskTCB) {                          /* Cannot delete the ISR handler task                     */
        *p_err = OS_ERR_TASK_DEL_INVALID;
        return;
    }
#endif

    if (p_tcb == (OS_TCB *)0) {                             /* Delete 'Self'?                                         */
        CPU_CRITICAL_ENTER();
        p_tcb  = OSTCBCurPtr;                               /* Yes.                                                   */
        CPU_CRITICAL_EXIT();
    }

    OS_CRITICAL_ENTER();
    switch (p_tcb->TaskState) {
        case OS_TASK_STATE_RDY:
             //OS_RdyListRemove(p_tcb);
              //TODO: remove from skiplist
              skiplistDelete(readyQueue, (int) p_tcb->ExtPtr, p_tcb); //How to find period/key????
             break;

        case OS_TASK_STATE_SUSPENDED:
             break;

        case OS_TASK_STATE_DLY:                             /* Task is only delayed, not on any wait list             */
        case OS_TASK_STATE_DLY_SUSPENDED:
             OS_TickListRemove(p_tcb);
             break;

        case OS_TASK_STATE_PEND:
        case OS_TASK_STATE_PEND_SUSPENDED:
        case OS_TASK_STATE_PEND_TIMEOUT:
        case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:
             OS_TickListRemove(p_tcb);
             switch (p_tcb->PendOn) {                       /* See what we are pending on                             */
                 case OS_TASK_PEND_ON_NOTHING:
                 case OS_TASK_PEND_ON_TASK_Q:               /* There is no wait list for these two                    */
                 case OS_TASK_PEND_ON_TASK_SEM:
                      break;

                 case OS_TASK_PEND_ON_FLAG:                 /* Remove from wait list                                  */
                 case OS_TASK_PEND_ON_MULTI:
                 case OS_TASK_PEND_ON_MUTEX:
                 case OS_TASK_PEND_ON_Q:
                 case OS_TASK_PEND_ON_SEM:
                      OS_PendListRemove(p_tcb);
                      break;

                 default:
                      break;
             }
             break;

        default:
            OS_CRITICAL_EXIT();
            *p_err = OS_ERR_STATE_INVALID;
            return;
    }

#if OS_CFG_TASK_Q_EN > 0u
    (void)OS_MsgQFreeAll(&p_tcb->MsgQ);                     /* Free task's message queue messages                     */
#endif

    //OSTaskDelHook(p_tcb);                                   /* Call user defined hook                                 */

#if OS_CFG_DBG_EN > 0u
    OS_TaskDbgListRemove(p_tcb);
#endif
    //OSTaskQty--;                                            /* One less task being managed                            */

    //OS_TaskInitTCB(p_tcb);                                  /* Initialize the TCB to default values                   */
    //p_tcb->TaskState = (OS_STATE)OS_TASK_STATE_DEL;         /* Indicate that the task was deleted                     */
    //p_tcb->TaskState = (OS_STATE)OS_TASK_STATE_PEND;  //not sure which to choose.

    OS_CRITICAL_EXIT_NO_SCHED();
    OSSched();                                              /* Find new highest priority task                         */
    //RMSched(); //new scheduler

    *p_err = OS_ERR_NONE;
}
#endif





void tickHandlerRecursion(){
  //OSTickCtr; //the current OSTick I think.

  struct rbtNode *minNode = RBFindMin();
  int minTime = minNode->key;
  int x = OSTickCtr;
  if(minTime == OSTickCtr){
    //traverse the list of the node and make the tasks in the list ready
    //reinsert each task into the redblack tree at its new time
    //and free the redblacknode, the linked list and all the nodes of the linked list
    struct rbtNode *rem = delete(minTime); //remove rbtNode from tree (does not free memory






    traverse(minNode->tasks, releaseTask); //f should be the function that does something for each node in the list of tasks.
    dispose(minNode->tasks); //remove all elements of the list
    //TODO: remove the list itself
    //TODO: remove the rbtNode

    OS_ERR err;
    if(rem != NULL){
      OSMemPut(&CommMem2,rem,&err);
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
    /*
    minNode = RBFindMin(); //THE problems seem to occur here as we do not retrieve our the correct node.
    if(minNode == NULL){
      break;
    }
    minTime = minNode->key;
*/
  }
}

void releaseTask(node* taskNode){
  OS_TCB *p_tcb = (OS_TCB*) taskNode->data;
  TaskInfo* taskInfo = taskNode->taskInfo;
  p_tcb->TaskState = 0; //ready

  //replacing below call
  //p_tcb->StkBasePtr=taskInfo->p_stk_base;
  //p_tcb->StkPtr = p_tcb->StkBasePtr+48;
  p_tcb->StkPtr = OSTaskStkInit(p_tcb->TaskEntryAddr,
                         p_tcb->TaskEntryArg,
                         p_tcb->StkBasePtr,
                         p_tcb->StkLimitPtr,
                         p_tcb->StkSize,
                         p_tcb->Opt);

  //OS_PrioInsert(p_tcb->Prio);
  //OS_RdyListInsertTail(p_tcb); //make task ready to run

  skiplistInsert(readyQueue, taskNode->period, p_tcb, taskNode->period); //insert into our readyqueue

  CPU_INT32U newRelease = OSTickCtr+taskNode->period;
  insert(newRelease, p_tcb, taskNode->period, taskInfo); //reinsert into RB-tree
}


OS_TCB* RMSched(){
  Skiplist minNode = getMinKeyNode2(readyQueue); //get highets priority task from our ready queue
  if(minNode == NULL) {
    //no more tasks in readyqueue;
    //schedule idle task
    //OSTCBHighRdyPtr = &OSIdleTaskTCB;
    return &OSIdleTaskTCB;
  }
  else{
    //CPU_INT32U taskPrio = minNode->key; //get priority
    //node* highestPrioNode = remove_front(&minNode->tasks)
    OSTCBHighRdyPtr = minNode->tasks->data; //!!!!Problems here Hard-fault!!!! //get the TCB of the first task/element of the list of the minNode
    return OSTCBHighRdyPtr;
  }
}


void RMSchedOLD(){

    CPU_SR_ALLOC();

    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {              /* ISRs still nested?                                     */
        return;                                             /* Yes ... only schedule when no nested ISRs              */
    }

    if (OSSchedLockNestingCtr > (OS_NESTING_CTR)0) {        /* Scheduler locked?                                      */
        return;                                             /* Yes                                                    */
    }

    CPU_INT_DIS();

    OSPrioHighRdy   = OS_PrioGetHighest();                  /* Find the highest priority ready from build in ready Queue */
    OSTCBHighRdyPtr = OSRdyList[OSPrioHighRdy].HeadPtr;
    if (OSTCBHighRdyPtr == OSTCBCurPtr) {                   /* Current task is still highest priority task?           */
        CPU_INT_EN();                                       /* Yes ... no need to context switch                      */
        return;
    }

    if(OSTCBHighRdyPtr == &OSIdleTaskTCB){ //is the highest priority task the idle task
      Skiplist minNode = getMinKeyNode2(readyQueue); //get highets priority task from our ready queue
      if(minNode == NULL) {
        //no more tasks in readyqueue;
        //schedule idle task
        OSTCBHighRdyPtr = &OSIdleTaskTCB;
      }
      else{
        CPU_INT32U taskPrio = minNode->key; //get priority
        //node* highestPrioNode = remove_front(&minNode->tasks)
        OSTCBHighRdyPtr = minNode->tasks->data; //!!!!Problems here Hard-fault!!!! //get the TCB of the first task/element of the list of the minNode
        if (OSTCBHighRdyPtr == OSTCBCurPtr) {                   /* Current task is still highest priority task?           */
          CPU_INT_EN();                                       /* Yes ... no need to context switch                      */
          return;
        }
      }
    }

#if OS_CFG_TASK_PROFILE_EN > 0u
    OSTCBHighRdyPtr->CtxSwCtr++;                            /* Inc. # of context switches to this task                */
#endif
    OSTaskCtxSwCtr++;                                       /* Increment context switch counter                       */

    OS_TASK_SW();                                           /* Perform a task level context switch                    */
    CPU_INT_EN();

}
