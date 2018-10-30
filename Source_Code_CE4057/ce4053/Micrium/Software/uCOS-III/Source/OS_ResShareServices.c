//this file should contain the functions and datastructures for resource sharing
//Can we just use the mutex data-structure already present in micrium?
//I think we can do a lot of copy-paste frome os_mutex.c, but we have to adapt to our readylist and scheduler
#include "os_extended.h"
#include <os.h>

void osMuRequest(OS_MUTEX* mutex, OS_ERR* err){ //maybe some more arguments are needed
   //TODO: implement function that makes calling task request a resource
  //if the mutex is already use, block this task, and put it into a wait-list for this mutex
  //remember to sort the list by priority (and maybe FIFO for same priority-tasks
  //We return from here only when we have the mutex (or if some errors occurred that waked us up, like deleting the mutex)
  //If we have to change the priority of the owner, then we would probably have to remove and reinsert the task in the readylist (also for release function)
  //After updating the readylist, call OSSched()
}

void osMuRelease(OS_MUTEX* mutex, OS_ERR* err){
  //TODO: implement function that makes calling task release a resource
  //This function should un-block a function waiting for this resource
}

OS_MUTEX osMuCreate(OS_MUTEX* mutex, char* name, OS_ERR* err){
  //TODO: instantiate and return a new and initialized mutex
  //I think we may use the data structure OS_Mutex build into micrium
}

void osMuDel(OS_MUTEX* mutex){
  //TODO: delete and free the given mutex
}

void osBlock(){
  //TODO: Utility function to block a task, if requested resource is taken
  //refer to rules for PCP protocol of slides for blocking rules.
  //Maybe we can use the micrium build ind OS_Pend function instead (Or maybe not as i think we need our own pend/wait list implementation)
}

void osWaitListCreate(){
  //TODO: Create and return and empty waitlist to be used when creating a mutex for mutex->PendList
  //Utility function
}