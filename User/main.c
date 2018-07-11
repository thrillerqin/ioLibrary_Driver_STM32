/**
******************************************************************************
* main.c
* 在HAPH数采仪2017上测试wiznet的官方库ioLibrary_Driver
* 2018年7月9日
******************************************************************************
*/ 
#include <stdio.h>
#include <string.h>
#include "stm32f10x.h"

#include "bsp.h"

#include "utility.h"
#include "bsp_config.h"

#include "wizchip_conf.h"
#include "w5500_conf.h"
#include "loopback.h"

//Include: Internet iolibrary
#include "MQTTClient.h"


#define TCP_SOCKET	    0   //Socket number defines
#define BUFFER_SIZE	    2048    //Receive Buffer Size define

//Global variables
unsigned char targetIP[4] = {192,168,1,103};    // mqtt server IP
unsigned int targetPort = 1883;                 // mqtt server port
wiz_NetInfo gWIZNETINFO;    // mac,ip,subnet,gw,dns,dhcp

unsigned char tempBuffer[BUFFER_SIZE];

/* fuction declaration */
void wiznet_register(void);
static void W5500_Network_Init(void);
void ChipParametersConfiguration(void);
void messageArrived(MessageData* md);
int mqtt_ex_main(void);


/* main entry */
int main(void)
{ 	
    systick_init(72);				            /*初始化Systick工作时钟*/
    NVIC_configuration();   //tick中断设定
    USART2_Config(); 				            /*初始化串口通信:115200@8-n-1*/

    gpio_for_w5500_config();	         	/*初始化MCU相关引脚*/
    reset_w5500();					    /*硬复位W5500*/
    
    wiznet_register();
    W5500_Network_Init();//network_init();
    ChipParametersConfiguration();

    printf(" W5500测试程序@HAPH数采仪2017 \r\n");
    printf(" 基于wiznet的官方库ioLibrary_Driver \r\n");
    
    mqtt_ex_main();

    while(1)                            /*循环执行的函数*/ 
    {
        //loopback_tcps(0,buffer,5000);
    }
}


void wiznet_register(void)
{
// First of all, Should register SPI callback functions implemented by user for accessing WIZCHIP 
    /* Critical section callback */
    reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);   //注册临界区函数
    /* Chip selection call back */
#if   _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_VDM_
    reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);//注册SPI片选信号函数
#elif _WIZCHIP_IO_MODE_ == _WIZCHIP_IO_MODE_SPI_FDM_
    reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);  // CS must be tried with LOW.
#else
    #if (_WIZCHIP_IO_MODE_ & _WIZCHIP_IO_MODE_SIP_) != _WIZCHIP_IO_MODE_SIP_
        #error "Unknown _WIZCHIP_IO_MODE_"
    #else
        reg_wizchip_cs_cbfunc(wizchip_select, wizchip_deselect);
    #endif
#endif
    /* SPI Read & Write callback function */
    reg_wizchip_spi_cbfunc(SPI_ReadByte, SPI_WriteByte);    //注册读写函数

    return;
}


/**
* @brief    : W5500_Network_Init
* @note	    : W5500 网络初始化
* @param    :
* @retval   :
*/
static void W5500_Network_Init(void)
{
    uint8_t chipid[6];
    uint8_t mac[6]={0x00,0x08,0xdc,0x11,0x11,0x11}; ///< Source Mac Address
    uint8_t ip[4]={192,168,0,88}; ///< Source IP Address
    uint8_t sn[4]={255,255,255,0}; ///< Subnet Mask
    uint8_t gw[4]={192,168,0,1}; ///< Gateway IP Address
    uint8_t dns[4]={114,114,114,114}; ///< DNS server IP Address

    memcpy(gWIZNETINFO.ip, ip, 4);
    memcpy(gWIZNETINFO.sn, sn, 4);
    memcpy(gWIZNETINFO.gw, gw, 4);
    memcpy(gWIZNETINFO.mac, mac,6);
    memcpy(gWIZNETINFO.dns,dns,4);
    
    gWIZNETINFO.dhcp = NETINFO_STATIC; //< 1 - Static, 2 - DHCP
    ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);

    ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
    // Display Network Information
    ctlwizchip(CW_GET_ID,(void*)chipid);
    printf("\r\n=== %s NET CONF ===\r\n",(char*)chipid);
    printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],
    gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
    printf("SIP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
    printf("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
    printf("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
    printf("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
    printf("======================\r\n");

    wizchip_init(NULL, NULL);
}


//初始化芯片参数
void ChipParametersConfiguration(void)
{
    uint8_t memsize[2][8] = {{2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};

    //WIZCHIP SOCKET缓存区初始化
    if(ctlwizchip(CW_INIT_WIZCHIP,(void*)memsize) == -1){
        printf("WIZCHIP Initialized fail.\r\n");
        while(1);
    }
}


// @brief messageArrived callback function
void messageArrived(MessageData* md)
{
	unsigned char testbuffer[100];
	MQTTMessage* message = md->message;
    
    printf("message received.\r\n");
    memcpy(testbuffer,(char*)message->payload,(int)message->payloadlen);
    *(testbuffer + (int)message->payloadlen + 1) = '\n';
    printf("%s\r\n",testbuffer);
}


int mqtt_ex_main(void)
{
	Network n;
	MQTTClient c;
    int rc = 0;
    unsigned char buf[100];

	NewNetwork(&n, TCP_SOCKET);
	ConnectNetwork(&n, (unsigned char*)targetIP, targetPort);
	MQTTClientInit(&c,&n,1000,buf,100,tempBuffer,2048);

	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = "stdout-subscriber";    //opts.clientid;
	data.username.cstring = ""; //opts.username;
	data.password.cstring = ""; //opts.password;

	data.keepAliveInterval = 60;
	data.cleansession = 1;

	rc = MQTTConnect(&c, &data);
	printf("Connected %d\r\n", rc);

	printf("Subscribing to %s\r\n", "hello/wiznet");
	rc = MQTTSubscribe(&c, "hello/wiznet", QOS0, messageArrived);
	printf("Subscribed %d\r\n", rc);

    MQTTMessage mqtt_msg;
    mqtt_msg.qos = QOS0;
    mqtt_msg.retained = 0;
    mqtt_msg.dup = 0;
    mqtt_msg.payload = "hello world!";
    mqtt_msg.payloadlen = sizeof("hello world!");

    while(1)
    {
        MQTTPublish(&c, "qinbao", &mqtt_msg);
        delay_s(5);
    	MQTTYield(&c, data.keepAliveInterval);
    }
}




