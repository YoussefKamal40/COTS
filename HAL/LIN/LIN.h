#ifndef LIN_H_
#define LIN_H_

#define LIN_CLUSTER_MASTER_TASK_CONTROL_ENABLE 	((u8)1)
#define LIN_CLUSTER_MASTER_TASK_CONTROL_DISABLE ((u8)0)

#define LIN_FRAME_TYPE_UNCONDITIONAL			((u8)0)
#define LIN_FRAME_TYPE_EVENT_TRIG				((u8)1)
#define LIN_FRAME_TYPE_SPORADIC					((u8)2)/*Must be publisher*/

#define LIN_SIGNAL_TYPE_SCALER					((u8)0)
#define LIN_SIGNAL_TYPE_BYTE_ARRAY				((u8)1)

#define LIN_FRAME_DIRECTION_PUBLISHER			((u8)1)
#define LIN_FRAME_DIRECTION_SUBSCRIBER			((u8)2)
#define LIN_FRAME_DIRECTION_NONE				((u8)0)

#define LIN_ERROR_OK							((u8)0)
#define LIN_ERROR_NOK							((u8)1)

#define LIN_BASE_TIME_5ms						5
#define LIN_BASE_TIME_10ms						10

typedef struct LIN_signalConfigType
{
	u8 	ID;
	u8 	type;
	u8	length;
	u8	offset;
}LIN_signalConfigType;

typedef struct LIN_frameConfigType
{
	const LIN_signalConfigType* 	signals;
	u16								periodicity;
	u8 								ID;
	u8 								type;
	u8 								numberOfSignals;
	u8 								frameSlotTimeTicks;
	u8								frameSize;
	u8								direction;
	u8								firstDelay;
}LIN_frameConfigType;

typedef struct LIN_clusterConfigType
{
	const LIN_frameConfigType* 	frames;
	f32 						bitRate;
	u8 							UART_ID;
	u8							UART_asyncMode;
	u8 							masterTaskControl;
	u8 							numberOfFrames;
}LIN_clusterConfigType;

typedef u8 LIN_ErrorType;

void LIN_init(void);
LIN_ErrorType LIN_sendScalerSignal(u8 ID,u16 signal);
LIN_ErrorType LIN_sendByteArraySignal(u8 ID,u8 length,u8* signal);
LIN_ErrorType LIN_receiveScalerSignal(u8 ID,u16* signal);
LIN_ErrorType LIN_receiveByteArraySignal(u8 ID,u8 length,u8* signal);

#include "LIN_config.h"

#endif /* LIN_H_ */
