#ifndef SYSTICK_INTERFACE_H_
#define SYSTICK_INTERFACE_H_

#define SYSTICK_CLK_SOURCE_EXT_REF 	((u8)0)
#define SYSTICK_CLK_SOURCE_FREE_RUN ((u8)1)

#define SYSTICK_ENABLE ((u8)1)
#define SYSTICK_DISABLE ((u8)0)

typedef void (*SYSTICK_CBF_type)(void);

void SYSTICK_init(SYSTICK_CBF_type callback,u32 preload,u8 clockSource);
void SYSTICK_deInit(void);
void SYSTICK_start(void);
u32 SYSTICK_current(void);

#endif /* SYSTICK_INTERFACE_H_ */
