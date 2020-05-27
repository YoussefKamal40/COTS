#include "STD_Types.h"
#include "NVIC.h"
#include "DMA_interface.h"
#include "systemClockHandler_interface.h"
#include "USART_registers.h"
#include "USART_config.h"
#include "USART_interface.h"

#define USART1_INT_NUM	((u8)37)
#define USART2_INT_NUM	((u8)38)
#define USART3_INT_NUM	((u8)39)
#define UART4_INT_NUM	((u8)52)
#define UART5_INT_NUM	((u8)53)

#define ENABLE 	1
#define DISABLE 0

#define IDLE 0
#define BUSY 1

#define NUMBER_OF_UARTS 5

#define USART_PTR_USART1 0x40013800
#define USART_PTR_USART2 0x40004400
#define USART_PTR_USART3 0x40004800
#define USART_PTR_UART4  0x40004C00
#define USART_PTR_UART5  0x40005000

typedef struct bufferType
{
	u8* ptr;
	u32 position;
	u32 size;
	u8	state;
	u8 	asyncMode;
}bufferType;

typedef void (*DMAFunPtr)(void);

static void DMACH2notify(void);
static void DMACH3notify(void);
static void DMACH4notify(void);
static void DMACH5notify(void);
static void DMACH6notify(void);
static void DMACH7notify(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);
void UART4_IRQHandler(void);
void UART5_IRQHandler(void);
static void USART_GeneralHandler(u8 ID);

static volatile USARTtype* const USARTPtrs[NUMBER_OF_UARTS]=
{
		(volatile USARTtype* const)USART_PTR_USART1,
		(volatile USARTtype* const)USART_PTR_USART2,
		(volatile USARTtype* const)USART_PTR_USART3,
		(volatile USARTtype* const)USART_PTR_UART4,
		(volatile USARTtype* const)USART_PTR_UART5
};

static volatile handlerNotifyType sendNotify[NUMBER_OF_UARTS];
static volatile handlerNotifyType receiveNotify[NUMBER_OF_UARTS];

static volatile bufferType TxBuffer[NUMBER_OF_UARTS];
static volatile bufferType RxBuffer[NUMBER_OF_UARTS];

static const u8 USART_InterruptNumber[NUMBER_OF_UARTS]=
{
		USART1_INT_NUM,
		USART2_INT_NUM,
		USART3_INT_NUM,
		UART4_INT_NUM,
		UART5_INT_NUM
};

static const u8 DMA_RX_Channels[NUMBER_OF_UARTS]={DMA_CHANNEL_5,DMA_CHANNEL_6,DMA_CHANNEL_3,(u8)0,(u8)0};
static const u8 DMA_TX_Channels[NUMBER_OF_UARTS]={DMA_CHANNEL_4,DMA_CHANNEL_7,DMA_CHANNEL_2,(u8)0,(u8)0};

static const DMAFunPtr DMA_RX_notifyMapp[NUMBER_OF_UARTS]={DMACH5notify,DMACH6notify,DMACH3notify,NULL,NULL};
static const DMAFunPtr DMA_TX_notifyMapp[NUMBER_OF_UARTS]={DMACH4notify,DMACH7notify,DMACH2notify,NULL,NULL};

static volatile handlerNotifyType linBreakNotify[NUMBER_OF_UARTS];

void USART_init(u8 ID)
{
	volatile USARTtype* USART;
	f32 BRRValue;
	f32 BRRFraction;
	u32 BRRMantessa;
#if ASYNC_MODE==USART_DMA_ASYNC_MODE
	DMA_configType DMA_config;
#endif


	USART=USARTPtrs[ID];
	NVIC_extIntControl(USART_InterruptNumber[ID],NVIC_DISABLE);

	/*Enable UART*/
	USART->CR1|=USART_CR1_UE;

	/*Disable interrupt*/
	USART->CR1&=~USART_CR1_TXEIE;
	USART->CR1&=~USART_CR1_RXNEIE;

	/*Set Word Length to 8 bit*/
	USART->CR1&=~USART_CR1_M;

	/*Stop bit configuration*/
	USART->CR2=(USART->CR2&USART_CR2_STOP_CLEARMASK)|(STOP_CONFIG);

	/*USART Parity configuer*/
#if PARITY_CONFIG == USART_PARITY_DISABLE
	USART->CR1&=~USART_CR1_PCE;
#elif PARITY_CONFIG == USART_PARITY_EVEN
	USART->CR1|=USART_CR1_PCE;
#elif PARITY_CONFIG == SART_PARITY_ODD
	USART->CR1|=USART_CR1_PCE|USART_CR1_PS;
#endif

	/*Set Baud Rate*/
	if(ID==USART_ID_USART1)
	{
		BRRValue=((f32)SYS_HAN_getAPB2Clock())/(((f32)16)*BAUDRATE_CONFIG);
	}
	else
	{
		BRRValue=((f32)SYS_HAN_getAPB1Clock())/(((f32)16)*BAUDRATE_CONFIG);
	}
	BRRMantessa=(u32)BRRValue;
	BRRFraction=(BRRValue-((f32)BRRMantessa))*((f32)16);
	if((BRRFraction-((u32)BRRFraction))>=((f32)0.5))
	{
		BRRFraction++;
	}
	USART->BRR=((u32)(BRRMantessa<<USART_BRR_DIV_Mantissa_OFFSET))+((u32)BRRFraction);

	/*Transmit and receive enable*/
	USART->CR1|=USART_CR1_TE|USART_CR1_RE;

#if ASYNC_MODE==USART_INTERRUPT_ASYNC_MODE
	/*Enable NVIC*/
	RxBuffer[ID].asyncMode=USART_INTERRUPT_ASYNC_MODE;
	TxBuffer[ID].asyncMode=USART_INTERRUPT_ASYNC_MODE;
	NVIC_extIntSetPending(USART_InterruptNumber[ID],NVIC_CLEAR);
	NVIC_extIntControl(USART_InterruptNumber[ID],NVIC_ENABLE);
#elif ASYNC_MODE==USART_DMA_ASYNC_MODE
	RxBuffer[ID].asyncMode=USART_DMA_ASYNC_MODE;
	TxBuffer[ID].asyncMode=USART_DMA_ASYNC_MODE;
	/*RX configuration*/
	DMA_config.channel=DMA_RX_Channels[ID];
	DMA_config.completeNotify=DMA_RX_notifyMapp[ID];
	DMA_config.direction=DMA_DIR_PREP2MEM;
	DMA_config.errorNotify=NULL;
	DMA_config.halfCompleteNotify=NULL;
	DMA_config.memElementSize=DMA_MEM_ELEMENT_SIZE_8b;
	DMA_config.memIncrement=DMA_ENABLE;
	DMA_config.mode=DMA_MODE_NON_MEM2MEM;
	DMA_config.periority=DMA_PERIORITY_VERY_HIGH;
	DMA_config.prepAddress=(u32)(&(USART->DR));
	DMA_config.prepElementSize=DMA_PREP_ELEMENT_SIZE_32b;
	DMA_config.prepIncrement=DMA_DISABLE;
	DMA_init(&DMA_config);

	/*TX configuration*/
	DMA_config.channel=DMA_TX_Channels[ID];
	DMA_config.completeNotify=DMA_TX_notifyMapp[ID];
	DMA_config.direction=DMA_DIR_MEM2PREP;
	DMA_config.periority=DMA_PERIORITY_LOW;
	DMA_init(&DMA_config);
#endif

	RxBuffer[ID].state=IDLE;
	TxBuffer[ID].state=IDLE;
}

void USART_config(USARTConfig_Type* data)
{
	volatile USARTtype* USART;
	f32 BRRValue;
	f32 BRRFraction;
	u32 BRRMantessa;
	u8 USARTIndex;
	DMA_configType DMA_config;

	USART=USARTPtrs[data->ID];
	USARTIndex=data->ID;
	RxBuffer[USARTIndex].asyncMode=data->asyncMode;
	TxBuffer[USARTIndex].asyncMode=data->asyncMode;
	NVIC_extIntControl(USART_InterruptNumber[USARTIndex],NVIC_DISABLE);

	/*Enable UART*/
	USART->CR1|=USART_CR1_UE;

	/*Disable interrupt*/
	USART->CR1&=~USART_CR1_TXEIE;
	USART->CR1&=~USART_CR1_RXNEIE;

	/*Set Word Length to 8 bit*/
	USART->CR1&=~USART_CR1_M;

	/*Stop bit configuration*/
	USART->CR2=(USART->CR2&USART_CR2_STOP_CLEARMASK)|(data->stopBit);

	/*USART Parity configuer*/
	switch(data->parity)
	{
	case (u8)USART_PARITY_DISABLE:
			USART->CR1&=~USART_CR1_PCE;
	break;
	case (u8)USART_PARITY_EVEN:
			USART->CR1|=USART_CR1_PCE;
	break;
	case (u8)USART_PARITY_ODD:
			USART->CR1|=USART_CR1_PCE|USART_CR1_PS;
	}
	/*Set Baud Rate*/
	if(USARTIndex==USART_ID_USART1)
	{
		BRRValue=((f32)SYS_HAN_getAPB2Clock())/(((f32)16)*data->baudRate);
	}
	else
	{
		BRRValue=((f32)SYS_HAN_getAPB1Clock())/(((f32)16)*data->baudRate);
	}
	BRRMantessa=(u32)BRRValue;
	BRRFraction=(BRRValue-((f32)BRRMantessa))*((f32)16);
	if((BRRFraction-((u32)BRRFraction))>=((f32)0.5))
	{
		BRRFraction++;
	}
	USART->BRR=((u32)(BRRMantessa<<USART_BRR_DIV_Mantissa_OFFSET))+((u32)BRRFraction);

	/*Transmit and receive enable*/
	USART->CR1|=USART_CR1_TE|USART_CR1_RE;

	if(data->asyncMode==USART_INTERRUPT_ASYNC_MODE)
	{
		/*Enable NVIC*/
		NVIC_extIntSetPending(USART_InterruptNumber[USARTIndex],NVIC_CLEAR);
		NVIC_extIntControl(USART_InterruptNumber[USARTIndex],NVIC_ENABLE);
	}
	else
	{
		/*RX configuration*/
		DMA_config.channel=DMA_RX_Channels[USARTIndex];
		DMA_config.completeNotify=DMA_RX_notifyMapp[USARTIndex];
		DMA_config.direction=DMA_DIR_PREP2MEM;
		DMA_config.errorNotify=NULL;
		DMA_config.halfCompleteNotify=NULL;
		DMA_config.memElementSize=DMA_MEM_ELEMENT_SIZE_8b;
		DMA_config.memIncrement=DMA_ENABLE;
		DMA_config.mode=DMA_MODE_NON_MEM2MEM;
		DMA_config.periority=DMA_PERIORITY_VERY_HIGH;
		DMA_config.prepAddress=(u32)(&(USART->DR));
		DMA_config.prepElementSize=DMA_PREP_ELEMENT_SIZE_32b;
		DMA_config.prepIncrement=DMA_DISABLE;
		DMA_init(&DMA_config);

		/*TX configuration*/
		DMA_config.channel=DMA_TX_Channels[USARTIndex];
		DMA_config.completeNotify=DMA_TX_notifyMapp[USARTIndex];
		DMA_config.direction=DMA_DIR_MEM2PREP;
		DMA_config.periority=DMA_PERIORITY_LOW;
		DMA_init(&DMA_config);
	}

	RxBuffer[USARTIndex].state=IDLE;
	TxBuffer[USARTIndex].state=IDLE;
}

USART_ErrorType USART_send(u8 ID,const u8* buffer,u16 length,handlerNotifyType notifyFunction)
{
	volatile USARTtype* USART;
	USART_ErrorType error=USART_ERROR_OK;

	if(NUMBER_OF_UARTS>ID)
	{
		USART=USARTPtrs[ID];
		if(TxBuffer[ID].state==IDLE)
		{
			if(length>=1)
			{
				TxBuffer[ID].state=BUSY;
				sendNotify[ID]=notifyFunction;
				if(TxBuffer[ID].asyncMode==USART_INTERRUPT_ASYNC_MODE)
				{
					TxBuffer[ID].ptr=(u8*)buffer;
					TxBuffer[ID].size=length;
					TxBuffer[ID].position=0;
					if(USART->SR&USART_SR_TXE)
					{
						USART->DR=*buffer;
						if(length==1)
						{
							if(notifyFunction)
							{
								notifyFunction();
							}
							TxBuffer[ID].state=IDLE;
							return error;
						}
						TxBuffer[ID].position++;
					}

					if(length==1)
					{
						USART->CR1|=USART_CR1_TCIE;
					}
					else
					{
						/*Enable Interrupt*/
						USART->CR1|=USART_CR1_TXEIE;
					}
				}
				else
				{
					if(USART->SR&USART_SR_TXE)
					{
						USART->DR=*buffer;
						if(length==1)
						{
							notifyFunction();
						}
						else
						{
							USART->CR3|=USART_CR3_DMAT;
							DMA_start((u32)(buffer+1),((u16)(length-1)),DMA_TX_Channels[ID]);
						}
					}
					else
					{
						USART->CR3|=USART_CR3_DMAT;
						DMA_start((u32)buffer,length,DMA_TX_Channels[ID]);
					}

				}
			}
			else
			{
				error=USART_ERROR_NOK;
			}
		}
		else
		{
			error=USART_ERROR_NOK;
		}
	}
	else
	{
		error=USART_ERROR_NOK;
	}
	return error;
}

USART_ErrorType USART_receive(u8 ID,u8* buffer,u16 length,handlerNotifyType notifyFunction)
{
	volatile USARTtype* USART;
	USART_ErrorType error=USART_ERROR_OK;

	if(NUMBER_OF_UARTS>ID)
	{
		USART=USARTPtrs[ID];
		if(RxBuffer[ID].state==IDLE)
		{
			if(length>=1)
			{
				RxBuffer[ID].state=BUSY;
				receiveNotify[ID]=notifyFunction;
				if(RxBuffer[ID].asyncMode==USART_INTERRUPT_ASYNC_MODE)
				{
					RxBuffer[ID].ptr=buffer;
					RxBuffer[ID].size=length;
					RxBuffer[ID].position=0;
					*buffer=(u8)USART->DR;
					/*Enable Interrupt*/
					USART->CR1|=USART_CR1_RXNEIE;
				}
				else
				{
					*buffer=(u8)USART->DR;
					USART->CR3|=USART_CR3_DMAR;
					DMA_start((u32)buffer,length,DMA_RX_Channels[ID]);
				}
			}
			else
			{
				error=USART_ERROR_NOK;
			}
		}
		else
		{
			error=USART_ERROR_NOK;
		}
	}
	else
	{
		error=USART_ERROR_NOK;
	}
	return error;
}

static void USART_GeneralHandler(u8 ID)
{
	volatile USARTtype* USART;

	USART=USARTPtrs[ID];
	if((USART->SR&USART_SR_LBD)&&linBreakNotify[ID])
	{
		USART->SR=~USART_SR_LBD;
		linBreakNotify[ID]();
	}
	if(RxBuffer[ID].state==BUSY)
	{
		if(USART->SR&USART_SR_RXNE)
		{
			if((RxBuffer[ID].size-RxBuffer[ID].position)==1)
			{
				/*Disable Interrupt*/
				USART->CR1&=~USART_CR1_RXNEIE;
				RxBuffer[ID].ptr[RxBuffer[ID].position]=(u8)USART->DR;
				RxBuffer[ID].state=IDLE;
				if(receiveNotify[ID])
				{
					receiveNotify[ID]();
				}
			}
			else
			{
				RxBuffer[ID].ptr[RxBuffer[ID].position]=(u8)USART->DR;
				RxBuffer[ID].position++;
			}
		}
	}
	if(TxBuffer[ID].state==BUSY)
	{
		if(USART->SR&USART_SR_TXE)
		{
			if((TxBuffer[ID].size-TxBuffer[ID].position)==1)
			{
				/*Disable Interrupt*/
				USART->CR1&=~USART_CR1_TXEIE;
				USART->DR=TxBuffer[ID].ptr[TxBuffer[ID].position];
				TxBuffer[ID].position++;
				/*Enable transmit complete interrupt*/
				USART->CR1|=USART_CR1_TCIE;
			}
			else if(TxBuffer[ID].size==TxBuffer[ID].position)
			{
				TxBuffer[ID].state=IDLE;
				if(sendNotify[ID])
				{
					sendNotify[ID]();
				}
				/*Disable transmit complete interrupt*/
				USART->CR1&=~USART_CR1_TCIE;
			}
			else
			{
				USART->DR=TxBuffer[ID].ptr[TxBuffer[ID].position];
				TxBuffer[ID].position++;
			}
		}
	}
}

void USART1_IRQHandler(void)
{
	USART_GeneralHandler(USART_ID_USART1);
}

void USART2_IRQHandler(void)
{
	USART_GeneralHandler(USART_ID_USART2);
}

void USART3_IRQHandler(void)
{
	USART_GeneralHandler(USART_ID_USART3);
}

void UART4_IRQHandler(void)
{
	USART_GeneralHandler(USART_ID_UART4);
}

void UART5_IRQHandler(void)
{
	USART_GeneralHandler(USART_ID_UART5);
}

static void DMACH2notify(void)
{
	volatile USARTtype* USART;

	USART=USARTPtrs[USART_ID_USART3];
	USART->CR3&=~USART_CR3_DMAT;
	TxBuffer[USART_ID_USART3].state=IDLE;
	if(sendNotify[USART_ID_USART3])
	{
		sendNotify[USART_ID_USART3]();
	}
}

static void DMACH3notify(void)
{
	volatile USARTtype* USART;

	USART=USARTPtrs[USART_ID_USART3];
	USART->CR3&=~USART_CR3_DMAR;
	RxBuffer[USART_ID_USART3].state=IDLE;
	if(receiveNotify[USART_ID_USART3])
	{
		receiveNotify[USART_ID_USART3]();
	}
}

static void DMACH4notify(void)
{
	volatile USARTtype* USART;

	USART=USARTPtrs[USART_ID_USART1];
	USART->CR3&=~USART_CR3_DMAT;
	TxBuffer[USART_ID_USART1].state=IDLE;
	if(sendNotify[USART_ID_USART1])
	{
		sendNotify[USART_ID_USART1]();
	}
}

static void DMACH5notify(void)
{
	volatile USARTtype* USART;

	USART=USARTPtrs[USART_ID_USART1];
	USART->CR3&=~USART_CR3_DMAR;
	RxBuffer[USART_ID_USART1].state=IDLE;
	if(receiveNotify[USART_ID_USART1])
	{
		receiveNotify[USART_ID_USART1]();
	}
}

static void DMACH6notify(void)
{
	volatile USARTtype* USART;

	USART=USARTPtrs[USART_ID_USART2];
	USART->CR3&=~USART_CR3_DMAR;
	RxBuffer[USART_ID_USART2].state=IDLE;
	if(receiveNotify[USART_ID_USART2])
	{
		receiveNotify[USART_ID_USART2]();
	}
}

static void DMACH7notify(void)
{
	volatile USARTtype* USART;

	USART=USARTPtrs[USART_ID_USART2];
	USART->CR3&=~USART_CR3_DMAT;
	TxBuffer[USART_ID_USART2].state=IDLE;
	if(sendNotify[USART_ID_USART2])
	{
		sendNotify[USART_ID_USART2]();
	}
}

void USART_setLinBreakCallback(u8 ID,handlerNotifyType callback)
{
	linBreakNotify[ID]=callback;
	NVIC_extIntControl(USART_InterruptNumber[ID],NVIC_ENABLE);
	USARTPtrs[ID]->CR2|=USART_CR2_LINEN;
}

void USART_sendLinBreak(u8 ID)
{
	USARTPtrs[ID]->CR1|=USART_CR1_SBK;
}

void USART_stopProcess(u8 ID)
{
	if(TxBuffer[ID].state==USART_INTERRUPT_ASYNC_MODE)
	{
		USARTPtrs[ID]->CR1&=~(USART_CR1_TXEIE|USART_CR1_TCIE);
	}
	else
	{
		DMA_stop(DMA_TX_Channels[ID]);
	}
	TxBuffer[ID].state=IDLE;
	if(RxBuffer[ID].state==USART_INTERRUPT_ASYNC_MODE)
	{
		USARTPtrs[ID]->CR1&=~USART_CR1_RXNEIE;
	}
	else
	{
		DMA_stop(DMA_RX_Channels[ID]);
	}
	RxBuffer[ID].state=IDLE;
}
