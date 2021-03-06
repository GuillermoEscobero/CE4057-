/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2009-2010; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : FUZZI
*
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>
#include <stdio.h>
#include "driverlib/timer.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include <os.h>
#include <stdlib.h>
#include "os_extended.h"
#include "os_cfg_app.h"

/*
*********************************************************************************************************
*                                             LOCAL DEFINES
*********************************************************************************************************
*/

#define ONESECONDTICK             7000000

#define TASK1PERIOD                   10
#define TASK2PERIOD                   20


#define WORKLOAD1                     3
#define WORKLOAD2                     2


#define TIMERDIV                      (BSP_CPUClkFreq() / (CPU_INT32U)OSCfg_TickRate_Hz)




/*
*********************************************************************************************************
*                                            LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB       AppTaskStartTCB;
static  CPU_STK      AppTaskStartStk[APP_TASK_START_STK_SIZE];

static  OS_TCB       AppTaskOneTCB;
static  CPU_STK      AppTaskOneStk[APP_TASK_ONE_STK_SIZE];

static  OS_TCB       AppTaskTwoTCB;
static  CPU_STK      AppTaskTwoStk[APP_TASK_TWO_STK_SIZE];

static  OS_TCB       AppTaskThreeTCB;
static  CPU_STK      AppTaskThreeStk[APP_TASK_THREE_STK_SIZE];

static CPU_INT32U periodBlink = 5*OS_CFG_TICK_RATE_HZ;
static CPU_INT32U periodForward = 10*OS_CFG_TICK_RATE_HZ;
static CPU_INT32U periodBackward = 17*OS_CFG_TICK_RATE_HZ;
static CPU_INT32U periodLeft = 25*OS_CFG_TICK_RATE_HZ;
static CPU_INT32U periodRight = 47*OS_CFG_TICK_RATE_HZ;


static  OS_TCB       AppTaskBlinkTCB;
static  CPU_STK      AppTaskBlinkStk[APP_TASK_STD_STK_SIZE];
static  OS_TCB       AppTaskForwardTCB;
static  CPU_STK      AppTaskForwardStk[APP_TASK_STD_STK_SIZE];
static  OS_TCB       AppTaskBackwardTCB;
static  CPU_STK      AppTaskBackwardStk[APP_TASK_STD_STK_SIZE];
static  OS_TCB       AppTaskLeftTCB;
static  CPU_STK      AppTaskLeftStk[APP_TASK_STD_STK_SIZE];
static  OS_TCB       AppTaskRightTCB;
static  CPU_STK      AppTaskRightStk[APP_TASK_STD_STK_SIZE];


CPU_INT32U      iCnt = 0;
CPU_INT08U      Left_tgt;
CPU_INT08U      Right_tgt;
CPU_INT32U      iToken  = 0;
CPU_INT32U      iCounter= 1;
CPU_INT32U      iMove   = 10;
CPU_INT32U      measure=0;


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void        AppRobotMotorDriveSensorEnable    ();
        void        IntWheelSensor                    ();
        void        RoboTurn                          (tSide dir, CPU_INT16U seg, CPU_INT16U speed);

static  void        AppTaskStart                 (void  *p_arg);
static  void        AppTaskOne                   (void  *p_arg);
static  void        AppTaskTwo                   (void  *p_arg);

static  void        LEDBlink                     (void  *p_arg);
static  void        moveForward                  (void  *p_arg);
static  void        moveBackward                 (void  *p_arg);
static  void        leftTurn                     (void  *p_arg);
static  void        rightTurn                    (void  *p_arg);


/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/
OS_MEM  CommMem2;
CPU_INT32U  *p_addr[N_BLKS][BLK_SIZE];          // 16 buffers of 32 words of 32 bits

int  main (void)
{
  OS_ERR  err;
    /*void *data = malloc(10);
    if(data == NULL){
      exit(0);
    }*/
  
  int x = sizeof(char); //1 byte
  int y = sizeof(OS_TCB); //140 byte
  //int z = sizeof(TCBInfo); //60 byte
  int z = sizeof(TaskInfo);
  int v = sizeof(node); //16 bytes
  int w = sizeof(struct rbtNode); //24 bytes
  int g = sizeof(struct skiplist);
  g=g+1;
    
    BSP_IntDisAll();                                            /* Disable all interrupts.                              */
    OSInit(&err);                                               /* Init uC/OS-III.                                      */
    
    
    //OS_MEM *CommMem; //pointer to memory partition
    CPU_CHAR *p_name = "Memory Partition"; //name for memory partition
    //CPU_INT32U  *p_addr[16][32];          // 16 buffers of 32 words of 32 bits 
    //n_blks = 12; //Probably way to few blocks for containing both RB-tree and linkedList nodes.
    //blk_size = 12 * sizeof(CPU_INT32U); //Maybe to small for RB tree or linked list nodes
    OSMemCreate (&CommMem2, p_name, &p_addr[0][0], N_BLKS, BLK_SIZE*sizeof(CPU_INT32U), &err);
    
    switch(err){
      case OS_ERR_NONE:
        break;
      case OS_ERR_ILLEGAL_CREATE_RUN_TIME:
        exit(0);
        break;
      case OS_ERR_MEM_CREATE_ISR:
        exit(0);
        break;
      case OS_ERR_MEM_INVALID_BLKS:
        exit(0);
        break;
      case OS_ERR_MEM_INVALID_P_ADDR:
        exit(0);
        break;
      case OS_ERR_MEM_INVALID_SIZE:
        exit(0);
        break;
    }
    
    
    
    skiplistCreate(); //intializing skip list
    
    
    
    
    void* mem = OSMemGet(&CommMem2, &err);
    int u = sizeof(mem); //This has only size 4 bytes (size of void pointer). Not blok size.
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

    /*
    if(mem==NULL){
      exit(0);
    }
*/
    
    //RedBlackTree RBTree = Initialize();

    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,           /* Create the start task                                */
                 (CPU_CHAR   *)"App Task Start",
                 (OS_TASK_PTR ) AppTaskStart,
                 (void       *) 0,
                 (OS_PRIO     ) APP_TASK_START_PRIO,
                 (CPU_STK    *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE / 10u,
                 (CPU_STK_SIZE) APP_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) (CPU_INT32U) 0, 
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void  *p_arg)
{
    CPU_INT32U  clk_freq;
    CPU_INT32U  cnts;
    OS_ERR      err;
    (void)&p_arg;
    BSP_Init();                                                 /* Initialize BSP functions                             */
    CPU_Init();                                                 /* Initialize the uC/CPU services                       */
    clk_freq = BSP_CPUClkFreq();                                /* Determine SysTick reference freq.                    */
    cnts     = clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;        /* Determine nbr SysTick increments                     */
    OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).              */
    CPU_TS_TmrFreqSet(clk_freq);
    
    /* Enable Wheel ISR Interrupt */
    AppRobotMotorDriveSensorEnable();
    
    /* Initialise the 2 Main Tasks to  Deleted State */

    //OSTaskCreate((OS_TCB     *)&AppTaskOneTCB, (CPU_CHAR   *)"App Task One", (OS_TASK_PTR ) AppTaskOne, (void       *) 0, (OS_PRIO     ) APP_TASK_ONE_PRIO, (CPU_STK    *)&AppTaskOneStk[0], (CPU_STK_SIZE) APP_TASK_ONE_STK_SIZE / 10u, (CPU_STK_SIZE) APP_TASK_ONE_STK_SIZE, (OS_MSG_QTY  ) 0u, (OS_TICK     ) 0u, (void       *)(CPU_INT32U) 1, (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), (OS_ERR     *)&err);
    //OSTaskCreate((OS_TCB     *)&AppTaskTwoTCB, (CPU_CHAR   *)"App Task Two", (OS_TASK_PTR ) AppTaskTwo, (void       *) 0, (OS_PRIO     ) APP_TASK_TWO_PRIO, (CPU_STK    *)&AppTaskTwoStk[0], (CPU_STK_SIZE) APP_TASK_TWO_STK_SIZE / 10u, (CPU_STK_SIZE) APP_TASK_TWO_STK_SIZE, (OS_MSG_QTY  ) 0u, (OS_TICK     ) 0u, (void       *) (CPU_INT32U) 2, (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), (OS_ERR     *)&err);
    
    
    //CPU_INT32U period = 5*ONESECONDTICK;
    //CPU_INT32U period = 5*OS_CFG_TICK_RATE_HZ;
    //OSTaskCreateRecursive((OS_TCB     *)&AppTaskThreeTCB, (CPU_CHAR   *)"App Task three", (OS_TASK_PTR ) AppTaskTwo, (void       *) 0, (OS_PRIO     ) APP_TASK_THREE_PRIO, (CPU_STK    *)&AppTaskThreeStk[0], (CPU_STK_SIZE) APP_TASK_THREE_STK_SIZE / 10u, (CPU_STK_SIZE) APP_TASK_THREE_STK_SIZE, (OS_MSG_QTY  ) 0u, (OS_TICK     ) 0u, (void       *) (CPU_INT32U) period, (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), (OS_ERR     *)&err);
    
    
//    CPU_INT32U APP_TASK_BLINK_PRIO  = periodBlink;
//    CPU_INT32U APP_TASK_FORWARD_PRIO = periodForward;
//    CPU_INT32U APP_TASK_BACKWARD_PRIO = periodBackward;
//    CPU_INT32U APP_TASK_LEFT_PRIO = periodLeft;
//    CPU_INT32U APP_TASK_RIGHT_PRIO = periodRight;

    OSTaskCreateRecursive((OS_TCB     *)&AppTaskBlinkTCB, (CPU_CHAR   *)"App Task blink", (OS_TASK_PTR ) LEDBlink, (void       *) 0, (OS_PRIO     ) APP_TASK_BLINK_PRIO, (CPU_STK    *)&AppTaskBlinkStk[0], (CPU_STK_SIZE) APP_TASK_STD_STK_SIZE / 10u, (CPU_STK_SIZE) APP_TASK_STD_STK_SIZE, (OS_MSG_QTY  ) 0u, (OS_TICK     ) 0u, (void       *) (CPU_INT32U) periodBlink, (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), (OS_ERR     *)&err);
    OSTaskCreateRecursive((OS_TCB     *)&AppTaskForwardTCB, (CPU_CHAR   *)"App Task forward", (OS_TASK_PTR ) moveForward, (void       *) 0, (OS_PRIO     ) APP_TASK_FORWARD_PRIO, (CPU_STK    *)&AppTaskForwardStk[0], (CPU_STK_SIZE) APP_TASK_STD_STK_SIZE / 10u, (CPU_STK_SIZE) APP_TASK_STD_STK_SIZE, (OS_MSG_QTY  ) 0u, (OS_TICK     ) 0u, (void       *) (CPU_INT32U) periodForward, (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), (OS_ERR     *)&err);
    OSTaskCreateRecursive((OS_TCB     *)&AppTaskBackwardTCB, (CPU_CHAR   *)"App Task backward", (OS_TASK_PTR ) moveBackward, (void       *) 0, (OS_PRIO     ) APP_TASK_BACKWARD_PRIO, (CPU_STK    *)&AppTaskBackwardStk[0], (CPU_STK_SIZE) APP_TASK_STD_STK_SIZE / 10u, (CPU_STK_SIZE) APP_TASK_STD_STK_SIZE, (OS_MSG_QTY  ) 0u, (OS_TICK     ) 0u, (void       *) (CPU_INT32U) periodBackward, (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), (OS_ERR     *)&err);
    OSTaskCreateRecursive((OS_TCB     *)&AppTaskLeftTCB, (CPU_CHAR   *)"App Task left", (OS_TASK_PTR ) leftTurn, (void       *) 0, (OS_PRIO     ) APP_TASK_LEFT_PRIO, (CPU_STK    *)&AppTaskLeftStk[0], (CPU_STK_SIZE) APP_TASK_STD_STK_SIZE / 10u, (CPU_STK_SIZE) APP_TASK_STD_STK_SIZE, (OS_MSG_QTY  ) 0u, (OS_TICK     ) 0u, (void       *) (CPU_INT32U) periodLeft, (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), (OS_ERR     *)&err);
    OSTaskCreateRecursive((OS_TCB     *)&AppTaskRightTCB, (CPU_CHAR   *)"App Task right", (OS_TASK_PTR ) rightTurn, (void       *) 0, (OS_PRIO     ) APP_TASK_RIGHT_PRIO, (CPU_STK    *)&AppTaskRightStk[0], (CPU_STK_SIZE) APP_TASK_STD_STK_SIZE / 10u, (CPU_STK_SIZE) APP_TASK_STD_STK_SIZE, (OS_MSG_QTY  ) 0u, (OS_TICK     ) 0u, (void       *) (CPU_INT32U) periodRight, (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR), (OS_ERR     *)&err);
    
    
    
    
    
    /* Delete this task */
    OSTaskDel((OS_TCB *)0, &err);
    
}

static  void  AppTaskOne (void  *p_arg)
{ 
    OS_ERR      err;
    CPU_INT32U  k, i, j, l;
    i=0;
    for(l=0; l<8; l++){
      if(iMove > 0)
      {
        if(iMove%2==0)
        {  
        RoboTurn(FRONT, 16, 50);
        iMove--;
        }
        else{
          RoboTurn(RIGHT_SIDE, 16, 50);
          //RoboTurn(BACK, 16, 50);
          iMove++;
        }
      }
      
      for(i=0; i <7300000; i++){
        j=2*i;
      }

    }
    /*
    for(k=0; k<WORKLOAD1; k++)
    {
      for(i=0; i <ONESECONDTICK; i++){
        j=2*i;
      }
     }
    */
    RoboTurn(-1, 16, 50);
    OSTaskDel((OS_TCB *)0, &err);   

}

static void LEDBlink(void  *p_arg){
  OS_ERR err;
  CPU_INT32U  i,j=0;
  BSP_LED_Off(0u);
  BSP_LED_On(0u);
      for(i=0; i <ONESECONDTICK/2; i++)
         j = ((i * 2)+j);
  BSP_LED_Off(0u);
  
  OSTaskDelRecursive((OS_TCB *)0, &err);
}


static void moveForward(void  *p_arg){
  OS_ERR err;
  RoboTurn(FRONT, 8, 50);
  OSTaskDelRecursive((OS_TCB *)0, &err);
}

static void moveBackward(void  *p_arg){
  OS_ERR err;
  RoboTurn(BACK, 8, 50);
  OSTaskDelRecursive((OS_TCB *)0, &err);
}

static void leftTurn(void  *p_arg){
  OS_ERR err;
  RoboTurn(LEFT_SIDE, 9, 50);
  OSTaskDelRecursive((OS_TCB *)0, &err);
}

static void rightTurn(void  *p_arg){
  OS_ERR err;
  RoboTurn(RIGHT_SIDE, 9, 50);
  OSTaskDelRecursive((OS_TCB *)0, &err);
}


static  void  AppTaskTwo (void  *p_arg)
{   
    OS_ERR      err;
    CPU_INT32U  i,k,j=0;
   
    for(i=0; i <(ONESECONDTICK); i++)
    {
      j = ((i * 2) + j);
    }
    //0+2+4+6+8+10
    
    //BSP_LED_Off(0u);
    BSP_LED_Off(1u);
    BSP_LED_On(2u);
    for(k=0; k<5; k++)
    {
      BSP_LED_Toggle(0u);
      for(i=0; i <ONESECONDTICK/2; i++)
         j = ((i * 2)+j);
    }
    
    BSP_LED_Off(0u);
   //OSTaskDel((OS_TCB *)0, &err);
    OSTaskDelRecursive((OS_TCB *)0, &err);

}

static  void  AppRobotMotorDriveSensorEnable ()
{
    BSP_WheelSensorEnable();
    BSP_WheelSensorIntEnable(RIGHT_SIDE, SENSOR_A, (CPU_FNCT_VOID)IntWheelSensor);
    BSP_WheelSensorIntEnable(LEFT_SIDE, SENSOR_A, (CPU_FNCT_VOID)IntWheelSensor);
}


void IntWheelSensor()
{
	CPU_INT32U         ulStatusR_A;
	CPU_INT32U         ulStatusL_A;

	static CPU_INT08U CountL = 0;
	static CPU_INT08U CountR = 0;

	static CPU_INT08U data = 0;

	ulStatusR_A = GPIOPinIntStatus(RIGHT_IR_SENSOR_A_PORT, DEF_TRUE);
	ulStatusL_A = GPIOPinIntStatus(LEFT_IR_SENSOR_A_PORT, DEF_TRUE);

        if (ulStatusR_A & RIGHT_IR_SENSOR_A_PIN)
        {
          GPIOPinIntClear(RIGHT_IR_SENSOR_A_PORT, RIGHT_IR_SENSOR_A_PIN);           /* Clear interrupt.*/
          CountR = CountR + 1;
        }

        if (ulStatusL_A & LEFT_IR_SENSOR_A_PIN)
        {
          GPIOPinIntClear(LEFT_IR_SENSOR_A_PORT, LEFT_IR_SENSOR_A_PIN);
          CountL = CountL + 1;
        }

	if((CountL >= Left_tgt) && (CountR >= Right_tgt))
        {
          data = 0x11;
          Left_tgt = 0;
          Right_tgt = 0;
          CountL = 0;
          CountR = 0;
          BSP_MotorStop(LEFT_SIDE);
          BSP_MotorStop(RIGHT_SIDE);
        }
        else if(CountL >= Left_tgt)
        {
          data = 0x10;
          Left_tgt = 0;
          CountL = 0;
          BSP_MotorStop(LEFT_SIDE);
        }
        else if(CountR >= Right_tgt)
        {
          data = 0x01;
          Right_tgt = 0;
          CountR = 0;
          BSP_MotorStop(RIGHT_SIDE);
        }
        return;
}

void RoboTurn(tSide dir, CPU_INT16U seg, CPU_INT16U speed)
{
	Left_tgt = seg;
        Right_tgt = seg;

	BSP_MotorStop(LEFT_SIDE);
	BSP_MotorStop(RIGHT_SIDE);

        BSP_MotorSpeed(LEFT_SIDE, speed <<8u);
	BSP_MotorSpeed(RIGHT_SIDE,speed <<8u);

	switch(dir)
	{
            case FRONT :
                    BSP_MotorDir(RIGHT_SIDE,FORWARD);
                    BSP_MotorDir(LEFT_SIDE,FORWARD);
                    BSP_MotorRun(LEFT_SIDE);
                    BSP_MotorRun(RIGHT_SIDE);
                    break;
                    
            case BACK :
                    BSP_MotorDir(LEFT_SIDE,REVERSE);
                    BSP_MotorDir(RIGHT_SIDE,REVERSE);
                    BSP_MotorRun(RIGHT_SIDE);
                    BSP_MotorRun(LEFT_SIDE);
                    break;
                    
            case LEFT_SIDE :
                    BSP_MotorDir(RIGHT_SIDE,FORWARD);
                    BSP_MotorDir(LEFT_SIDE,REVERSE);
                    BSP_MotorRun(LEFT_SIDE);
                    BSP_MotorRun(RIGHT_SIDE);
                    break;
                    
            case RIGHT_SIDE:
                    BSP_MotorDir(LEFT_SIDE,FORWARD);
                    BSP_MotorDir(RIGHT_SIDE,REVERSE);
                    BSP_MotorRun(RIGHT_SIDE);
                    BSP_MotorRun(LEFT_SIDE);
                    break;
                    
            default:
                    BSP_MotorStop(LEFT_SIDE);
                    BSP_MotorStop(RIGHT_SIDE);
                    break;
	}

	return;
}
