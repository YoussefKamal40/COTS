#include "STD_TYPES_H.h"
#include "GPIO_Interface.h"

#define GPIO_u8_CONFIG_BITS_NUMBER ((u8)4)
#define GPIO_u32_CONFIG_BITS_MASK	((u32)0xf)

#pragma pack(4)
typedef struct GPIOType
{
	u32 CRL;
	u32 CRH;
	u32 IDR;
	u32 ODR;
	u32 BSR;
	u32 BRR;
	u32 LCKR;
}GPIOType;

void GPIO_configPins(GPIO_pinType* pins)
{
	volatile GPIOType* GPIO;
	u8 i;
	u8 shift;
	u32 mode;
	u32 temp;

	GPIO=((volatile GPIOType* )pins->port);
	if(pins->pin&((u16)0xff))
	{
		for(i=0;i<8;i++)
		{
			if(pins->pin&((u16)(1<<i)))
			{
				shift=(u8)(i*GPIO_u8_CONFIG_BITS_NUMBER);
				mode=(pins->mode>>1)&pins->speed;
				temp=GPIO->CRL;
				temp&=~(GPIO_u32_CONFIG_BITS_MASK<<shift);
				temp|=(u32)(mode<<shift);
				GPIO->CRL=temp;
				if(pins->mode&1)
				{
					GPIO->BSR=(u32)(1<<i);
				}
				else
				{
					GPIO->BRR=(u32)(1<<i);
				}
			}
		}
	}
	if(pins->pin&((u16)0xff00))
	{
		for(i=8;i<16;i++)
		{
			if(pins->pin&((u16)(1<<i)))
			{
				shift=(u8)((i-8)*GPIO_u8_CONFIG_BITS_NUMBER);
				mode=(pins->mode>>1)&pins->speed;
				temp=GPIO->CRH;
				temp&=~(GPIO_u32_CONFIG_BITS_MASK<<shift);
				temp|=(u32)(mode<<shift);
				GPIO->CRH=temp;
				if(pins->mode&1)
				{
					GPIO->BSR=(u32)(1<<i);
				}
				else
				{
					GPIO->BRR=(u32)(1<<i);
				}
			}
		}
	}
}

void GPIO_setPinValue(void* port,u16 pin,u8 value)
{
	volatile GPIOType* GPIO;

	GPIO=(volatile GPIOType* )port;
	if(value)
	{
		GPIO->BSR=(u32)pin;
	}
	else
	{
		GPIO->BRR=(u32)pin;
	}
}

void GPIO_maskedWrite(void* port,u16 pins,u16 value)
{
	((volatile GPIOType* )port)->ODR=(((volatile GPIOType* )port)->ODR&(~((u32)pins)))|(((u32)pins)&((u32)value));
}

u8 GPIO_getPinValue(void* port,u16 pin)
{
	if(((volatile GPIOType* )port)->IDR&pin)
	{
		return GPIO_HIGH_VALUE;
	}
	else
	{
		return GPIO_LOW_VALUE;
	}
}

u16 GPIO_getPortValue(void* port)
{
	return (u16)(((volatile GPIOType* )port)->IDR);
}
