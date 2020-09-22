#ifndef LEDHANDLER_INTERFACE_H_
#define LEDHANDLER_INTERFACE_H_

#include "GPIO_Interface.h"

#define LEDH_PP_MODE		GPIO_GPOUT_PP_MODE
#define LEDH_OD_MODE		GPIO_GPOUT_OD_MODE
#define LEDH_10M_SPEED	        GPIO_10M_SPEED
#define LEDH_2M_SPEED	        GPIO_2M_SPEED
#define LEDH_50M_SPEED	        GPIO_50M_SPEED

#define LEDH_PIN_0       GPIO_PIN_0
#define LEDH_PIN_1       GPIO_PIN_1
#define LEDH_PIN_2       GPIO_PIN_2
#define LEDH_PIN_3       GPIO_PIN_3
#define LEDH_PIN_4       GPIO_PIN_4
#define LEDH_PIN_5       GPIO_PIN_5
#define LEDH_PIN_6       GPIO_PIN_6
#define LEDH_PIN_7       GPIO_PIN_7
#define LEDH_PIN_8       GPIO_PIN_8
#define LEDH_PIN_9       GPIO_PIN_9
#define LEDH_PIN_10      GPIO_PIN_10
#define LEDH_PIN_11      GPIO_PIN_11
#define LEDH_PIN_12      GPIO_PIN_12
#define LEDH_PIN_13      GPIO_PIN_13
#define LEDH_PIN_14      GPIO_PIN_14
#define LEDH_PIN_15      GPIO_PIN_15

#define LEDH_A_PORT	GPIO_A_PORT
#define LEDH_B_PORT GPIO_B_PORT
#define LEDH_C_PORT GPIO_C_PORT
#define LEDH_D_PORT GPIO_D_PORT
#define LEDH_E_PORT GPIO_E_PORT
#define LEDH_F_PORT GPIO_F_PORT
#define LEDH_G_PORT GPIO_G_PORT

#define LEDH_ACTIVE_STATE_HIGH ((u8)0)
#define LEDH_ACTIVE_STATE_LOW  ((u8)1)

#define LEDH_STATE_ON 	GPIO_HIGH_VALUE
#define LEDH_STATE_OFF 	GPIO_LOW_VALUE

#pragma pack(4)
typedef struct LEDHandlerConfigType
{
	GPIO_pinType ledPin;
	u8 initState;
	u8 activeState;
}LEDHandlerConfigType;

void LED_init(void);
void LED_setState(u8 ledID,u8 state);

#endif /* LEDHANDLER_INTERFACE_H_ */
