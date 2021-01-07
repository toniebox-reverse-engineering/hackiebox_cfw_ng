//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/
//
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions
//  are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

#include "string.h"
#include "task.h"
#include "systick.h"

//****************************************************************************
//
//! \addtogroup ota_update_nonos
//! @{
//
//****************************************************************************

//****************************************************************************
// Global Static variables
//****************************************************************************
static tTask g_TaskList[MAX_NOF_TASK];
static tTask *g_psCurrentTask;

//****************************************************************************
//
//! \internal
//!
//! \brief The Function handles the systick event for time keeping
//!
//! \return None.
//
//****************************************************************************
static void SysTickIntHandler()
{
    int iNdx;

    for(iNdx = 0; iNdx < MAX_NOF_TASK; iNdx++ )
    {
        if(g_TaskList[iNdx].ulSleepCount > 0)
        {
          g_TaskList[iNdx].ulSleepCount--;
        }
    }
}

//****************************************************************************
//
//! \brief The Function creates a new task
//!
//! \param pfnTaskFunc - Pointer to task function
//! \param pvParams	   - Parameter to be passed to task function
//!
//! \return Returns 0 on success, -1 otherwise.
//
//****************************************************************************
int TaskCreate(TaskFunc pfnTaskFunc, void *pvParams)
{
  static unsigned long ulNxtTask = 0;

  //
  // Initialize the task list if this is the first task created
  //
  if( ulNxtTask == 0 )
  {
    memset((char *)g_TaskList,0,sizeof(g_TaskList));
  }

  //
  // Create the task
  //
  if(ulNxtTask < MAX_NOF_TASK)
  {
    g_TaskList[ulNxtTask].pfnTaskFunc  = pfnTaskFunc;
    g_TaskList[ulNxtTask].cTaskState   = TASK_STATE_RUN;
    g_TaskList[ulNxtTask].pvParams     = pvParams;
    g_TaskList[ulNxtTask].ulSleepCount = 0;
    ulNxtTask++;

	//
	// Return success
	//
	return TASK_OK;
  }

  //
  // Return error
  //
  return TASK_ERROR;
}

//****************************************************************************
//
//! \brief The Function initializes a sync object
//!
//! \param psSyncObj - Pointer to sync object to be initialized
//!
//! \return Returns 0 on success, -1 otherwise.
//
//****************************************************************************
int TaskSyncObjCreate(tSyncObj *psSyncObj)
{

  //
  // Initialize the sync object if it is not null
  //
  if(psSyncObj != NULL )
  {
    psSyncObj->psTask = NULL;

	//
	// Return success
	//
    return TASK_OK;
  }

  //
  // Return error
  //
  return TASK_ERROR;
}

//****************************************************************************
//
//! \brief The Function sets the calling task in wait state
//!
//! \param psSyncObj - Pointer to sync object
//!
//! \return Returns 0 on success, -1 otherwise.
//
//****************************************************************************
int TaskSyncObjWait(tSyncObj *psSyncObj)
{
  //
  // Set the invoking task in wait state.
  // Only on task can use a sync object at a time
  //
  if(psSyncObj->psTask == NULL )
  {
    g_psCurrentTask->cTaskState = TASK_STATE_WAIT;
    psSyncObj->psTask = g_psCurrentTask;

	//
	// Return success
	//
    return TASK_OK;
  }

  //
  // Return error
  //
  return TASK_ERROR;
}

//****************************************************************************
//
//! \brief The Function signals a task waiting on sync object
//!
//! \param psSyncObj - Pointer to sync object
//!
//! \return None.
//
//****************************************************************************
void TaskSyncObjSignal(tSyncObj *psSyncObj)
{
  //
  // Release the waiting task from wait state
  //
  if(psSyncObj->psTask != NULL)
  {
    psSyncObj->psTask->cTaskState = TASK_STATE_RUN;
    psSyncObj->psTask = NULL;
  }
}

//****************************************************************************
//
//! \brief The Function sets the sleep count
//!
//! \param ulSleepCount - Sleep count value
//!
//! \return None.
//
//****************************************************************************
void TaskSleep(unsigned long ulSleepCount)
{
  //
  // Set the invoking task's sleep count
  //
  g_psCurrentTask->ulSleepCount = ulSleepCount;
}


//****************************************************************************
//
//! \brief Scheduler function, loop through every active task
//!
//!
//! \return None.
//
//****************************************************************************
int TaskMainLoopStart()
{
  int iNdx;
  int iRet;

  //
  // Set the Sys-Tick handler
  //
  SysTickIntRegister(SysTickIntHandler);

  //
  // Set the tick period
  //
  SysTickPeriodSet(80000);

  //
  // Enable the timer
  //
  SysTickEnable();

  //
  // Infinite loop
  //
  while(1)
  {
    //
    // Loop through every task
    //
    for(iNdx = 0; iNdx <MAX_NOF_TASK; iNdx++ )
    {
      //
      // Set the current task pointer
      //
      g_psCurrentTask = &g_TaskList[iNdx];

      //
      // Check if the task is active
      //
      if( TASK_STATE_RUN == g_psCurrentTask->cTaskState )
      {
        //
        // Check if task is not in sleep
        //
        if(g_psCurrentTask->ulSleepCount == 0)
        {
          //
          // Invoke the task fuction
          //
          iRet = g_psCurrentTask->pfnTaskFunc( g_psCurrentTask->pvParams );

          //
          // check the return value
          //
          if( TASK_RET_DONE == iRet )
          {
            //
            // Mark the task as done base on return value
            //
            g_psCurrentTask->cTaskState = TASK_STATE_DONE;
          }
        }
      }
    }
  }
}


//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
