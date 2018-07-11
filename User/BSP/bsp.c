#include "bsp.h" 


// @brief 1 millisecond Tick Timer setting
void NVIC_configuration(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK);
//	SysTick_Config(72000);
	NVIC_InitStructure.NVIC_IRQChannel = SysTick_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // Highest priority
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}


/**
*@brief		配置W5500的GPIO接口
*@param		无
*@return	无
*/
void gpio_for_w5500_config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    
    SPI_FLASH_Init();       /*初始化STM32 SPI1接口*/

#ifdef STM32F103RC
    RCC_APB2PeriphClockCmd(W5500_RST_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(W5500_NCS_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* W5500模块之RESET引脚 */
    GPIO_InitStructure.GPIO_Pin = W5500_RST_GPIO_PIN;       /*选择要控制的GPIO引脚*/		 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       /*设置引脚速率为50MHz */		
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;        /*设置引脚模式为通用推挽输出*/	
    GPIO_Init(W5500_RST_GPIO_PORT, &GPIO_InitStructure);    /*调用库函数，初始化GPIO*/
    GPIO_SetBits(W5500_RST_GPIO_PORT, W5500_RST_GPIO_PIN);		

    /* W5500模块之NCS */
    GPIO_InitStructure.GPIO_Pin = W5500_NCS_GPIO_PIN;       /*选择要控制的GPIO引脚*/		 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       /*设置引脚速率为50MHz */		
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;        /*设置引脚模式为通用推挽输出*/	
    GPIO_Init(W5500_NCS_GPIO_PORT, &GPIO_InitStructure);    /*调用库函数，初始化GPIO*/
    GPIO_SetBits(W5500_NCS_GPIO_PORT, W5500_NCS_GPIO_PIN);  //w5500片选

    /*定义INT引脚*/	
//		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;           /*选择要控制的GPIO引脚*/		 
//		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   /*设置引脚速率为50MHz*/		
//		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;       /*设置引脚模式为通用推挽模拟上拉输入*/		
//		GPIO_Init(GPIOG, &GPIO_InitStructure);              /*调用库函数，初始化GPIO*/

    /* spi flash的片选，失效spi flash，以免spi冲突 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;               /*选择要控制的GPIO引脚*/		 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       /*设置引脚速率为50MHz */		
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;        /*设置引脚模式为通用推挽输出*/	
    GPIO_Init(GPIOB, &GPIO_InitStructure);                  /*调用库函数，初始化GPIO*/
    GPIO_SetBits(GPIOB, GPIO_Pin_1);	//qinbao数采仪的SPI FLASH片选置1
#endif
    
    return;
}


/// TIM2中断优先级配置
void TIM2_NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure; 
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;	  
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;	
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/*
 * TIM_Period / Auto Reload Register(ARR) = 1000   TIM_Prescaler--71 
 * 中断周期为 = 1/(72MHZ /72) * 1000 = 1ms
 *
 * TIMxCLK/CK_PSC --> TIMxCNT --> TIM_Period(ARR) --> 中断 且TIMxCNT重置为0重新计数 
 */
void TIM2_Configuration(void)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
		
		/* 设置TIM2CLK 为 72MHZ */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , ENABLE);
    //TIM_DeInit(TIM2);
	
	/* 自动重装载寄存器周期的值(计数值) */
    TIM_TimeBaseStructure.TIM_Period=1000;
	
    /* 累计 TIM_Period个频率后产生一个更新或者中断 */
	  /* 时钟预分频数为72 */
    TIM_TimeBaseStructure.TIM_Prescaler= 71;
	
		/* 对外部时钟进行采样的时钟分频,这里没有用到 */
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; 
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
	
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	
    TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
		
    TIM_Cmd(TIM2, ENABLE);																		
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 , DISABLE);		/*先关闭等待使用*/    
}


 /**
  * @brief  USART1 GPIO 配置,工作模式配置。9600 8-N-1
  * @param  无
  * @retval 无
  */
void USART1_Config(void)
{
		GPIO_InitTypeDef GPIO_InitStructure;
		USART_InitTypeDef USART_InitStructure;
		
		/* config USART1 clock */
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE);
		
		/* USART1 GPIO config */
		/* Configure USART1 Tx (PA.09) as alternate function push-pull */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_Init(GPIOA, &GPIO_InitStructure);    
		/* Configure USART1 Rx (PA.10) as input floating */
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
			
		/* USART1 mode config */
		USART_InitStructure.USART_BaudRate = 115200;
		USART_InitStructure.USART_WordLength = USART_WordLength_8b;
		USART_InitStructure.USART_StopBits = USART_StopBits_1;
		USART_InitStructure.USART_Parity = USART_Parity_No ;
		USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
		USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
		USART_Init(USART1, &USART_InitStructure); 
		USART_Cmd(USART1, ENABLE);
}


/* USART2 115200,8,N,1 */
void USART2_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;

    /* config USART2 clock */
    RCC_APB2PeriphClockCmd(DEBUG_COM_TX_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(DEBUG_COM_RX_GPIO_CLK, ENABLE);
    RCC_APB1PeriphClockCmd(DEBUG_COM_CLK, ENABLE);

    /* USART2 GPIO config */
    /* Configure USART2 Tx (PA.02) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = DEBUG_COM_TX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(DEBUG_COM_TX_GPIO_PORT, &GPIO_InitStructure);    
    /* Configure USART2 Rx (PA.03) as input floating */
    GPIO_InitStructure.GPIO_Pin = DEBUG_COM_RX_GPIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(DEBUG_COM_RX_GPIO_PORT, &GPIO_InitStructure);
        
    /* USART1 mode config */
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No ;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(DEBUG_COM_NAME, &USART_InitStructure); 
    USART_Cmd(DEBUG_COM_NAME, ENABLE);
}



///重定向c库函数printf到USART2
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到USART1 */
		USART_SendData(USART2, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);		
	
		return (ch);
}

///重定向c库函数scanf到USART1
int fgetc(FILE *f)
{
		/* 等待串口1输入数据 */
		while (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(USART2);
}


/*******************************************************************************
* Function Name  : SPI_FLASH_Init
* Description    : Initializes the peripherals used by the SPI FLASH driver.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SPI_FLASH_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;

  /* Enable SPI1 and GPIO clocks */
  RCC_APB2PeriphClockCmd(W5500_SPI_SCLK_GPIO_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(W5500_SPI_MISO_GPIO_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(W5500_SPI_MOSI_GPIO_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(W5500_NCS_GPIO_CLK, ENABLE);

  /*!< SPI_FLASH_SPI Periph clock enable */
  RCC_APB1PeriphClockCmd(W5500_SPI_CLK, ENABLE);
 
  
  /*!< Configure SPI_FLASH_SPI pins: SCK */
  GPIO_InitStructure.GPIO_Pin = W5500_SPI_SCLK_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(W5500_SPI_SCLK_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SPI_FLASH_SPI pins: MISO */
  GPIO_InitStructure.GPIO_Pin = W5500_SPI_MISO_GPIO_PIN;
  GPIO_Init(W5500_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SPI_FLASH_SPI pins: MOSI */
  GPIO_InitStructure.GPIO_Pin = W5500_SPI_MOSI_GPIO_PIN;
  GPIO_Init(W5500_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

  /*!< Configure SPI_FLASH_SPI_CS_PIN pin: SPI_FLASH Card CS pin */
  GPIO_InitStructure.GPIO_Pin = W5500_NCS_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_Init(W5500_NCS_GPIO_PORT, &GPIO_InitStructure);

  /* Deselect the FLASH: Chip Select high */
//  SPI_FLASH_CS_HIGH();

  /* SPI1 configuration */
  // W25X16: data input on the DIO pin is sampled on the rising edge of the CLK. 
  // Data on the DO and DIO pins are clocked out on the falling edge of CLK.
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(W5500_SPI_NAME, &SPI_InitStructure);
  SPI_Cmd(W5500_SPI_NAME, ENABLE);
  /* Enable SPI  */
//  SPI_Cmd(SPI2, ENABLE);
}



/*******************************************************************************
* Function Name  : SPI_FLASH_ReadByte
* Description    : Reads a byte from the SPI Flash.
*                  This function must be used only if the Start_Read_Sequence
*                  function has been previously called.
* Input          : None
* Output         : None
* Return         : Byte Read from the SPI Flash.
*******************************************************************************/
u8 SPI_FLASH_ReadByte(void)
{
  return (SPI_FLASH_SendByte(Dummy_Byte));
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendByte
* Description    : Sends a byte through the SPI interface and return the byte
*                  received from the SPI bus.
* Input          : byte : byte to send.
* Output         : None
* Return         : The value of the received byte.
*******************************************************************************/
u8 SPI_FLASH_SendByte(u8 byte)
{
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);   //qinbao

  /* Send byte through the SPI1 peripheral */
  SPI_I2S_SendData(SPI2, byte); //qinbao

  /* Wait to receive a byte */
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);  //qinbao

  /* Return the byte read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI2); //qinbao
}

/*******************************************************************************
* Function Name  : SPI_FLASH_SendHalfWord
* Description    : Sends a Half Word through the SPI interface and return the
*                  Half Word received from the SPI bus.
* Input          : Half Word : Half Word to send.
* Output         : None
* Return         : The value of the received Half Word.
*******************************************************************************/
u16 SPI_FLASH_SendHalfWord(u16 HalfWord)
{
  /* Loop while DR register in not emplty */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

  /* Send Half Word through the SPI1 peripheral */
  SPI_I2S_SendData(SPI1, HalfWord);

  /* Wait to receive a Half Word */
  while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);

  /* Return the Half Word read from the SPI bus */
  return SPI_I2S_ReceiveData(SPI1);
}

void SPI_Delay(__IO uint32_t nCount)
{
    for(; nCount != 0; nCount--);
}
