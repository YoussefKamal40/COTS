#include "STD_Types.h"
#include "NVIC.h"
#include "DMA_registers.h"
#include "DMA_interface.h"



#define NUMBER_OF_CHANNELS 7
#define DMA ((volatile DMAtype*)0x40020000)
#define NVIC_NUMBER(x) ((u8)(x+11))
#define NUMBER_OF_FLAGS 4

void DMA1_Channel1_IRQHandler(void);
void DMA1_Channel2_IRQHandler(void);
void DMA1_Channel3_IRQHandler(void);
void DMA1_Channel4_IRQHandler(void);
void DMA1_Channel5_IRQHandler(void);
void DMA1_Channel6_IRQHandler(void);
void DMA1_Channel7_IRQHandler(void);
static void generalHandler(u8 ch);

static void(*ErrorCallBackFunctions[NUMBER_OF_CHANNELS])(void);
static void(*CompleteCallBackFunctions[NUMBER_OF_CHANNELS])(void);
static void(*HalfCallBackFunctions[NUMBER_OF_CHANNELS])(void);

inline u32 ReadTransfereErrorFlag(u8 ch)
{
	return DMA->ISR&((u32)(DMA_ISR_TEIF1<<((u8)(ch*((u8)NUMBER_OF_FLAGS)))));
}

inline u32 ReadHalfTransfereFlag(u8 ch)
{
	return DMA->ISR&((u32)(DMA_ISR_HTIF1<<((u8)(ch*((u8)NUMBER_OF_FLAGS)))));
}

inline u32 ReadTransfereCompleteFlag(u8 ch)
{
	return DMA->ISR&((u32)(DMA_ISR_TCIF1<<((u8)(ch*((u8)NUMBER_OF_FLAGS)))));
}

inline u32 ReadGlobalInterruptFlag(u8 ch)
{
	return DMA->ISR&((u32)(DMA_ISR_GIF1<<((u8)(ch*((u8)NUMBER_OF_FLAGS)))));
}

inline void ClearTransfereErrorFlag(u8 ch)
{
	DMA->IFCR=(u32)(DMA_IFCR_CTEIF1<<((u8)(ch*((u8)NUMBER_OF_FLAGS))));
}

inline void ClearHalfTransfereFlag(u8 ch)
{
	DMA->IFCR=(u32)(DMA_IFCR_CHTIF1<<((u8)(ch*((u8)NUMBER_OF_FLAGS))));
}

inline void ClearTransfereCompleteFlag(u8 ch)
{
	DMA->IFCR=(u32)(DMA_IFCR_CTCIF1<<((u8)(ch*((u8)NUMBER_OF_FLAGS))));
}

inline void ClearGlobalInterruptFlag(u8 ch)
{
	DMA->IFCR=(u32)(DMA_IFCR_CGIF1<<((u8)(ch*((u8)NUMBER_OF_FLAGS))));
}

inline void EnableChannel(u8 ch)
{
	DMA->channel[ch].CCR|=DMA_CCR_EN;
}

inline void DisableChannel(u8 ch)
{
	DMA->channel[ch].CCR&=~DMA_CCR_EN;
}

inline void EnableTransefereCompleteInt(u8 ch)
{
	DMA->channel[ch].CCR|=DMA_CCR_TCIE;
}

inline void DisableTransefereCompleteInt(u8 ch)
{
	DMA->channel[ch].CCR&=~DMA_CCR_TCIE;
}

inline void EnableHalfTransefereCompleteInt(u8 ch)
{
	DMA->channel[ch].CCR|=DMA_CCR_HTIE;
}

inline void DisableHalfTransefereCompleteInt(u8 ch)
{
	DMA->channel[ch].CCR&=~DMA_CCR_HTIE;
}

inline void EnableErrorInt(u8 ch)
{
	DMA->channel[ch].CCR|=DMA_CCR_TEIE;
}

inline void DisableErrorInt(u8 ch)
{
	DMA->channel[ch].CCR&=~DMA_CCR_TEIE;
}

static void generalHandler(u8 ch)
{
	if(!(DMA->channel[ch].CCR&DMA_CCR_CIRC))
	{
		DisableChannel(ch);
	}
	if(ReadTransfereErrorFlag(ch))
	{
		if(ErrorCallBackFunctions[ch])
		{
			ErrorCallBackFunctions[ch]();
		}
	}
	else
	{
		if(ReadTransfereCompleteFlag(ch))
		{
			if(CompleteCallBackFunctions[ch])
			{
				CompleteCallBackFunctions[ch]();
			}
		}
		if(ReadHalfTransfereFlag(ch))
		{
			if(HalfCallBackFunctions[ch])
			{
				HalfCallBackFunctions[ch]();
			}
		}
	}
	ClearGlobalInterruptFlag(ch);
}

void DMA_init(DMA_configType *config)
{
	u32 temp;

	NVIC_extIntControl(NVIC_NUMBER(config->channel),NVIC_DISABLE);

	DisableChannel(config->channel);

	DMA->channel[config->channel].CPAR=config->prepAddress;

	temp=DMA->channel[config->channel].CCR;
	temp&=DMA_CCR_PL_CLEARMASK;
	temp|=config->periority;
	DMA->channel[config->channel].CCR=temp;

	if(config->direction==DMA_DIR_MEM2PREP)
	{
		DMA->channel[config->channel].CCR|=DMA_CCR_DIR;
	}
	else
	{
		DMA->channel[config->channel].CCR&=~DMA_CCR_DIR;
	}

	switch(config->mode)
	{
	case DMA_MODE_CIRCULAR:
		DMA->channel[config->channel].CCR&=~DMA_CCR_MEM2MEM;
		DMA->channel[config->channel].CCR|=DMA_CCR_CIRC;
		break;
	case DMA_MODE_MEM2MEM:
		DMA->channel[config->channel].CCR&=~DMA_CCR_CIRC;
		DMA->channel[config->channel].CCR|=DMA_CCR_MEM2MEM;
		break;
	case DMA_MODE_NON_MEM2MEM:
		DMA->channel[config->channel].CCR&=~DMA_CCR_CIRC;
		DMA->channel[config->channel].CCR&=~DMA_CCR_MEM2MEM;
	}

	if(config->prepIncrement)
	{
		DMA->channel[config->channel].CCR|=DMA_CCR_PINC;
	}
	else
	{
		DMA->channel[config->channel].CCR&=~DMA_CCR_PINC;
	}

	if(config->memIncrement)
	{
		DMA->channel[config->channel].CCR|=DMA_CCR_MINC;
	}
	else
	{
		DMA->channel[config->channel].CCR&=~DMA_CCR_MINC;
	}

	temp=DMA->channel[config->channel].CCR;
	temp&=DMA_CCR_PSIZE_CLEARMASK;
	temp&=DMA_CCR_MSIZE_CLEARMASK;
	temp|=config->memElementSize;
	temp|=config->prepElementSize;
	DMA->channel[config->channel].CCR=temp;

	if(config->errorNotify)
	{
		ErrorCallBackFunctions[config->channel]=config->errorNotify;
		EnableErrorInt(config->channel);
	}
	else
	{
		DisableErrorInt(config->channel);
	}

	if(config->halfCompleteNotify)
	{
		HalfCallBackFunctions[config->channel]=config->halfCompleteNotify;
		EnableHalfTransefereCompleteInt(config->channel);
	}
	else
	{
		DisableHalfTransefereCompleteInt(config->channel);
	}

	if(config->completeNotify)
	{
		CompleteCallBackFunctions[config->channel]=config->completeNotify;
		EnableTransefereCompleteInt(config->channel);
	}
	else
	{
		DisableTransefereCompleteInt(config->channel);
	}

	NVIC_extIntSetPending(NVIC_NUMBER(config->channel),NVIC_CLEAR);
	NVIC_extIntControl(NVIC_NUMBER(config->channel),NVIC_ENABLE);
}

void DMA_start(u32 memAddress,u16 numberOfData,u8 ch)
{
	DMA->channel[ch].CMAR=memAddress;
	DMA->channel[ch].CNDTR=numberOfData;
	DMA->channel[ch].CCR|=DMA_CCR_EN;
}

void DMA_stop(u8 ch)
{
	DMA->channel[ch].CCR&=~DMA_CCR_EN;
}

void DMA1_Channel1_IRQHandler(void)
{
	generalHandler(DMA_CHANNEL_1);
}

void DMA1_Channel2_IRQHandler(void)
{
	generalHandler(DMA_CHANNEL_2);
}

void DMA1_Channel3_IRQHandler(void)
{
	generalHandler(DMA_CHANNEL_3);
}

void DMA1_Channel4_IRQHandler(void)
{
	generalHandler(DMA_CHANNEL_4);
}

void DMA1_Channel5_IRQHandler(void)
{
	generalHandler(DMA_CHANNEL_5);
}

void DMA1_Channel6_IRQHandler(void)
{
	generalHandler(DMA_CHANNEL_6);
}

void DMA1_Channel7_IRQHandler(void)
{
	generalHandler(DMA_CHANNEL_7);
}
