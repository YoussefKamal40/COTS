#ifndef LIN_CONFIG_H_
#define LIN_CONFIG_H_

/*
 * THeader_Nominal= 34 * TBit
 * TResponse_Nominal= 10 * (NData + 1) * TBit
 * TFrame_Nominal= THeader_Nominal + TResponse_Nominal
 * TFrame_Nominal= (34+10*(NData+1))*TBit
 * TFrame_Maximum=1.4*((34+10*(NData+1))*TBit)
 * TFrame_Maximum=1.4*((34+10*(NData+1))/BIT_RATE)
 * TFrame_Slot > jitter + TFrame_Maximum
 * */

#define NUMBER_OF_CLUSTER_0_FRAMES 	2
#define NUMBER_OF_FRAME_0_0_SIGNALS	2
#define FRAME_0_0_SIZE				4
#define FRAME_0_0_TIME_SLOT			2
#define NUMBER_OF_FRAME_0_1_SIGNALS	2
#define FRAME_0_1_SIZE				3
#define FRAME_0_1_TIME_SLOT			2

#define NUMBER_OF_CLUSTER_1_FRAMES 	2
#define NUMBER_OF_FRAME_1_0_SIGNALS	2
#define FRAME_1_0_SIZE				2
#define FRAME_1_0_TIME_SLOT			2
#define NUMBER_OF_FRAME_1_1_SIGNALS	2
#define FRAME_1_1_SIZE				8
#define	FRAME_1_1_TIME_SLOT			3

#define NUMBER_OF_CLUSTERS 					2
#define BASE_TIME							LIN_BASE_TIME_5ms
#define NUMBER_OF_FRAMES					4
#define NUMBER_OF_SCALER_SIGNALS			4
#define NUMBER_OF_BYTE_ARRAY_SIGNALS		4

#define SIGNAL0_ID	((u8)0)
#define SIGNAL1_ID	((u8)1)
#define SIGNAL2_ID	((u8)2)
#define SIGNAL3_ID	((u8)3)
#define SIGNAL4_ID	((u8)0)
#define SIGNAL5_ID	((u8)1)
#define SIGNAL6_ID	((u8)2)
#define SIGNAL7_ID	((u8)3)

#define SIGNAL4_LENGTH	((u8)3)
#define SIGNAL5_LENGTH	((u8)2)
#define SIGNAL6_LENGTH	((u8)1)
#define	SIGNAL7_LENGTH	((u8)7)

#endif /* LIN_CONFIG_H_ */
