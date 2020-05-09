#ifndef DMA_INTERFACE_H_
#define DMA_INTERFACE_H_

#define DMA_CHANNEL_1 0
#define DMA_CHANNEL_2 1
#define DMA_CHANNEL_3 2
#define DMA_CHANNEL_4 3
#define DMA_CHANNEL_5 4
#define DMA_CHANNEL_6 5
#define DMA_CHANNEL_7 6

#define DMA_PERIORITY_LOW 		((u32)0x0)
#define DMA_PERIORITY_MEDIUM 	((u32)0x1000)
#define DMA_PERIORITY_HIGH 		((u32)0x2000)
#define DMA_PERIORITY_VERY_HIGH ((u32)0x3000)

#define DMA_MODE_CIRCULAR		((u8)1)
#define DMA_MODE_MEM2MEM		((u8)2)
#define DMA_MODE_NON_MEM2MEM	((u8)3)

#define DMA_MEM_ELEMENT_SIZE_8b		((u32)0x000)
#define DMA_MEM_ELEMENT_SIZE_16b	((u32)0x400)
#define DMA_MEM_ELEMENT_SIZE_32b	((u32)0x800)

#define DMA_PREP_ELEMENT_SIZE_8b	((u32)0x000)
#define DMA_PREP_ELEMENT_SIZE_16b	((u32)0x100)
#define DMA_PREP_ELEMENT_SIZE_32b	((u32)0x200)

#define DMA_ENABLE	((u8)1)
#define DMA_DISABLE	((u8)0)

#define DMA_DIR_MEM2PREP ((u8)1)
#define DMA_DIR_PREP2MEM ((u8)0)

typedef struct DMA_configType
{
	u32 periority;
	u32 memElementSize;
	u32 prepElementSize;
	u32 prepAddress;
	void (*completeNotify)(void);
	void (*halfCompleteNotify)(void);
	void (*errorNotify)(void);
	u8 mode;
	u8 memIncrement;
	u8 prepIncrement;
	u8 direction;
	u8 channel;
}DMA_configType;

void DMA_init(DMA_configType *config);
void DMA_start(u32 memAddress,u16 numberOfData,u8 ch);
void DMA_stop(u8 ch);

#endif /* DMA_INTERFACE_H_ */
