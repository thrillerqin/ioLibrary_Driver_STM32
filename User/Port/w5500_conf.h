
#ifndef __W5500_CONF_H_
#define __W5500_CONF_H_

#include <stdio.h> 
#include <string.h>

#include "bsp_config.h"
#include "bsp.h"
#include "utility.h"

void wiz_cs(uint8_t val);
void iinchip_csoff(void);
void iinchip_cson(void);
void SPI_CrisEnter(void);
void SPI_CrisExit(void);
void SPI_CS_Select(void);
void SPI_CS_Deselect(void);
void SPI_WriteByte(uint8_t TxData);
uint8_t SPI_ReadByte(void);
void reset_w5500(void);
uint8  IINCHIP_SpiSendData(uint8 dat);
//void IINCHIP_WRITE( uint32 addrbsb,  uint8 data);
//uint8 IINCHIP_READ(uint32 addrbsb);
uint16 wiz_write_buf(uint32 addrbsb,uint8* buf,uint16 len);
uint16 wiz_read_buf(uint32 addrbsb, uint8* buf,uint16 len);
void reboot(void);
    
#endif
