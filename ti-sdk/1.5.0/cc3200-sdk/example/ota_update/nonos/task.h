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
#ifndef __TASK_H__
#define __TASK_H__

typedef int  (*TaskFunc)(void *);


#define TASK_OK                 0
#define TASK_ERROR              (-1)

#define TASK_RET_IN_PROG        0
#define TASK_RET_DONE           1

#define TASK_STATE_RUN          2
#define TASK_STATE_WAIT         1
#define TASK_STATE_DONE         0


#define MAX_NOF_TASK            5

typedef struct
{
  TaskFunc      pfnTaskFunc;
  void          *pvParams;
  unsigned long ulSleepCount;
  char          cTaskState;
}tTask;


typedef struct
{
  tTask *psTask;

}tSyncObj;

extern int TaskCreate(TaskFunc pfnTaskFunc, void *pvParams);
extern int TaskSyncObjCreate(tSyncObj *psSyncObj);
extern void TaskSyncObjSignal(tSyncObj *psSyncObj);
extern int TaskSyncObjWait(tSyncObj *psSyncObj);
extern int TaskMainLoopStart();
extern void TaskSleep(unsigned long ulSleepCount);

#endif //__TASK_H__
