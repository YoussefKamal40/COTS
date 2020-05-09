#include "STD_TYPES_H.h"
#include "LEDHandler_interface.h"
#include "LEDHandler_config.h"

extern LEDHandlerConfigType ledsConfig[NUMBER_OF_LEDS];

void LED_init(void)
{
	u8 i;
	for(i=0;i<NUMBER_OF_LEDS;i++)
	{
		GPIO_configPins(&ledsConfig[i].ledPin);
		GPIO_setPinValue(ledsConfig[i].ledPin.port,ledsConfig[i].ledPin.pin,ledsConfig[i].initState^ledsConfig[i].activeState);
	}
}

void LED_setState(u8 ledID,u8 state)
{
	/*Check if input is valid*/
	if(ledID<NUMBER_OF_LEDS)
	{
		GPIO_setPinValue(ledsConfig[ledID].ledPin.port,ledsConfig[ledID].ledPin.pin,state^ledsConfig[ledID].activeState);
	}
}
