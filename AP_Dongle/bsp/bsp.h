

#include "datatype.h"

void BSP_EnterCri(void);
void BSP_ExitCri(void);
void BSP_Delay1S(INT32 n);
void BSP_Delay1MS(INT32 n);
void BSP_Delay100US(INT32 n);
void BSP_Delay10US(INT32 n);
void BSP_Delay1US(INT32 n);
void BSP_Nop(void);
void BSP_Reboot(void);

void BSP_NVIC_Config(UINT32 table_offset);
void BSP_NVIC_SetUSBLP(UINT8 enable);
void BSP_NVIC_SetDebugCom(UINT8 enable);
void BSP_NVIC_SetALLTIM(UINT8 enable);

/* RCC functions */
void BSP_RCC_SetUSB(UINT8 enable);
void BSP_RCC_SetAllGPIO(UINT8 enable);
void BSP_RCC_SetAFIO(UINT8 enable);
void BSP_RCC_SetGPIOA(UINT8 enable);
void BSP_RCC_SetGPIOB(UINT8 enable);
void BSP_RCC_SetSPI2(UINT8 enable);
void BSP_RCC_SetDebugCom(UINT8 enable);
void BSP_RCC_SetALLTIM(UINT8 enable);

/* GPIO Config functions */
void BSP_GPIO_SetAIN(void);
void BSP_GPIO_DisableJTAG(void);
void BSP_GPIO_CfgSPI2(void);
void BSP_GPIO_CfgDebugCom(void);
void BSP_GPIO_SetRCI(UINT8 enable);
void BSP_GPIO_CfgUSBCable(void);
void BSP_GPIO_SetUSBCable(int enable);

void BSP_GPIO_CfgDebugPin(void);
void BSP_GPIO_ToggleDebugPin(void);

void BSP_SPI_CfgSPI2(void);

/* watchdog feed */
void WD_Feed(void);
void WD_Init(void);

/* bsp config */
void BSP_Config(void);
