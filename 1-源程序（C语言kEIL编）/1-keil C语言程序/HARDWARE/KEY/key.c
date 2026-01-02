#include "key.h"
#include "sys.h" 
#include "delay.h"
#include "OLED_I2C.h"
#include "RTC.h"

	
#include "string.h"
#include "stdio.h"

//按键初始化函数
void KEY_Init(void) //IO初始化
{ 
 	GPIO_InitTypeDef GPIO_InitStructure;
 
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);//使能PORTB 时钟
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;//KEY0-4
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		//设置成上拉输入
 	GPIO_Init(GPIOB, &GPIO_InitStructure);				//初始化GPIO

}

extern unsigned char Dis_mode;		//显示状态标志 0：显示传感器数据  1：显示日期时间
	
void Key_set(void)
{
	unsigned char ic = 0;
	unsigned char disp[20];
	unsigned int syear = 0;
	
	unsigned char smonth,sday,shour,smin;		//临时变量
	syear = calendar.w_year;
	smonth = calendar.w_month;
	sday = calendar.w_date;
	shour = calendar.hour;
	smin = calendar.min;
	if(KEY1==0)
	{
		while(KEY1==0);				//等待释放
		OLED_CLS();						//清屏
		OLED_ShowCN(16*3,0,28);			//时间
		OLED_ShowCN(16*4,0,29);
		
		OLED_ShowCN(0,2,18);			//日期
		OLED_ShowCN(16,2,19);
		
		OLED_ShowCN(0,4,16);			//时间
		OLED_ShowCN(16,4,17);


		while(1)
		{
			if(KEY1==0)
			{
				while(KEY1==0);		//等待释放	
				ic++;
				if(ic>9)
					ic= 0;
			}

			if(ic==0)				//年设置
			{
				if(KEY2==0)
				{
					while(KEY2==0);
					syear++;
				}
				if(KEY3==0)
				{
					while(KEY3==0);
					syear--;
				}

				
				sprintf((char*)disp,">%d-%02d-%02d",syear,smonth,sday);			
				OLED_ShowStr(32,2,disp,2);

				sprintf((char*)disp,":%02d:%02d:%02d",shour,smin,0);
				OLED_ShowStr(32,4,disp,2);

			}
			if(ic==1)				//月设置
			{
				if(KEY2==0)
				{
					while(KEY2==0);				
					smonth++;
					if(smonth>12)
						smonth = 1;
				}
				if(KEY3==0)
				{
					while(KEY3==0);
					if(smonth>1)
						smonth--;
				}

				
				sprintf((char*)disp,":%d>%02d-%02d",syear,smonth,sday);			
				OLED_ShowStr(32,2,disp,2);

				sprintf((char*)disp,":%02d:%02d:%02d",shour,smin,0);
				OLED_ShowStr(32,4,disp,2);

							
			}
			if(ic==2)				//日设置
			{
				if(KEY2==0)
				{
					while(KEY2==0);				
					sday++;
					if(sday>31)
						sday = 1;
				}
				if(KEY3==0)
				{
					while(KEY3==0);
					if(sday>1)
						sday--;
				}
			
				
				sprintf((char*)disp,":%d-%02d>%02d",syear,smonth,sday);			
				OLED_ShowStr(32,2,disp,2);

				sprintf((char*)disp,":%02d:%02d:%02d",shour,smin,0);
				OLED_ShowStr(32,4,(u8 *)disp,2);	
				
			}
			if(ic==3)				//时设置
			{
				if(KEY2==0)
				{
					while(KEY2==0);				
					shour++;
					if(shour>23)
						shour = 0;
				}
				if(KEY3==0)
				{
					while(KEY3==0);
					if(shour>0)
						shour--;
				}
			
				
				sprintf((char*)disp,":%d-%02d-%02d",syear,smonth,sday);			
				OLED_ShowStr(32,2,(u8 *)disp,2);

				sprintf((char*)disp,">%02d:%02d:%02d",shour,smin,0);
				OLED_ShowStr(32,4,(u8 *)disp,2);

				
			}
			if(ic==4)				//分设置
			{
				if(KEY2==0)
				{
					while(KEY2==0);				
					smin++;
					if(smin>59)
						smin = 0;
				}
				if(KEY3==0)
				{
					while(KEY3==0);
					if(smin>0)
						smin--;
				}
				
				
				sprintf((char*)disp,":%d-%02d-%02d",syear,smonth,sday);			
				OLED_ShowStr(32,2,(u8 *)disp,2);

				sprintf((char*)disp,":%02d>%02d:%02d",shour,smin,0);
				OLED_ShowStr(32,4,(u8 *)disp,2);	
				
			}
			
			if(ic==5)				//退0出设置
			{
					
			
				RTC_Set(syear,smonth,sday,shour,smin,0);  //设置时间
				Dis_mode = 0;					//显示状态标志 0：显示传感器数据  1：显示日期时间
				OLED_CLS();						//清屏
				OLED_ShowCN(0,0,10);			//心率
				OLED_ShowCN(16,0,11);
				
				OLED_ShowStr(32,0,":---r/min",2);
				
				OLED_ShowCN(0,2,12);			//步数
				OLED_ShowCN(16,2,13);

				
				OLED_ShowCN(0,4,14);			//体温
				OLED_ShowCN(16,4,15);
				ic =0;
				break;
			}
		}
	}
}
	
