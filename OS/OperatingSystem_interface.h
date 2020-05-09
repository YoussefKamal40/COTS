#ifndef OPERATINGSYSTEM_INTERFACE_H_
#define OPERATINGSYSTEM_INTERFACE_H_

typedef void(*taskRunnableType)(void);

typedef struct
{
	taskRunnableType taskRunnable;
	u32 periodicityMS;
}taskType;

typedef struct taskConfigType
{
	const taskType* task;
	u32 firstDelayTicks;
}taskConfigType;

void OS_schedularInit(void);
void OS_schedularStart(void);
void OS_suspendTask(void);
u32 OS_lastCPULoad(void);
u32 OS_lastMaxCPULoad(void);

#endif /* OPERATINGSYSTEM_INTERFACE_H_ */
