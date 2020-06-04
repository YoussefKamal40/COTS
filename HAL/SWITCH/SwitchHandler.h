#ifndef SWITCHHANDLER_H_
#define SWITCHHANDLER_H_

#define SWITCH_PRESSED		((u8)1)
#define SWITCH_NOT_PRESSED	((u8)0)

#define SWITCH_INPUT_FLOAT_MODE		GPIO_INPUT_FLOAT_MODE
#define SWITCH_INPUT_PULL_UP_MODE	GPIO_INPUT_PULL_UP_MODE
#define SWITCH_INPUT_PULL_DOWN_MODE	GPIO_INPUT_PULL_DOWN_MODE

typedef struct switchConfigType
{
	u32 swMode;
	void* GPIOPort;
	u16 GPIOPin;
	u8 pressedGPIOValue;
}switchConfigType;

void Switch_switchesInit(void);
u8 Switch_getPinValue(u8 ID);

#endif /* SWITCHHANDLER_H_ */
