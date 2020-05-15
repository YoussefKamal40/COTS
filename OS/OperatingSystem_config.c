#include "STD_Types.h"
#include "OperatingSystem_interface.h"
#include "OperatingSystem_config.h"

extern const taskType turneOnTask;
extern const taskType turneOffTask;
extern const taskType loadPrinterTask;
extern const taskType dummyTask;

const taskConfigType taskConfig[NUMBER_OF_TASKS]=
{
		{.task=&turneOnTask,.firstDelayTicks=25},
		{.task=&turneOffTask,.firstDelayTicks=75},
		{.task=&loadPrinterTask,.firstDelayTicks=0},
		{.task=&dummyTask,.firstDelayTicks=0}
};
