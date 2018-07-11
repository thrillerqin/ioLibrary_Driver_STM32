/*
**************************************************************************************************
* @file    		w5500_conf.c
* @author  		WIZnet Software Team 
* @version 		V1.0
* @date    		2015-02-14
* @brief  		配置MCU，移植W5500程序需要修改的文件，配置W5500的MAC和IP地址
**************************************************************************************************
*/
#include "w5500_conf.h"

uint8 dhcp_ok=0;                /*dhcp成功获取IP*/
uint32	ms=0;                   /*毫秒计数*/
uint32	dhcp_time= 0;           /*DHCP运行计数*/
vu8	ntptimer = 0;               /*NPT秒计数*/


/**
*@brief		W5500片选信号设置函数
*@param		val: 为“0”表示片选端口为低，为“1”表示片选端口为高
*@return	无
*/
void wiz_cs(uint8_t val)
{
	if (val == LOW) 
	{
		GPIO_ResetBits(W5500_NCS_GPIO_PORT, W5500_NCS_GPIO_PIN); 
	}
	else if (val == HIGH)
	{
		GPIO_SetBits(W5500_NCS_GPIO_PORT, W5500_NCS_GPIO_PIN); 
	}
}

/**
*@brief		设置W5500的片选端口SCSn为低
*@param		无
*@return	无
*/
void iinchip_csoff(void)
{
	wiz_cs(LOW);
}

/**
*@brief		设置W5500的片选端口SCSn为高
*@param		无
*@return	无
*/
void iinchip_cson(void)
{	
   wiz_cs(HIGH);
}



/**
* @brief? 进入临界区
* @retval None
*/
void SPI_CrisEnter(void)
{
    __set_PRIMASK(1);
}

/**
* @brief? 退出临界区
* @retval None
*/
void SPI_CrisExit(void)
{
    __set_PRIMASK(0);
}

/**
* @brief? 片选信号输出低电平
* @retval None
*/
void SPI_CS_Select(void)
{
    wiz_cs(LOW);
}


/**
* @brief 片选信号输出高电平
* @retval None
*/
void SPI_CS_Deselect(void)
{
    wiz_cs(HIGH);
}


/**
  * @brief  写1字节数据到SPI总线
  * @param  TxData 写到总线的数据
  * @retval None
  */
void SPI_WriteByte(uint8_t TxData)
{                
     SPI_FLASH_SendByte(TxData);
}


/**
  * @brief  从SPI总线读取1字节数据
  * @retval 读到的数据
  */
uint8_t SPI_ReadByte(void)
{            
    uint8_t data = SPI_FLASH_ReadByte();
    return data;
}


/**
*@brief		W5500复位设置函数
*@param		无
*@return	无
*/
void reset_w5500(void)
{
/* 数据采集仪 */
#ifdef  STM32F103RC 
    GPIO_ResetBits(W5500_RST_GPIO_PORT, W5500_RST_GPIO_PIN);
    delay_us(2);
    GPIO_SetBits(W5500_RST_GPIO_PORT, W5500_RST_GPIO_PIN);
    delay_ms(1600);
#endif
}

/**
*@brief		STM32 SPI1读写8位数据
*@param		dat：写入的8位数据
*@return	无
*/
uint8  IINCHIP_SpiSendData(uint8 dat)
{
   return(SPI_FLASH_SendByte(dat));
}

///**
//*@brief		写入一个8位数据到W5500
//*@param		addrbsb: 写入数据的地址
//*@param   data：写入的8位数据
//*@return	无
//*/
//void IINCHIP_WRITE( uint32 addrbsb,  uint8 data)
//{
//   iinchip_csoff();                              		
//   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);	
//   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
//   IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + 4);  
//   IINCHIP_SpiSendData(data);                   
//   iinchip_cson();                            
//}

///**
//*@brief		从W5500读出一个8位数据
//*@param		addrbsb: 写入数据的地址
//*@param   data：从写入的地址处读取到的8位数据
//*@return	无
//*/
//uint8 IINCHIP_READ(uint32 addrbsb)
//{
//   uint8 data = 0;
//   iinchip_csoff();                            
//   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
//   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
//   IINCHIP_SpiSendData( (addrbsb & 0x000000F8))    ;
//   data = IINCHIP_SpiSendData(0x00);            
//   iinchip_cson();                               
//   return data;    
//}

/**
*@brief		向W5500写入len字节数据
*@param		addrbsb: 写入数据的地址
*@param   buf：写入字符串
*@param   len：字符串长度
*@return	len：返回字符串长度
*/
uint16 wiz_write_buf(uint32 addrbsb,uint8* buf,uint16 len)
{
   uint16 idx = 0;
   if(len == 0) printf("Unexpected2 length 0\r\n");
   iinchip_csoff();                               
   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
   IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + 4); 
   for(idx = 0; idx < len; idx++)
   {
     IINCHIP_SpiSendData(buf[idx]);
   }
   iinchip_cson();                           
   return len;  
}


/**
*@brief		从W5500读出len字节数据
*@param		addrbsb: 读取数据的地址
*@param 	buf：存放读取数据
*@param		len：字符串长度
*@return	len：返回字符串长度
*/
uint16 wiz_read_buf(uint32 addrbsb, uint8* buf,uint16 len)
{
  uint16 idx = 0;
  if(len == 0)
  {
    printf("Unexpected2 length 0\r\n");
  }
  iinchip_csoff();                                
  IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);
  IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);
  IINCHIP_SpiSendData( (addrbsb & 0x000000F8));    
  for(idx = 0; idx < len; idx++)                   
  {
    buf[idx] = IINCHIP_SpiSendData(0x00);
  }
  iinchip_cson();                                  
  return len;
}

