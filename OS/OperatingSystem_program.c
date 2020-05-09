#include "STD_Types_H.h"
#include "OperatingSystem_interface.h"
#include "OperatingSystem_config.h"
#include "systemClockHandler_interface.h"
#include "SYSTICK_interface.h"

#define READY 0
#define SUSPENDED 1

typedef struct schedularTasks
{
	u32 periodicityTicks;
	u32 tiksToExecute;
	u32 state;
}schedualrTasks;

static void tickHandler(void);

extern taskConfigType taskConfig[NUMBER_OF_TASKS];
schedualrTasks allTasks[NUMBER_OF_TASKS];
static volatile u8 unhandeledTicks;
static u32 preload;
static u32 CPULoad;
static u32 maxCPULoad;
static u16 currentTask;

void OS_schedularInit(void)
{
	u16 i;
	for(i=0;i<NUMBER_OF_TASKS;i++)
	{
		allTasks[i].periodicityTicks=taskConfig[i].task->periodicityMS/TICK_TIME_ms;
		allTasks[i].tiksToExecute=taskConfig[i].firstDelayTicks;
	}
	preload=((u32)TICK_TIME_ms)*(SYS_HAN_getSystemClock()/((u32)1000));
	SYSTICK_init(tickHandler,preload,SYSTICK_CLK_SOURCE_FREE_RUN);
}

void OS_schedularStart(void)
{
	SYSTICK_start();
	while(1)
	{
		if(unhandeledTicks)
		{
			for(currentTask=0;currentTask<NUMBER_OF_TASKS;currentTask++)
			{
				if(allTasks[currentTask].state==READY&&allTasks[currentTask].tiksToExecute==0)
				{
					taskConfig[currentTask].task->taskRunnable();
					allTasks[currentTask].tiksToExecute=allTasks[currentTask].periodicityTicks;
				}
				allTasks[currentTask].tiksToExecute--;
			}
			unhandeledTicks--;
			CPULoad=(((preload-SYSTICK_current())*((u32)100))/preload)+(((u32)unhandeledTicks)*((u32)100));
			if(CPULoad>maxCPULoad)
			{
				maxCPULoad=CPULoad;
			}
			unhandeledTicks=0;
		}
	}
}

void OS_suspendTask(void)
{
	allTasks[currentTask].state=SUSPENDED;
}

void tickHandler(void)
{
	unhandeledTicks++;
}

u32 OS_lastCPULoad(void)
{
	return CPULoad;
}

u32 OS_lastMaxCPULoad(void)
{
	return maxCPULoad;
}
