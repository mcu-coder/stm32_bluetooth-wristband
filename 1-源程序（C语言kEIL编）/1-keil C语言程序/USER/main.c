#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "timer.h"
#include "adc.h"
#include "adxl345.h"
#include "Pedometer.h"
#include "stmflash.h"
#include "OLED_I2C.h"
#include "ds18b20.h"
#include "rtc.h"
#include "key.h"

#include "string.h" 	
/************************************************

************************************************/


extern _Bool Timer_Flag ;			//时间到 标准位
extern _Bool update_flag;			//更新标志变量


//要写入到STM32 FLASH的字符串数组
u8 TEXT_Buffer[]={"0000000"};
#define SIZE sizeof(TEXT_Buffer)	 	//数组长度
//#define FLASH_SAVE_ADDR  0X08020000 	//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)
#define FLASH_SAVE_ADDR  0X0800f400 	//设置FLASH 保存地址(必须为偶数，且其值要大于本代码所占用FLASH的大小+0X08000000)



void Dis_Init(void)
{
	OLED_ShowCN(0,0,10);			//心率
	OLED_ShowCN(16,00,11);

	OLED_ShowStr(32,0,":---r/min",2);
 
}
unsigned char Dis_mode = 0;		//显示状态标志 0：显示传感器数据  1：显示日期时间


short x, y, z;
int main(void)
{	

	unsigned char p[16]=" ";

	u8 datatemp[SIZE];	
	unsigned int  STEP=0;			//步数临时替换值			
	_Bool Heart_OK = 0;				//读取到正确心率标志位
	unsigned char Heart = 0;		//心率值
	short temperature = 0; 				//温度值
	
	STEPS_DIS = 1;
	delay_init();	    			//延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_3);//设置中断优先级分组为组3：2位抢占优先级，2位响应优先级
	
    LED_Init();		  				//初始化与控制设备连接的硬件接口
	OLED_Init();					//OLED初始化
	delay_ms(50);
	OLED_CLS();						//清屏
	 

 
 
	while(DS18B20_Init())	//DS18B20初始化	
	{
		OLED_ShowStr(0,0,"DS18B20 Error",2);
		
		delay_ms(200);
		OLED_ShowStr(0,0,"             ",2);	

		delay_ms(200);
	}
	delay_ms(100);
	while(DS18B20_Get_Temp()==850);	//DS18B20刚上电时候 读取的值是850 这里等待 直到不是850才开始下一步		
	delay_ms(5000);
	
	
	OLED_CLS();						//清屏
	Adc_Init();
 
	
	uart_init(9600);	 			//串口一初始化为9600
	TIM2_Int_Init(199,7199);		//10Khz的计数频率，计数到500为20ms 
	KEY_Init();						//IO初始化		
	EXTIX_Init();					//外部中断初始化
	
	OLED_CLS();						//清屏 
	Dis_Init();						
 
	
//	RTC_Set(2019,8,5,20,43,55);  //设置时间
 	while(1)
	{	
		

		if(Timer_Flag==1)					//500ms到 读取数据
		{
			Timer_Flag = 0;					//清除标志
		 	
			temperature=DS18B20_Get_Temp();	//读取温度
			TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); 						//使能指定的TIM3中断,允许更新中断
			TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE ); 						//使能指定的TIM3中断,允许更新中断

			TIM_Cmd(TIM2, ENABLE);  		//使能TIMx	
		 
		}
				
		if(ADXL345_FLAG==1)					//20ms到？
		{
			ADXL345_FLAG = 0;	  			//清除标志位
			step_counter();	
		}
//		printf("三轴加速器: %d \r\n",ADXL345_FLAG);	//串口发送出去

		delay_ms(20);	
		 
		Key_set();							//时间设置
	
		if(KEY0==0)							//KEY0切换显示按键
		{
			while(KEY0==0);					//等待松开
			OLED_CLS();						//清屏	
			if(Dis_mode==0)					//如果上一次是显示心率、步数、体温 的 则显示时间
				Dis_mode = 1;
			else 							//如果上一次不是显示心率、步数、体温 的 则心率、步数、体温
			{
				Dis_mode = 0;				
				Dis_Init();					//显示汉字信息
			}		
		}
		 
	
		if(update_flag==1) 					//2S标志到 发送一次数据到手机APP
		{
			update_flag = 0;
			printf("Step:%5d H:%3d T:%4.1f \r\n",STEPS,Heart,(float)temperature/10);	//串口发送出去

		}
	
		switch(Dis_mode)
		{
			case 0:	  										//显示传感器数据
				if(STEPS_DIS==1)							//刷新计步
				{
					sprintf((char*)p,":%-5d ",STEPS);		//显示步数
					OLED_ShowStr(32,2,p,2);
				}
 				sprintf((char*)p,":%4.1f    ",(float)temperature/10);
				OLED_ShowStr(32,4,p,2);						//显示温度
				 
			break;

			case 1:											//显示时间 日期等信息
				RTC_Display();								//显示时钟
			break;
		}
		
	}	
}

