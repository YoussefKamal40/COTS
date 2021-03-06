#ifndef USART_INTERFACE_H_
#define USART_INTERFACE_H_

#define USART_BAUDRATE_2400		((f32)2400)
#define USART_BAUDRATE_9600		((f32)9600)
#define USART_BAUDRATE_19200    ((f32)19200)
#define USART_BAUDRATE_57600    ((f32)57600)
#define USART_BAUDRATE_115200   ((f32)115200)

#define USART_PARITY_EVEN		2
#define USART_PARITY_ODD		1
#define USART_PARITY_DISABLE	0

#define USART_ID_USART1 ((u8)0)
#define USART_ID_USART2 ((u8)1)
#define USART_ID_USART3 ((u8)2)
#define USART_ID_UART4  ((u8)3)
#define USART_ID_UART5  ((u8)4)

#define USART_STOP_1_p_5 	((u32)0x3000)
#define USART_STOP_2 		((u32)0x2000)
#define USART_STOP_0_p_5 	((u32)0x1000)
#define USART_STOP_1 		((u32)0x0000)

#define USART_ERROR_OK		((u8)0)
#define USART_ERROR_NOK		((u8)1)

#define USART_INTERRUPT_ASYNC_MODE		0
#define USART_DMA_ASYNC_MODE			1

typedef u8 USART_ErrorType;

typedef void (*handlerNotifyType)(void);

#pragma pack(4)
typedef struct USARTConfig_Type
{
	u32 stopBit;
	f32 baudRate;
	u8 parity;
	u8 asyncMode;
	u8 ID;
}USARTConfig_Type;

void USART_init(u8 ID);
void USART_config(USARTConfig_Type* data);
USART_ErrorType USART_send(u8 ID,const u8* buffer,u16 length,handlerNotifyType notifyFunction);
USART_ErrorType USART_receive(u8 ID,u8* buffer,u16 length,handlerNotifyType notifyFunction);
void USART_setLinBreakCallback(u8 ID,handlerNotifyType callback);
void USART_sendLinBreak(u8 ID);
void USART_stopProcess(u8 ID);

#endif /* USART_INTERFACE_H_ */
