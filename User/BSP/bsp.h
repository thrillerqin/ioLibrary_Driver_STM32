#ifndef __BSP_H_
#define __BSP_H_

#include <stdio.h>
#include "stm32f10x.h"
#include "bsp_config.h"

#define Dummy_Byte                    0xFF

void NVIC_configuration(void);
void gpio_for_w5500_config(void);

void TIM2_NVIC_Configuration(void);
void TIM2_Configuration(void);

void USART1_Config(void);
void USART2_Config(void);

void SPI_FLASH_Init(void);
u8 SPI_FLASH_ReadByte(void);
u8 SPI_FLASH_SendByte(u8 byte);
u16 SPI_FLASH_SendHalfWord(u16 HalfWord);
void SPI_Delay(__IO uint32_t nCount);


#endif	/* __BSP_H_ */







