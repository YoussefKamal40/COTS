#include "STD_Types.h"
#include "LIN.h"
#include "USART_interface.h"
#include "OperatingSystem_interface.h"
#include "RCC_interface.h"
#include "GPIO_Interface.h"
#include "NVIC.h"
#include "DMA_interface.h"

#define SYNC_BYTE			((u8)0x55)

#define DATA_RECEIVED		((u8)0xff)
#define	DATA_NOT_RECEIVED	((u8)0x00)

#define	PID_BUFFER_INDEX	1
#define SYNC_BUFFER_INDEX	0
#define HEADER_SIZE			2

#define NUMBER_OF_UARTS 			3
#define MAXIMUM_FRAME_SIZE			8
#define CHECKSUM_SIZE				1
#define MAXIMUM_SCALER_SGNAL_SIZE	16
#define BYTE_SIZE					8

#define IDLE						0
#define RECEIVING_HEADER			1
#define SENDING_DATA_CHECKSUM		2
#define RECEIVING_DATA_CHECKSUM		3

#define ID_MASK			((u8)0x3F)
#define GET_BIT(x,y)	((x>>y)&1)
#define ID_GET(PID)		((u8)(ID_MASK&PID))
#define PID_GET(ID)		((u8)(ID|((GET_BIT(ID,0)^GET_BIT(ID,1)^GET_BIT(ID,2)^GET_BIT(ID,4))<<6)|((GET_BIT(ID,1)^GET_BIT(ID,3)^GET_BIT(ID,4)^GET_BIT(ID,5))<<7)))

#define UPDATED							((u8)0x00)
#define NOT_UPDATED						((u8)0xff)
#define EVENT_TRIG_READY_FOR_PUBLISH	((u8)0x55)

#define BUSY		((u8)0xff)
#define NOT_BUSY	((u8)0x00)

#define CLUSTER_INDEX_0	0
#define CLUSTER_INDEX_1	1
#define CLUSTER_INDEX_2	2

#define USART1_INT_NUM	((u8)37)
#define USART2_INT_NUM	((u8)38)
#define USART3_INT_NUM	((u8)39)

#define DMA_NVIC_NUMBER(x) ((u8)(x+11))

/* #define UPDATED
 * All frames buffer and signals is updated.
 * Publisher have to send.
 * Receiver doesn't have to update signals
 * */

typedef void (*FuncType) (void);

static void mainRunnable(void);
static void linBreakHandler0(void);
static void linBreakHandler1(void);
static void linBreakHandler2(void);
static void receiveHandler0(void);
static void receiveHandler1(void);
static void receiveHandler2(void);
static void sendHandler0(void);
static void sendHandler1(void);
static void sendHandler2(void);
static void generalLinBreakHandler(u8 clusterIndex);
static void generalReceiveHandler(u8 clusterIndex);
static void generalSendHandler(u8 clusterIndex);
static u8	checksumCalcuklator(u8*data,u8 length,u8 pid);
static void enterCriticalSection(u8 clusterIndex);
static void exitCriticalSection(u8 clusterIndex);

const				taskType 				Lin_runnable={.taskRunnable=mainRunnable,.periodicityMS=BASE_TIME};
extern				u16 					scalerSignals[];
extern 	u8* 		const 					byteArraySignals[];
extern 	const 		LIN_clusterConfigType 	clusters[NUMBER_OF_CLUSTERS];
static 				u8 						allFrames[NUMBER_OF_FRAMES][MAXIMUM_FRAME_SIZE+CHECKSUM_SIZE];
static	volatile	u8						allFramesUpdateFlag[NUMBER_OF_FRAMES];
static	volatile	u8						allFramesState[NUMBER_OF_FRAMES];
static 	volatile	u8						slaveTaskState[NUMBER_OF_CLUSTERS];
static 	volatile	u8						currentBusyFrame[NUMBER_OF_CLUSTERS];
static				u8						scalerSignalsFrameIndexMap[NUMBER_OF_SCALER_SIGNALS];
static				u8						byteArraySignalsFrameIndexMap[NUMBER_OF_SCALER_SIGNALS];
static				u8						PIDs[NUMBER_OF_FRAMES];
static				u8						clustersAllFramesMarkers[NUMBER_OF_CLUSTERS];
static				u8						headerSendBuffer[NUMBER_OF_CLUSTERS][HEADER_SIZE];
static				u8						headerReceiveBuffer[NUMBER_OF_CLUSTERS][HEADER_SIZE];
static				u16						allFramesTicksTosend[NUMBER_OF_FRAMES];
static	const		FuncType				breakNotifyMap[NUMBER_OF_UARTS]={linBreakHandler0,linBreakHandler1,linBreakHandler2};
static	const		FuncType				receiveHandlerMap[NUMBER_OF_UARTS]={receiveHandler0,receiveHandler1,receiveHandler2};
static	const		FuncType				sendHandlerMap[NUMBER_OF_UARTS]={sendHandler0,sendHandler1,sendHandler2};
static 	const 		u32 					RCCPrephirals[NUMBER_OF_UARTS]={RCC_u32_CONTROL_PERIP_USART1EN,RCC_u32_CONTROL_PERIP_USART2EN,RCC_u32_CONTROL_PERIP_USART3EN};
static 	const 		u16 					txGPIOPin[NUMBER_OF_UARTS]={GPIO_PIN_9,GPIO_PIN_2,GPIO_PIN_10};
static 	const 		u16 					rxGPIOPin[NUMBER_OF_UARTS]={GPIO_PIN_10,GPIO_PIN_3,GPIO_PIN_11};
static 	const 		void* 					GPIOPorts[NUMBER_OF_UARTS]={GPIO_A_PORT,GPIO_A_PORT,GPIO_B_PORT};
static 	const 		u8 						USART_InterruptNumber[NUMBER_OF_UARTS]={USART1_INT_NUM,USART2_INT_NUM,USART3_INT_NUM};
static 	const 		u8 						DMA_RX_Channels[NUMBER_OF_UARTS]={DMA_CHANNEL_5,DMA_CHANNEL_6,DMA_CHANNEL_3};
static 	const 		u8 						DMA_TX_Channels[NUMBER_OF_UARTS]={DMA_CHANNEL_4,DMA_CHANNEL_7,DMA_CHANNEL_2};

void LIN_init(void)
{
	u8 					signalsIndex,framesIndex,clustersIndex,allFramesIndex,signalDataIndex;
	u8					signalShift,signalLength,USARTID,signalAlignmentShift;
	u16					scalerSignal;
	const	u8*			byteArraySignal;
	USARTConfig_Type 	UARTConfig;
	GPIO_pinType 		tx,rx;
	void* 				GPIOPort;

	allFramesIndex=0;
	UARTConfig.parity=USART_PARITY_DISABLE;
	UARTConfig.stopBit=USART_STOP_1;
	tx.mode=GPIO_AFOUT_OD_MODE;
	tx.speed=GPIO_50M_SPEED;
	rx.mode=GPIO_INPUT_FLOAT_MODE;
	rx.speed=GPIO_50M_SPEED;
	for(clustersIndex=0;clustersIndex<NUMBER_OF_CLUSTERS;clustersIndex++)
	{
		clustersAllFramesMarkers[clustersIndex]=allFramesIndex;
		USARTID=clusters[clustersIndex].UART_ID;
		GPIOPort=(void*)GPIOPorts[USARTID];
		tx.port=GPIOPort;
		tx.pin=txGPIOPin[USARTID];
		rx.port=GPIOPort;
		rx.pin=rxGPIOPin[USARTID];
		UARTConfig.asyncMode=clusters[clustersIndex].UART_asyncMode;
		UARTConfig.ID=USARTID;
		UARTConfig.baudRate=clusters[clustersIndex].bitRate;
		RCC_u8_ControlPrephiral(RCCPrephirals[USARTID],RCC_u8_ENABLE);
		GPIO_configPins(&tx);
		GPIO_configPins(&rx);
		USART_config(&UARTConfig);
		USART_setLinBreakCallback(USARTID,breakNotifyMap[clustersIndex]);
		headerSendBuffer[clustersIndex][SYNC_BUFFER_INDEX]=SYNC_BYTE;
		for(framesIndex=0;framesIndex<clusters[clustersIndex].numberOfFrames;framesIndex++)
		{
			PIDs[allFramesIndex]=PID_GET(clusters[clustersIndex].frames[framesIndex].ID);
			allFramesTicksTosend[allFramesIndex]=clusters[clustersIndex].frames[framesIndex].firstDelay;
			if(clusters[clustersIndex].frames[framesIndex].direction==LIN_FRAME_DIRECTION_PUBLISHER)
			{
				for(signalsIndex=0;signalsIndex<clusters[clustersIndex].frames[framesIndex].numberOfSignals;signalsIndex++)
				{
					signalLength=clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].length;
					signalShift=clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].offset;
					if(clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].type==LIN_SIGNAL_TYPE_SCALER)
					{
						scalerSignal=scalerSignals[clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].ID];
						signalDataIndex=signalShift/8;
						signalAlignmentShift=signalShift%8;
						allFrames[allFramesIndex][signalDataIndex]|=(u8)(scalerSignal<<signalAlignmentShift);
						if(signalLength>8)
						{
							allFrames[allFramesIndex][signalDataIndex+1]=(u8)(scalerSignal>>(8-signalAlignmentShift));
						}
						scalerSignalsFrameIndexMap[clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].ID]=allFramesIndex;
					}
					else
					{
						byteArraySignal=byteArraySignals[clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].ID];
						for(signalDataIndex=0;signalDataIndex<signalLength;signalDataIndex++)
						{
							allFrames[allFramesIndex][signalDataIndex+signalShift]=byteArraySignal[signalDataIndex];
						}
						byteArraySignalsFrameIndexMap[clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].ID]=allFramesIndex;
					}
				}
			}
			allFramesIndex++;
		}
	}
}

LIN_ErrorType LIN_sendScalerSignal(u8 ID,u16 signal)
{
	LIN_ErrorType errorState;

	errorState=LIN_ERROR_OK;
	if(ID<NUMBER_OF_SCALER_SIGNALS)
	{
		if(scalerSignals[ID]!=signal)
		{
			allFramesUpdateFlag[scalerSignalsFrameIndexMap[ID]]=UPDATED;
			scalerSignals[ID]=signal;
		}
	}
	else
	{
		errorState=LIN_ERROR_NOK;
	}
	return errorState;
}

LIN_ErrorType LIN_sendByteArraySignal(u8 ID,u8 length,u8* signal)
{
	LIN_ErrorType errorState;
	u8 bytesIndex;

	errorState=LIN_ERROR_OK;
	if(ID<NUMBER_OF_BYTE_ARRAY_SIGNALS)
	{
		for(bytesIndex=0;bytesIndex<length;bytesIndex++)
		{
			if((byteArraySignals[ID])[bytesIndex]!=signal[bytesIndex])
			{
				allFramesUpdateFlag[byteArraySignalsFrameIndexMap[ID]]=UPDATED;
				(byteArraySignals[ID])[bytesIndex]=signal[bytesIndex];
			}
		}
	}
	else
	{
		errorState=LIN_ERROR_NOK;
	}
	return errorState;
}

LIN_ErrorType LIN_receiveScalerSignal(u8 ID,u16* signal)
{
	LIN_ErrorType errorState;

	errorState=LIN_ERROR_OK;
	if(ID<NUMBER_OF_SCALER_SIGNALS)
	{
		*signal=scalerSignals[ID];
	}
	else
	{
		errorState=LIN_ERROR_NOK;
	}
	return errorState;
}

LIN_ErrorType LIN_receiveByteArraySignal(u8 ID,u8 length,u8* signal)
{
	LIN_ErrorType errorState;
	u8 bytesIndex;

	errorState=LIN_ERROR_OK;
	if(ID<NUMBER_OF_BYTE_ARRAY_SIGNALS)
	{
		for(bytesIndex=0;bytesIndex<length;bytesIndex++)
		{
			signal[bytesIndex]=(byteArraySignals[ID])[bytesIndex];
		}
	}
	else
	{
		errorState=LIN_ERROR_NOK;
	}
	return errorState;
}

void mainRunnable(void)
{
	u8 			framesIndex,clustersIndex,allFramesIndex,signalsIndex,signalDataIndex;
	u8			signalShift,signalLength,signalAlignmentShift;
	u8			tempFrame[MAXIMUM_FRAME_SIZE];
	u16			scalerSignal;
	const	u8*	byteArraySignal;
	static 	u8 	ticksToSendHeader[NUMBER_OF_CLUSTERS];

	/*Slave task*/
	allFramesIndex=0;
	for(clustersIndex=0;clustersIndex<NUMBER_OF_CLUSTERS;clustersIndex++)
	{
		for(framesIndex=0;framesIndex<clusters[clustersIndex].numberOfFrames;framesIndex++)
		{
			if(clusters[clustersIndex].frames[framesIndex].direction!=LIN_FRAME_DIRECTION_NONE)
			{
				if((clusters[clustersIndex].frames[framesIndex].direction==LIN_FRAME_DIRECTION_PUBLISHER)&&(allFramesUpdateFlag[allFramesIndex]==UPDATED))
				{
					*((u64*)tempFrame)=(u64)0;
					for(signalsIndex=0;signalsIndex<clusters[clustersIndex].frames[framesIndex].numberOfSignals;signalsIndex++)
					{
						signalLength=clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].length;
						signalShift=clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].offset;
						if(clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].type==LIN_SIGNAL_TYPE_SCALER)
						{
							scalerSignal=((u16)((u16)(scalerSignals[clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].ID]<<(MAXIMUM_SCALER_SGNAL_SIZE-signalLength))>>(MAXIMUM_SCALER_SGNAL_SIZE-signalLength)));
							signalDataIndex=signalShift/8;
							signalAlignmentShift=signalShift%8;
							tempFrame[signalDataIndex]|=(u8)(scalerSignal<<signalAlignmentShift);
							if(signalLength>8)
							{
								tempFrame[signalDataIndex+1]=(u8)(scalerSignal>>(8-signalAlignmentShift));
							}
						}
						else
						{
							byteArraySignal=byteArraySignals[clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].ID];
							for(signalDataIndex=0;signalDataIndex<signalLength;signalDataIndex++)
							{
								tempFrame[signalDataIndex+signalShift]=byteArraySignal[signalDataIndex];
							}
						}
					}
					enterCriticalSection(clustersIndex);
					if(allFramesState[allFramesIndex]==NOT_BUSY)
					{
						*((u64*)allFrames[allFramesIndex])=*((u64*)tempFrame);
						if(clusters[clustersIndex].frames[framesIndex].type==LIN_FRAME_TYPE_UNCONDITIONAL)
						{
							allFramesUpdateFlag[allFramesIndex]=NOT_UPDATED;
						}
						else if(clusters[clustersIndex].frames[framesIndex].type==LIN_FRAME_TYPE_EVENT_TRIG)
						{
							allFramesUpdateFlag[allFramesIndex]=EVENT_TRIG_READY_FOR_PUBLISH;
						}
					}
					exitCriticalSection(clustersIndex);
				}
				else if((clusters[clustersIndex].frames[framesIndex].direction==LIN_FRAME_DIRECTION_SUBSCRIBER)&&(allFramesUpdateFlag[allFramesIndex]!=UPDATED))
				{
					enterCriticalSection(clustersIndex);
					if(allFramesState[allFramesIndex]==NOT_BUSY)
					{
						*((u64*)tempFrame)=*((u64*)allFrames[allFramesIndex]);
						allFramesUpdateFlag[allFramesIndex]=UPDATED;
					}
					exitCriticalSection(clustersIndex);
					if(allFramesUpdateFlag[allFramesIndex]==UPDATED)
					{
						for(signalsIndex=0;signalsIndex<clusters[clustersIndex].frames[framesIndex].numberOfSignals;signalsIndex++)
						{
							signalLength=clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].length;
							signalShift=clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].offset;
							if(clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].type==LIN_SIGNAL_TYPE_SCALER)
							{
								signalDataIndex=signalShift/8;
								signalAlignmentShift=signalShift%8;
								scalerSignal=0;
								if(signalLength<=8)
								{
									scalerSignal|=(u16)((tempFrame[signalDataIndex]<<(BYTE_SIZE-signalAlignmentShift-signalLength))>>(BYTE_SIZE-signalLength));
								}
								else
								{
									scalerSignal|=(u16)(tempFrame[signalDataIndex]>>signalAlignmentShift);
									scalerSignal|=(u16)(((u16)(tempFrame[signalDataIndex+1]<<(MAXIMUM_SCALER_SGNAL_SIZE-signalAlignmentShift-signalLength)))<<(signalLength-BYTE_SIZE));
								}
								scalerSignals[clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].ID]=scalerSignal;
							}
							else
							{
								for(signalDataIndex=0;signalDataIndex<signalLength;signalDataIndex++)
								{
									byteArraySignals[clusters[clustersIndex].frames[framesIndex].signals[signalsIndex].ID][signalDataIndex]=tempFrame[signalDataIndex+signalShift];
								}
							}
						}
					}
				}
			}
			allFramesIndex++;
		}
	}
	/*Master task*/
	allFramesIndex=0;
	for(clustersIndex=0;clustersIndex<NUMBER_OF_CLUSTERS;clustersIndex++)
	{
		if(clusters[clustersIndex].masterTaskControl==LIN_CLUSTER_MASTER_TASK_CONTROL_ENABLE)
		{
			for(framesIndex=0;framesIndex<clusters[clustersIndex].numberOfFrames;framesIndex++)
			{
				if(clusters[clustersIndex].frames[framesIndex].type==LIN_FRAME_TYPE_SPORADIC)
				{
					if((ticksToSendHeader[clustersIndex]==0)&&(allFramesUpdateFlag[allFramesIndex]==UPDATED))
					{
						allFramesUpdateFlag[allFramesIndex]=NOT_UPDATED;
						ticksToSendHeader[clustersIndex]=clusters[clustersIndex].frames[framesIndex].frameSlotTimeTicks;
						headerSendBuffer[clustersIndex][PID_BUFFER_INDEX]=PIDs[allFramesIndex];
						USART_sendLinBreak(clusters[clustersIndex].UART_ID);
						USART_send(clusters[clustersIndex].UART_ID,headerSendBuffer[clustersIndex],HEADER_SIZE,NULL);
					}
				}
				else
				{
					if((ticksToSendHeader[clustersIndex]==0)&&(allFramesTicksTosend[allFramesIndex]==0))
					{
						allFramesTicksTosend[allFramesIndex]=clusters[clustersIndex].frames[framesIndex].periodicity;
						ticksToSendHeader[clustersIndex]=clusters[clustersIndex].frames[framesIndex].frameSlotTimeTicks;
						headerSendBuffer[clustersIndex][PID_BUFFER_INDEX]=PIDs[allFramesIndex];
						USART_sendLinBreak(clusters[clustersIndex].UART_ID);
						USART_send(clusters[clustersIndex].UART_ID,headerSendBuffer[clustersIndex],HEADER_SIZE,NULL);
					}
					if(allFramesTicksTosend[allFramesIndex]!=0)
					{
						allFramesTicksTosend[allFramesIndex]--;
					}
				}
				if(ticksToSendHeader[clustersIndex]!=0)
				{
					ticksToSendHeader[clustersIndex]--;
				}
				allFramesIndex++;
			}
		}
		else
		{
			allFramesIndex=(u8)(allFramesIndex+clusters[clustersIndex].numberOfFrames);
		}
	}
}

u8	checksumCalcuklator(u8*data,u8 length,u8 PID)
{
	u8 checksum,dataIndex;
	u16 checksumWithCarry;

	checksum=0;
	for(dataIndex=0;dataIndex<length;dataIndex++)
	{
		checksumWithCarry=(u16)((u16)checksum+(u16)data[dataIndex]);
		checksum=(u8)checksumWithCarry;
		checksum=(u8)(checksum+((u8)(checksumWithCarry>>BYTE_SIZE)));
	}
	checksumWithCarry=(u16)((u16)checksum+(u16)PID);
	checksum=(u8)checksumWithCarry;
	checksum=(u8)(checksum+((u8)(checksumWithCarry>>BYTE_SIZE)));
	return checksum;
}

void linBreakHandler0(void)
{
	generalLinBreakHandler(CLUSTER_INDEX_0);
}

void linBreakHandler1(void)
{
	generalLinBreakHandler(CLUSTER_INDEX_1);
}

void linBreakHandler2(void)
{
	generalLinBreakHandler(CLUSTER_INDEX_2);
}


void generalLinBreakHandler(u8 clusterIndex)
{
	u8	framesIndex;
	if(slaveTaskState!=IDLE)
	{
		USART_stopProcess(clusters[clusterIndex].UART_ID);
		for(framesIndex=clustersAllFramesMarkers[clusterIndex];framesIndex<(clusters[clusterIndex].numberOfFrames+clustersAllFramesMarkers[clusterIndex]);framesIndex++)
		{
			allFramesState[framesIndex]=NOT_BUSY;
		}
	}
	slaveTaskState[clusterIndex]=RECEIVING_HEADER;
	USART_receive(clusters[clusterIndex].UART_ID,headerReceiveBuffer[clusterIndex],HEADER_SIZE,receiveHandlerMap[clusterIndex]);
}

void receiveHandler0(void)
{
	generalReceiveHandler(CLUSTER_INDEX_0);
}

void receiveHandler1(void)
{
	generalReceiveHandler(CLUSTER_INDEX_1);
}

void receiveHandler2(void)
{
	generalReceiveHandler(CLUSTER_INDEX_2);
}

void generalReceiveHandler(u8 clusterIndex)
{
	u8 framesIndex,allFramesIndex;
	if(slaveTaskState[clusterIndex]==RECEIVING_HEADER)
	{
		if(headerReceiveBuffer[clusterIndex][SYNC_BUFFER_INDEX]==SYNC_BYTE)
		{
			allFramesIndex=clustersAllFramesMarkers[clusterIndex];
			slaveTaskState[clusterIndex]=IDLE;
			for(framesIndex=0;framesIndex<clusters[clusterIndex].numberOfFrames;framesIndex++,allFramesIndex++)
			{
				if(PIDs[allFramesIndex]==headerReceiveBuffer[clusterIndex][PID_BUFFER_INDEX])
				{
					if(clusters[clusterIndex].frames[framesIndex].direction==LIN_FRAME_DIRECTION_PUBLISHER)
					{
						if(clusters[clusterIndex].frames[framesIndex].type==LIN_FRAME_TYPE_EVENT_TRIG)
						{
							if(allFramesUpdateFlag[allFramesIndex]==EVENT_TRIG_READY_FOR_PUBLISH)
							{
								allFramesState[allFramesIndex]=BUSY;
								allFrames[allFramesIndex][clusters[clusterIndex].frames[framesIndex].frameSize]=checksumCalcuklator(allFrames[allFramesIndex],clusters[clusterIndex].frames[framesIndex].frameSize,PIDs[allFramesIndex]);
								slaveTaskState[clusterIndex]=SENDING_DATA_CHECKSUM;
								currentBusyFrame[clusterIndex]=allFramesIndex;
								USART_send(clusters[clusterIndex].UART_ID,allFrames[allFramesIndex],(u16)(clusters[clusterIndex].frames[framesIndex].frameSize+CHECKSUM_SIZE),sendHandlerMap[clusterIndex]);
							}
						}
						else
						{
							allFramesState[allFramesIndex]=BUSY;
							allFrames[allFramesIndex][clusters[clusterIndex].frames[framesIndex].frameSize]=checksumCalcuklator(allFrames[allFramesIndex],clusters[clusterIndex].frames[framesIndex].frameSize,PIDs[allFramesIndex]);
							slaveTaskState[clusterIndex]=SENDING_DATA_CHECKSUM;
							currentBusyFrame[clusterIndex]=allFramesIndex;
							USART_send(clusters[clusterIndex].UART_ID,allFrames[allFramesIndex],(u16)(clusters[clusterIndex].frames[framesIndex].frameSize+CHECKSUM_SIZE),sendHandlerMap[clusterIndex]);
						}
					}
					else if(clusters[clusterIndex].frames[framesIndex].direction==LIN_FRAME_DIRECTION_SUBSCRIBER)
					{
						allFramesState[allFramesIndex]=BUSY;
						slaveTaskState[clusterIndex]=RECEIVING_DATA_CHECKSUM;
						currentBusyFrame[clusterIndex]=allFramesIndex;
						USART_receive(clusters[clusterIndex].UART_ID,allFrames[allFramesIndex],(u16)(clusters[clusterIndex].frames[framesIndex].frameSize+CHECKSUM_SIZE),receiveHandlerMap[clusterIndex]);
					}
				}
				break;
			}
		}
	}
	else if(slaveTaskState[clusterIndex]==RECEIVING_DATA_CHECKSUM)
	{
		if(checksumCalcuklator(allFrames[currentBusyFrame[clusterIndex]],clusters[clusterIndex].frames[currentBusyFrame[clusterIndex]-clustersAllFramesMarkers[clusterIndex]].frameSize,PIDs[currentBusyFrame[clusterIndex]])==allFrames[currentBusyFrame[clusterIndex]][clusters[clusterIndex].frames[currentBusyFrame[clusterIndex]-clustersAllFramesMarkers[clusterIndex]].frameSize])
		{
			allFramesUpdateFlag[currentBusyFrame[clusterIndex]]=UPDATED;
		}
		allFramesState[currentBusyFrame[clusterIndex]]=NOT_BUSY;
	}
}

void sendHandler0(void)
{
	generalSendHandler(CLUSTER_INDEX_0);
}

void sendHandler1(void)
{
	generalSendHandler(CLUSTER_INDEX_1);
}

void sendHandler2(void)
{
	generalSendHandler(CLUSTER_INDEX_2);
}

void generalSendHandler(u8 clusterIndex)
{
	if(slaveTaskState[clusterIndex]==SENDING_DATA_CHECKSUM)
	{
		if(allFramesUpdateFlag[currentBusyFrame[clusterIndex]]==EVENT_TRIG_READY_FOR_PUBLISH)
		{
			allFramesUpdateFlag[currentBusyFrame[clusterIndex]]=NOT_UPDATED;
		}
		allFramesState[currentBusyFrame[clusterIndex]]=NOT_BUSY;
	}
}

static void enterCriticalSection(u8 clusterIndex)
{
	if(clusters[clusterIndex].UART_asyncMode==USART_INTERRUPT_ASYNC_MODE)
	{
		NVIC_extIntControl(USART_InterruptNumber[clusters[clusterIndex].UART_ID],NVIC_DISABLE);
	}
	else
	{
		NVIC_extIntControl(DMA_NVIC_NUMBER(DMA_RX_Channels[clusters[clusterIndex].UART_ID]),NVIC_DISABLE);
		NVIC_extIntControl(DMA_NVIC_NUMBER(DMA_TX_Channels[clusters[clusterIndex].UART_ID]),NVIC_DISABLE);
	}
}

static void exitCriticalSection(u8 clusterIndex)
{
	if(clusters[clusterIndex].UART_asyncMode==USART_INTERRUPT_ASYNC_MODE)
	{
		NVIC_extIntControl(USART_InterruptNumber[clusters[clusterIndex].UART_ID],NVIC_ENABLE);
	}
	else
	{
		NVIC_extIntControl(DMA_NVIC_NUMBER(DMA_RX_Channels[clusters[clusterIndex].UART_ID]),NVIC_ENABLE);
		NVIC_extIntControl(DMA_NVIC_NUMBER(DMA_TX_Channels[clusters[clusterIndex].UART_ID]),NVIC_ENABLE);
	}
}
