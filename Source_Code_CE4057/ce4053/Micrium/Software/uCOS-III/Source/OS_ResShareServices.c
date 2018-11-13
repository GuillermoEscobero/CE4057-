//this file should contain the functions and datastructures for resource sharing
//Can we just use the mutex data-structure already present in micrium?
//I think we can do a lot of copy-paste frome os_mutex.c, but we have to adapt to our readylist and scheduler
#include "os_extended.h"
#include <os.h>

//void osMuRequest(OS_MUTEX* mutex, OS_ERR* err){ //maybe some more arguments are needed
void  osMuRequest (EXT_MUTEX   *p_mutex,
                   OS_TICK     timeout,
                   OS_OPT      opt,
                   CPU_TS     *p_ts,
                   OS_ERR     *p_err)
{

   //implement function that makes calling task request a resource
  //if the mutex is already use, block this task, and put it into a wait-list for this mutex
  //remember to sort the list by priority (and maybe FIFO for same priority-tasks)
  //We return from here only when we have the mutex (or if some errors occurred that waked us up, like deleting the mutex)
  //If we have to change the priority of the owner, then we would probably have to remove and reinsert the task in the readylist (also for release function)
  //After updating the readylist, call OSSched()
  
  
  
  
  
    OS_PEND_DATA  pend_data;
    OS_TCB       *p_tcb;
    CPU_SR_ALLOC();



#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u
    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {              /* Not allowed to call from an ISR                        */
       *p_err = OS_ERR_PEND_ISR;
        return;
    }
#endif

#if OS_CFG_ARG_CHK_EN > 0u
    if (p_mutex == (EXT_MUTEX *)0) {                         /* Validate arguments                                     */
        *p_err = OS_ERR_OBJ_PTR_NULL;
        return;
    }
    switch (opt) {
        case OS_OPT_PEND_BLOCKING:
        case OS_OPT_PEND_NON_BLOCKING:
             break;

        default:
             *p_err = OS_ERR_OPT_INVALID;
             return;
    }
#endif

#if OS_CFG_OBJ_TYPE_CHK_EN > 0u
    if (p_mutex->Type != OS_OBJ_TYPE_MUTEX) {               /* Make sure mutex was created                            */
        *p_err = OS_ERR_OBJ_TYPE;
        return;
    }
#endif

    if (p_ts != (CPU_TS *)0) {
       *p_ts  = (CPU_TS  )0;                                /* Initialize the returned timestamp                      */
    }

    CPU_CRITICAL_ENTER();
    
    
    if (OSTCBCurPtr == p_mutex->OwnerTCBPtr) {              /* See if current task is already the owner of the mutex  */
        p_mutex->OwnerNestingCtr++;
        if (p_ts != (CPU_TS *)0) {
           *p_ts  = p_mutex->TS;
        }
        CPU_CRITICAL_EXIT();
        *p_err = OS_ERR_MUTEX_OWNER;                        /* Indicate that current task already owns the mutex      */
        return;
    }
    
    OS_TCB* not_used;
    int systemCeiling = peek(ceilingStack, &not_used);
    node* TCBListNode = (search(resUseTask, OSTCBCurPtr));
    //Can we take the mutex (slide 15 PCP)
    if(OSTCBCurPtr->Prio < systemCeiling || TCBListNode!=NULL){
        /* Yes, caller may proceed                                */
      //Does task already own mutex
        if(TCBListNode==NULL){
          //No: tell that the tcb owns a mutex
          //the period is used to tell the number of mutexes the task has. No task info needed.
          resUseTask = prepend(resUseTask,OSTCBCurPtr, 1, NULL);
        }
        else{
          //Yes: increment the number of mutexes owned
          TCBListNode->period++;
        }
        p_mutex->OwnerTCBPtr       =  OSTCBCurPtr; //Save the owning task
        p_mutex->OwnerOriginalPrio =  OSTCBCurPtr->Prio; //save the owners original priority
        p_mutex->OwnerNestingCtr   = (OS_NESTING_CTR)1; //the number of times the owner was granted this mutex
        //Push the resource ceiling and the task causing it onto the system ceiling stack
        ceilingStack = push(ceilingStack , p_mutex->resourceCeiling, OSTCBCurPtr);
        if (p_ts != (CPU_TS *)0) {
           *p_ts                   = p_mutex->TS;
        }
        CPU_CRITICAL_EXIT();
        *p_err                     =  OS_ERR_NONE;
        return;
    }

    if ((opt & OS_OPT_PEND_NON_BLOCKING) != (OS_OPT)0) {    /* Caller wants to block if not available?                */
        CPU_CRITICAL_EXIT();
        *p_err = OS_ERR_PEND_WOULD_BLOCK;                   /* No                                                     */
        return;
    } else {
        if (OSSchedLockNestingCtr > (OS_NESTING_CTR)0) {    /* Can't pend when the scheduler is locked                */
            CPU_CRITICAL_EXIT();
            *p_err = OS_ERR_SCHED_LOCKED;
            return;
        }
    }

    //We did not succeed in getting the mutex
    OS_CRITICAL_ENTER_CPU_CRITICAL_EXIT();                  /* Lock the scheduler/re-enable interrupts                */
    peek(ceilingStack, &p_tcb); //Get the TCB/mutex owner that we wait for. We don't need the return value
    if (p_tcb->Prio > OSTCBCurPtr->Prio) {                  /* See if mutex owner has a lower priority than current   */
        switch (p_tcb->TaskState) {
            case OS_TASK_STATE_RDY:
                  //Priority inheritance and update readylist accordingly
                 skiplistDelete(readyQueue, (int) p_tcb->Prio, p_tcb); //Beaware of cast
                 p_tcb->Prio = OSTCBCurPtr->Prio;           /* Raise owner's priority                                 */
                 skiplistInsert(readyQueue, (int) p_tcb->Prio, p_tcb, (CPU_INT32U) p_tcb->ExtPtr); //readyQueue, priority, tcb, period
                 break;

            case OS_TASK_STATE_DLY:
            case OS_TASK_STATE_DLY_SUSPENDED:
            case OS_TASK_STATE_SUSPENDED:
                 p_tcb->Prio = OSTCBCurPtr->Prio;           /* Only need to raise the owner's priority                */
                 break;

            case OS_TASK_STATE_PEND:                        /* Change the position of the task in the wait list       */
            case OS_TASK_STATE_PEND_TIMEOUT:     /*In these cases the current task wants a mutex from another task, which is itself waiting for a second mutex*/
            case OS_TASK_STATE_PEND_SUSPENDED:
            case OS_TASK_STATE_PEND_TIMEOUT_SUSPENDED:
//                 OS_PendListChangePrio(p_tcb,
//                                       OSTCBCurPtr->Prio);
                //The following tries to cover the function call above
              //Priority inhertance and update waitQueue accordingly
                 EXT_MUTEX* mutex2; //the mutex p_tcb is waiting for
                 OS_TCB* p_tcb2 = avlDeleteNode(&waitQueue, p_tcb->Prio, p_tcb, &mutex2); //last argument is for getting the mutex
                 if(p_tcb != p_tcb2){
                   //If the found tcb is not the same as as the given one, then we have a big problem
                   exit(0);
                 }
                 p_tcb->Prio = OSTCBCurPtr->Prio;
                 waitQueue = avlInsert(waitQueue, p_tcb->Prio, p_tcb, mutex2);
                 break;

            default:
                 OS_CRITICAL_EXIT();
                 *p_err = OS_ERR_STATE_INVALID;
                 return;
        }
    }

//    OS_Pend(&pend_data,                                     /* Block task pending on Mutex                            */
//            (OS_PEND_OBJ *)((void *)p_mutex),
//             OS_TASK_PEND_ON_MUTEX,
//             timeout);
    //The calls below should cover the one above
    
    OSTCBCurPtr->PendOn = OS_TASK_PEND_ON_MUTEX;
    OSTCBCurPtr->PendStatus = OS_STATUS_PEND_OK;
    
    //Insert in the waitQueue and remove from the readylist as we have to wait
    waitQueue = avlInsert(waitQueue, OSTCBCurPtr->Prio, OSTCBCurPtr, p_mutex); //Insert the task into the wait list
    OSTCBCurPtr->TaskState = OS_TASK_STATE_PEND; //change the state to OS_TASK_STATE_PEND (_timout)
    skiplistDelete(readyQueue, OSTCBCurPtr->Prio, OSTCBCurPtr); //Remove the task from the readyqueue
    
    OS_CRITICAL_EXIT_NO_SCHED();

    OSSched();                                              /* Find the next highest priority task ready to run       */

    CPU_CRITICAL_ENTER();
    //Maybe we should remove this
    switch (OSTCBCurPtr->PendStatus) {
        case OS_STATUS_PEND_OK:                             /* We got the mutex                                       */
             if (p_ts != (CPU_TS *)0) {
                *p_ts  = OSTCBCurPtr->TS;
             }
             *p_err = OS_ERR_NONE;
             break;

        case OS_STATUS_PEND_ABORT:                          /* Indicate that we aborted                               */
             if (p_ts != (CPU_TS *)0) {
                *p_ts  = OSTCBCurPtr->TS;
             }
             *p_err = OS_ERR_PEND_ABORT;
             break;

        case OS_STATUS_PEND_TIMEOUT:                        /* Indicate that we didn't get mutex within timeout       */
             if (p_ts != (CPU_TS *)0) {
                *p_ts  = (CPU_TS  )0;
             }
             *p_err = OS_ERR_TIMEOUT;
             break;

        case OS_STATUS_PEND_DEL:                            /* Indicate that object pended on has been deleted        */
             if (p_ts != (CPU_TS *)0) {
                *p_ts  = OSTCBCurPtr->TS;
             }
             *p_err = OS_ERR_OBJ_DEL;
             break;

        default:
             *p_err = OS_ERR_STATUS_INVALID;
             break;
    }
    CPU_CRITICAL_EXIT();
  
}

void osMuRelease(EXT_MUTEX  *p_mutex,
                   OS_OPT     opt,
                   OS_ERR    *p_err){
  //implement function that makes calling task release a resource
  //This function should un-block a function waiting for this resource
  
  
  //Copied from OSMutexPost
    OS_PEND_LIST  *p_pend_list;
    OS_TCB        *p_tcb;
    CPU_TS         ts;
    CPU_SR_ALLOC();



#ifdef OS_SAFETY_CRITICAL
    if (p_err == (OS_ERR *)0) {
        OS_SAFETY_CRITICAL_EXCEPTION();
        return;
    }
#endif

#if OS_CFG_CALLED_FROM_ISR_CHK_EN > 0u
    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {              /* Not allowed to call from an ISR                        */
       *p_err = OS_ERR_POST_ISR;
        return;
    }
#endif

#if OS_CFG_ARG_CHK_EN > 0u
    if (p_mutex == (EXT_MUTEX *)0) {                         /* Validate 'p_mutex'                                     */
        *p_err = OS_ERR_OBJ_PTR_NULL;
        return;
    }
#endif

#if OS_CFG_OBJ_TYPE_CHK_EN > 0u
    if (p_mutex->Type != OS_OBJ_TYPE_MUTEX) {               /* Make sure mutex was created                            */
        *p_err = OS_ERR_OBJ_TYPE;
        return;
    }
#endif

    CPU_CRITICAL_ENTER();
    if (OSTCBCurPtr != p_mutex->OwnerTCBPtr) {              /* Make sure the mutex owner is releasing the mutex       */
        CPU_CRITICAL_EXIT();
        *p_err = OS_ERR_MUTEX_NOT_OWNER;
        return;
    }

    OS_CRITICAL_ENTER_CPU_CRITICAL_EXIT();
    ts          = OS_TS_GET();                              /* Get timestamp                                          */
    p_mutex->TS = ts;
    p_mutex->OwnerNestingCtr--;                             /* Decrement owner's nesting counter                      */
    if (p_mutex->OwnerNestingCtr > (OS_NESTING_CTR)0) {     /* Are we done with all nestings?                         */
        OS_CRITICAL_EXIT();                                 /* No                                                     */
        *p_err = OS_ERR_MUTEX_NESTING;
        return;
    }
    
    //Update that this task has one less mutex
    node* taskNode = search(resUseTask,OSTCBCurPtr); //check that the task actually owns a mutex
    if(taskNode == NULL){
      exit(0); //A task not owning a mutex tried to release a mutex!!
    }
    if(taskNode->period>1){
      //if more than one mutex owned: decrement
      taskNode->period--;
    }
    else{
      //only one mutex owned: remove from list
      node* removeNode = remove_any(&resUseTask, OSTCBCurPtr);
      if(removeNode->data != OSTCBCurPtr){
        exit(0); //We somehow managed to get the wrong node/tcb
      }
      OS_ERR err;
      OSMemPut(&CommMem2,removeNode,&err); //should we check if temp is NULL?
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
    
    //TODO: Can we change the system ceiling - I think it is done
    //update systemceiling and undo priority inheritance
    int oldCeiling;
    OS_TCB* not_used;
    ceilingStack = pop(ceilingStack, &oldCeiling, &not_used); //Update system ceiling
    int newCeiling = peek(ceilingStack, &not_used); //read the new ceiling
    p_mutex->OwnerTCBPtr     = (OS_TCB       *)0; //no owner
    if (OSTCBCurPtr->Prio != p_mutex->OwnerOriginalPrio) { //priority inheritance done?
      //undo priority inheritance and update readyQueue accordingly
      skiplistDelete(readyQueue, (int) OSTCBCurPtr->Prio, OSTCBCurPtr); //Beaware of cast
      OSTCBCurPtr->Prio = p_mutex->OwnerOriginalPrio;     /* Lower owner's priority back to its original one        */
      skiplistInsert(readyQueue, (int) OSTCBCurPtr->Prio, OSTCBCurPtr, (CPU_INT32U) OSTCBCurPtr->ExtPtr); //readyQueue, period, tcb, period
      
      OSPrioCur         = OSTCBCurPtr->Prio; //TODO: What is this used for?????
    }
    
    //check if we may start waiting task
    avlnode* minNode = minValueNode(waitQueue);
    while((minNode !=NULL) && (minNode->key<newCeiling)){ //can we remove any task from the waitqueue
      EXT_MUTEX* mutex2; //The mutex the task is waiting for is found in the waitQueue/AVL tree together with the task
      OS_TCB* p_tcb = avlDeleteNode(&waitQueue, minNode->key, NULL, &mutex2); //We don't care which task we get with this priority
      mutex2->OwnerTCBPtr       =  p_tcb; //Save the owning task
      mutex2->OwnerOriginalPrio =  p_tcb->Prio; //save the owners original priority
      mutex2->OwnerNestingCtr   = (OS_NESTING_CTR)1; //the number of times the owner was granted this mutex
      ceilingStack = push(ceilingStack , mutex2->resourceCeiling, p_tcb); //update system ceiling
      //Tell the system that the p_tcb owns one (more) mutex
      node* TCBListNode = (search(resUseTask, p_tcb));
      if(TCBListNode==NULL){ //tell that the tcb owns a mutex.
        resUseTask = prepend(resUseTask,p_tcb, 1, NULL); //the period is used to tell the number of mutexes the task has. No task info needed.
      }
      else{
        //or increment number of owned mutexes
        TCBListNode->period++;
      }
      
      //Refer to OS_POST call
      //make task ready
      if(p_tcb->TaskState == OS_TASK_STATE_PEND){
        //OS_TaskRdy(p_tcb); removes from tickList and adds to readylist if suspended  /* Make task ready to run                            */
        skiplistInsert(readyQueue, (int) p_tcb->Prio, p_tcb, (CPU_INT32U) p_tcb->ExtPtr); //readyQueue, period, tcb, period
        p_tcb->TaskState  = OS_TASK_STATE_RDY;
        p_tcb->PendStatus = OS_STATUS_PEND_OK;              /* Clear pend status                                 */
        p_tcb->PendOn     = OS_TASK_PEND_ON_NOTHING;        /* Indicate no longer pending                        */
      }
      else{
        //????????????????????????
      }
      minNode = minValueNode(waitQueue); //for next iteration
    }

    OS_CRITICAL_EXIT_NO_SCHED();

    if ((opt & OS_OPT_POST_NO_SCHED) == (OS_OPT)0) {
        OSSched();                                          /* Run the scheduler                                      */
    }

    *p_err = OS_ERR_NONE;
}

void osMuCreate(EXT_MUTEX    *p_mutex,
                     CPU_CHAR    *p_name,
                     CPU_INT32U resourceCeiling,
                     OS_ERR      *p_err)
{
  //instantiate and return a new and initialized mutex
  //I think we may use the data structure OS_Mutex build into micrium
  
  
  
  //Copied from OSMutexCreate
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
    if (OSIntNestingCtr > (OS_NESTING_CTR)0) {              /* Not allowed to be called from an ISR                   */
        *p_err = OS_ERR_CREATE_ISR;
        return;
    }
#endif

#if OS_CFG_ARG_CHK_EN > 0u
    if (p_mutex == (EXT_MUTEX *)0) {                         /* Validate 'p_mutex'                                     */
        *p_err = OS_ERR_OBJ_PTR_NULL;
        return;
    }
#endif

    CPU_CRITICAL_ENTER();
    p_mutex->Type              =  OS_OBJ_TYPE_MUTEX;        /* Mark the data structure as a mutex                     */
    p_mutex->NamePtr           =  p_name;
    p_mutex->OwnerTCBPtr       = (OS_TCB       *)0;
    p_mutex->OwnerNestingCtr   = (OS_NESTING_CTR)0;         /* Mutex is available                                     */
    p_mutex->TS                = (CPU_TS        )0;
    p_mutex->OwnerOriginalPrio =  OS_CFG_PRIO_MAX;
    p_mutex->resourceCeiling = resourceCeiling; //added
    OS_PendListInit(&p_mutex->PendList);                    /* Initialize the waiting list                            */

#if OS_CFG_DBG_EN > 0u
    //OS_MutexDbgListAdd(p_mutex); //incombatibility problem between OS_MUTEX and EXT_MUTEX
#endif
    OSMutexQty++;

    CPU_CRITICAL_EXIT();
    *p_err = OS_ERR_NONE;
}

//void osMuDel(EXT_MUTEX* mutex){
//  //TODO: delete and free the given mutex
//}
//
//void osBlock(){
//  //TODO: Utility function to block a task, if requested resource is taken
//  //refer to rules for PCP protocol of slides for blocking rules.
//  //Maybe we can use the micrium build ind OS_Pend function instead (Or maybe not as i think we need our own pend/wait list implementation)
//}
//
//void osWaitListCreate(){
//  //TODO: Create and return and empty waitlist to be used when creating a mutex for mutex->PendList
//  //Utility function
//}