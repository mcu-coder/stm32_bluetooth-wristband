
#include "delay.h"
#include "timer.h"
#include "adc.h"
#include "led.h"

#include "Pedometer.h"

#include "string.h"

_Bool Timer_Flag = 0 ;		//0.5s时间到 标准位
_Bool update_flag =0;		//更新标志变量

void TIM2_Int_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); 			//时钟使能
	
	//定时器TIM3初始化
	TIM_TimeBaseStructure.TIM_Period = arr; 						//设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 						//设置用来作为TIMx时钟频率除数的预分频值
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; 		//设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  	//TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); 				//根据指定的参数初始化TIMx的时间基数单位
 
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE ); 						//使能指定的TIM3中断,允许更新中断

	//中断优先级NVIC设置
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  				//TIM3中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 		//先占优先级0级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;  			//从优先级3级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure); 								//初始化NVIC寄存器


	TIM_Cmd(TIM2, ENABLE);  //使能TIMx					 
}
//定时器2中断服务程序
void TIM2_IRQHandler(void)   //TIM2中断		20ms中断
{
	static unsigned char t = 0;
	static unsigned int Cnt = 0;
	static unsigned int Cnt1 = 0;
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) 				//检查TIM3更新中断发生与否
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  				//清除TIMx更新中断标志 
		ADXL345_FLAG = 1;				//赋予标志位 计步一次
		Cnt++;
		if(Cnt>=100)							//20ms*100 = 2s
		{
			Cnt = 0;
			update_flag = 1;	
		}
		if(Cnt1++ >= 10)    				//20ms*25 =0.5s
		{
			Cnt1 = 0;
			Timer_Flag = 1;	 				//2s到 赋值标志为1  	
		}
	}
}
/******************心率算法采集部分*********************/
#define true 1
#define false 0

	
int BPM;                   		 //脉搏率==就是心率
int Signal;               		 //传入的原始数据。
int IBI = 600;            		 //节拍间隔，两次节拍之间的时间（ms）。计算：60/IBI(s)=心率（BPM）
unsigned char Pulse = false;     //脉冲高低标志。当脉波高时为真，低时为假。
unsigned char QS = false;        //当发现一个节拍时，就变成了真实
int rate[10];                    //数组保存最后10个IBI值。
unsigned long sampleCounter = 0; //用于确定脉冲定时。
unsigned long lastBeatTime = 0;  //用于查找IBI
int P =512;                      //用于在脉冲波中寻找峰值
int T = 512;                     //用于在脉冲波中寻找波谷
int thresh = 512;                //用来寻找瞬间的心跳
int amp = 100;                   //用于保持脉冲波形的振幅
int Num;
unsigned char firstBeat = true;  //第一个脉冲节拍
unsigned char secondBeat = false;//第二个脉冲节拍，这两个变量用于确定两个节拍


/*
 * 函数名：TIM3_Int_Init
 * 描述  ：配置TIM3
 * 输入  ：arr, psc
 * 输出  ：无
 * 调用  ：外部调用
 */

void TIM3_Int_Init(u16 arr,u16 psc)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure; 

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE); 

	TIM_TimeBaseStructure.TIM_Period = arr;      
	TIM_TimeBaseStructure.TIM_Prescaler =psc;	    
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1 ;	
	TIM_TimeBaseStructure.TIM_CounterMode =TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	TIM_SelectOutputTrigger(TIM3,TIM_TRGOSource_Update);
	/*选择update event作为TRGO,利用TIM3触发ADC通道 */
	//每个定时周期结束后触发一次
	TIM_ClearFlag(TIM3, TIM_FLAG_Update);
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM3, ENABLE);                 

	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);  													
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;	  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , DISABLE); 
	/*先关闭等待使用*/ 
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3 , ENABLE);
}



//定时器3中断服务函数
 /*该源码由开源硬件提供*/
void TIM3_IRQHandler(void)
{
	unsigned char i = 0;
	unsigned int  runningTotal;

	if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET)
	{
//		读取到的值右移2位，12位-->10位
		Signal = ADC_GetConversionValue(ADC1)/4;     //读取A/D转换数据
//		Signal=Get_Adc_Average(ADC_Channel_0,1)>>2;//读取A/D转换数据
		sampleCounter += 2;                          //计算CPU运行时间	
		Num = sampleCounter - lastBeatTime;          //监控最后一次节拍后的时间，以避免噪声	

		//发现脉冲波的波峰和波谷
		if(Signal < thresh && Num > (IBI/5)*3)  	//为了避免需要等待3/5个IBI的时间
		{ 
			if (Signal < T)                         //T是阈值
			{
				T = Signal;                         //跟踪脉搏波的最低点，改变阈值
			}
		}
		if(Signal > thresh && Signal > P)        	//采样值大于阈值并且采样值大于峰值
		{
			P = Signal;                             //P是峰值，改变峰值
		}                                        
		//开始寻找心跳,现在开始寻找心跳节拍
		//当脉冲来临的时候，signal的值会上升
		if (Num > 250)                              //避免高频噪声
		{ 
			if ( (Signal > thresh) && (Pulse == false) && (Num > (IBI/5)*3) )
			{        
				Pulse = true;                       //当有脉冲的时候就设置脉冲信号
//				LED_ON();							//打开LED，表示已经有脉冲了
				IBI = sampleCounter - lastBeatTime; //测量节拍的ms级的时间
				lastBeatTime = sampleCounter;       //录下一个脉冲的时间。

				if(secondBeat)						//如果这是第二个节拍，如果secondBeat == TRUE，表示是第二个节拍
				{                        		
					secondBeat = false;             //清除secondBeat节拍标志
					i = 0;
					for( i=0; i<=9; i++)			//在启动时，种子的运行总数得到一个实现的BPM。
					{                 
						rate[i] = IBI;                     		
					}
				}
				if(firstBeat)                        //如果这是第一次发现节拍，如果firstBeat == TRUE。
				{
					firstBeat = false;               //清除firstBeat标志
					secondBeat = true;               //设置secongBeat标志
					return;                          //IBI值是不可靠的，所以放弃它。
				}   

				//保留最后10个IBI值的运行总数。
				runningTotal = 0;                  	 //清除runningTotal变量     
				for(i=0; i<=8; i++)                  //转换数据到rate数组中
				{
					rate[i] = rate[i+1];              // 去掉旧的的IBI值。 
					runningTotal += rate[i];          //添加9个以前的老的IBI值。
				}

				rate[9] = IBI;                        //将最新的IBI添加到速率数组中。
				runningTotal += rate[9];              //添加最新的IBI到runningTotal。
				runningTotal /= 10;                   //平均最后10个IBI值。
				BPM = 60000/runningTotal;             //一分钟有多少拍。即心率BPM
				QS = true;                            //设置量化自我标志Quantified Self标志
			}                       
		}
		//脉冲开始下降
		if (Signal < thresh && Pulse == true)  		//当值下降时，节拍就结束了。
		{
//			LED0=1; 								//灯灭
			Pulse = false;                         	//重设脉冲标记，这样方便下一次的计数
			amp = P - T;                           	//得到脉冲波的振幅。
			thresh = amp/2 + T;                    	//设置thresh为振幅的50%。
			P = thresh;                            	//重新设置下一个时间
			T = thresh;
		}
		//没有检测到脉冲，设置默认值
		if (Num > 2500)                    	 		//如果2.5秒过去了还没有节拍
		{ 
			thresh = 512;                          	//设置默认阈值
			P = 512;                               	//设置默认P值
			T = 512;                               	//设置默认T值
			lastBeatTime = sampleCounter;          	//把最后的节拍跟上来。      
			firstBeat = true;                      	//设置firstBeat为true方便下一次处理
			secondBeat = false;                    	//设置secondBeat为false方便重新处理
			QS = false; 							//清除标志
		}

	}
	TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
}


