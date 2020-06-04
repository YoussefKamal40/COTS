#include "STD_Types.h"
#include "SwitchHandler.h"
#include "OperatingSystem_interface.h"
#include "GPIO_Interface.h"
#include "SwitchHandler_config.h"

#define VALUE_MASK	((u8)1)

static void runnable (void);

extern const switchConfigType switchesConfig[];
const taskType switchTask={.taskRunnable=runnable,.periodicityMS=CHECK_PERIODICITYms};
static u8 stableValues[NUMBER_OF_SWITCHES];

void Switch_switchesInit(void)
{
	GPIO_pinType tempPin;
	u8 switchesIndex;

	tempPin.speed=GPIO_50M_SPEED;
	for(switchesIndex=0;switchesIndex<NUMBER_OF_SWITCHES;switchesIndex++)
	{
		tempPin.mode=switchesConfig[switchesIndex].swMode;
		tempPin.pin=switchesConfig[switchesIndex].GPIOPin;
		tempPin.port=switchesConfig[switchesIndex].GPIOPort;
		GPIO_configPins(&tempPin);
	}
}

u8 Switch_getPinValue(u8 ID)
{
	return stableValues[ID];
}

void runnable (void)
{
	u8 actualValue,switchesIndex;
	static u8 timers[NUMBER_OF_SWITCHES];

	for(switchesIndex=0;switchesIndex<NUMBER_OF_SWITCHES;switchesIndex++)
	{
		if(switchesConfig[switchesIndex].pressedGPIOValue==GPIO_getPinValue(switchesConfig[switchesIndex].GPIOPort,switchesConfig[switchesIndex].GPIOPin))
		{
			actualValue=SWITCH_PRESSED;
		}
		if(actualValue==stableValues[switchesIndex])
		{
			timers[switchesIndex]=0;
		}
		else
		{
			timers[switchesIndex]++;
			if(timers[switchesIndex]>UNSTABLE_TIMEOUT_TICKS)
			{
				stableValues[switchesIndex]=actualValue;
			}
		}
	}
}
