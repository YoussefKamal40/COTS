#ifndef CHIPUSARTHANDLER_CONFIG_H_
#define CHIPUSARTHANDLER_CONFIG_H_

#pragma pack(4)
typedef struct ChipUSARTHandler_configType
{
	USARTConfig_Type USARTConfig;
	u8 configMode;
	/*u8 connection;*/
}ChipUSARTHandler_configType;

#define NUMBER_OF_CHIPS 1

#endif /* CHIPUSARTHANDLER_CONFIG_H_ */
