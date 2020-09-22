#include "STD_Types.h"
#include "LEDHandler_Interface.h"

const LEDHandlerConfigType ledsConfig[NUMBER_OF_LEDS]=
{
		{
				.ledPin=
				{
						.mode=LEDH_PP_MODE,
						.speed=LEDH_50M_SPEED,
						.port=LEDH_C_PORT,
						.pin=LEDH_PIN_13
				},
				.initState=LEDH_STATE_OFF,
				.activeState=LEDH_ACTIVE_STATE_LOW
		}
};
