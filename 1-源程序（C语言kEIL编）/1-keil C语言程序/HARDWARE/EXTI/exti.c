 /******************************************************************************

 ******************************************************************************
*  文 件 名   : EXTI.c
EXTI外部中断相关程序
******************************************************************************/
#include "exti.h"
#include "led.h"
#include "adxl345.h"
#include "Pedometer.h"

//外部中断0服务程序
void EXTIX_Init(void)
{
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

    GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_7;			//PA7 外部中断输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		//设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);				//初始化GPIOA7 


  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

    //GPIOA.7 中断线以及中断初始化配置   下降沿触发
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource7);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line7;			// 
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 					//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器


  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//使能外部中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;		//子优先级3
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 
}

//外部中断7服务程序 
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line7)!=RESET)						//判断某个线上的中断是否发生
	{
		if(((ADXL345_Read_Single(INT_SOURCE)) & 0x80 ) == 0x80) 
		{
			interval++; 
		}	
		EXTI_ClearITPendingBit(EXTI_Line7); 					//清除LINE7上的中断标志位 
	}

}
 

