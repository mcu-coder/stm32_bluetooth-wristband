/******************************************************************************

 ******************************************************************************
*  文 件 名   : Pedometer.c
Pedometer计步算法子程序
******************************************************************************/
#include "adxl345.h"
#include "Pedometer.h"
#include "EXTI.h"


#include <math.h>
#include <stdlib.h>
#include <stdio.h>

_Bool ADXL345_FLAG = 0;						//采样标志位 采样时间20ms标志位

unsigned char bad_flag[3];					//错误标志位 为1说明迈出步了 0说明没有迈步
unsigned short int array0[3]={1,1,1}; 		//四个采样数组array0 array1 array2 array3
unsigned short int array1[3]={1,1,1};
unsigned short int array2[3]={0,0,0};
unsigned short int array3[3]={0,0,0};
unsigned short int filter_out[3];	   		//滤波器存储数据
unsigned short int max[3]={0,0,0};		   	//3轴加速度最大值
unsigned short int min[3]={0,0,0};			//3轴加速度最小值
unsigned short int dc[3]={0,0,0};	   		//动态阈值
unsigned short int vpp[3]={30,30,30};		//双峰值	
unsigned short int precision[3]={5,5,5};	//预定义动态精度
unsigned short int sample_old[3];			//sample_old寄存器
unsigned short int sample_new[3];  			//sample_new寄存器
unsigned short int interval = 0;			//时间窗口
unsigned char Regulation0,Regulation1,Regulation2,Regulation3;		//计数规则
unsigned short int STEPS=0;				    //步数

_Bool STEPS_DIS = 0;						//步数显示更新

float Height = 1.7; 						//身高默认1.7m
unsigned char Weight = 60;					//体重默认 60kg
float StepLen = 0;							//步长
										
float Speed=0;								//行走瞬时速度 	单位：m/s	
float Dist=0;								//行走距离		单位：m
float Calories=0; 							//卡路里   		单位：C

/************************************************************************
* 函数: void step_counter(void)
* 描述: 实现Pedometer的基本算法.
* 参数: none.
* 返回: none.
************************************************************************/
void step_counter(void)
{
	char ds[20];
	short x, y, z;
	static unsigned char sampling_counter=0;				//采样次数计数
	unsigned char jtemp;
	ADXL345_RD_XYZ(BUF);									//连续读出数据，存储在BUF中 
	x=(unsigned short int)(((u16)BUF[1]<<8)+BUF[0]); 	    
	y=(unsigned short int)(((u16)BUF[3]<<8)+BUF[2]); 	    
	z=(unsigned short int)(((u16)BUF[5]<<8)+BUF[4]); 
	
	//采样滤波 数字滤波器 将读取到的数据变得平滑
	//需要一个数字滤波器。可以使用四个寄存器和一个求和单元，当然，可以使用更多寄存器以使加速度数据更加平滑，但响应时间会变慢。
	for(jtemp=0;jtemp<=2;jtemp++)      						//jtemp 0,1,2分别代表x，y，z
	{	
		array3[jtemp]=array2[jtemp];
		array2[jtemp]=array1[jtemp];
		array1[jtemp]=array0[jtemp];	
		array0[jtemp]=abs((short int)(BUF[2*jtemp]+(unsigned short int)((unsigned short int)BUF[2*jtemp+1]<<8)));		//数据合成

		sprintf(ds,"%4d %4d  %5d ",x,y,z);
//		Lcd1602_String(0,1,(u8 *)ds);	
		
   		filter_out[jtemp]=(array0[jtemp]+array1[jtemp]+array2[jtemp]+array3[jtemp])/4; 		//滤波后的值保存到filter_out
		if(filter_out[jtemp]>max[jtemp])               {max[jtemp]=filter_out[jtemp];}	    //计算出最大值max
		if(filter_out[jtemp]<min[jtemp])               {min[jtemp]=filter_out[jtemp];}	    //计算出最小值min
	}
	sampling_counter=sampling_counter+1;					//采样次数自增
	//计算动态门限和动态精度
	//每采样50次更新一次。
    if(sampling_counter>=50)  								//采样50次？
    {                
      	sampling_counter=0;									//清除sampling_counter计数值		
		for(jtemp=0;jtemp<=2;jtemp++)
		{
			vpp[jtemp]=max[jtemp]-min[jtemp];				//计算双峰值			
			dc[jtemp] =(max[jtemp]+min[jtemp])/2;    		//dc为动态阈值 计算动态阈值	平均值(Max + Min)/2称为"动态阈值"

			max[jtemp]=0;	 								//重新初始各轴的最大、最小值
        	min[jtemp]=1024;
			bad_flag[jtemp]=0;	   							//错误标志位0

			/*调试的时候显示动态阈值 方便看到数据*/
			/*
            Lcd1602_Write_Com(0Xc0+0);
            Lcd1602_Write_Data(max[0]/10000+0x30);
            Lcd1602_Write_Data(max[0]%10000/1000+0x30);
            Lcd1602_Write_Data(max[0]%10000%1000/100+0x30);
            Lcd1602_Write_Data(max[0]%10000%1000%100/10+0x30);
            Lcd1602_Write_Data(max[0]%10000%1000%100%10+0x30);	
            
            Lcd1602_Write_Com(0Xc0+8);
            Lcd1602_Write_Data(min[0]/10000+0x30);
            Lcd1602_Write_Data(min[0]%10000/1000+0x30);
            Lcd1602_Write_Data(min[0]%10000%1000/100+0x30);
            Lcd1602_Write_Data(min[0]%10000%1000%100/10+0x30);
            Lcd1602_Write_Data(min[0]%10000%1000%100%10+0x30);	
                    
            Lcd1602_Write_Com(0X80+11);
            Lcd1602_Write_Data(vpp[0]/10000+0x30);
            Lcd1602_Write_Data(vpp[0]%10000/1000+0x30);
            Lcd1602_Write_Data(vpp[0]%10000%1000/100+0x30);
            Lcd1602_Write_Data(vpp[0]%10000%1000%100/10+0x30);
            Lcd1602_Write_Data(vpp[0]%10000%1000%100%10+0x30);	
            
            Lcd1602_Write_Com(0X80+11);
            Lcd1602_Write_Data(vpp[0]/10000+0x30);
            Lcd1602_Write_Data(vpp[0]%10000/1000+0x30);
            Lcd1602_Write_Data(vpp[0]%10000%1000/100+0x30);
            Lcd1602_Write_Data(vpp[0]%10000%1000%100/10+0x30);
            Lcd1602_Write_Data(vpp[0]%10000%1000%100%10+0x30);

			Lcd1602_Write_Com(0Xc0+0);
			Lcd1602_Write_Data(vpp[1]/10000+0x30);
			Lcd1602_Write_Data(vpp[1]%10000/1000+0x30);
			Lcd1602_Write_Data(vpp[1]%10000%1000/100+0x30);
			Lcd1602_Write_Data(vpp[1]%10000%1000%100/10+0x30);
			Lcd1602_Write_Data(vpp[1]%10000%1000%100%10+0x30);

			Lcd1602_Write_Com(0Xc0+7);
			Lcd1602_Write_Data(vpp[2]/10000+0x30);
			Lcd1602_Write_Data(vpp[2]%10000/1000+0x30);
			Lcd1602_Write_Data(vpp[2]%10000%1000/100+0x30);
			Lcd1602_Write_Data(vpp[2]%10000%1000%100/10+0x30);
			Lcd1602_Write_Data(vpp[2]%10000%1000%100%10+0x30);
			*/
			if(vpp[jtemp]>=250)	   							//对于跑步者，峰峰值会更高。这个数据 可以调整
			{
				precision[jtemp]=vpp[jtemp]/50; 			//跑步
			}
			else if((vpp[jtemp]>=100) && (vpp[jtemp]<250))  //走路 这个数据 可以调整
			{
				precision[jtemp] = 3;						//走路
			}			
			else
       		{ 
          		precision[jtemp]=2;
            	bad_flag[jtemp]=1;
        	}
		}
  	}		
	//--------------------------线性移位寄存器 消除高频噪声--------------------------------------
	for(jtemp=0;jtemp<=2;jtemp++)
	{
		sample_old[jtemp]=sample_new[jtemp];   			//当新采样数据到来时，sample_new无条件移入sample_old寄存器

		//filter_out是否移入sample_new寄存器取决于下述条件
    	if(filter_out[jtemp]>=sample_new[jtemp])       //如果加速度变化大于预定义精度，则最新的采样结果filter_out移入sample_new寄存器                  
    	{   
     		if((filter_out[jtemp]-sample_new[jtemp])>=precision[jtemp])   
				{sample_new[jtemp]=filter_out[jtemp];}
    	}
    	else if(filter_out[jtemp]<sample_new[jtemp])
   	 	{   
       		if((sample_new[jtemp]-filter_out[jtemp])>=precision[jtemp])   
				{sample_new[jtemp]=filter_out[jtemp];}
    	}	
	}
	//动态门限判决  最大峰值检测,判断活跃轴,步伐判断
	//峰值检测：步伐计数器根据x、y、z三轴中加速度变化最大的一个轴计算步数。如果加速度变化太小，步伐计数器将忽略。
	/*
	步伐计数器利用此算法可以很好地工作，但有时显得太敏感。当计步器因为步行或跑步之外的原因而非常迅速或非常缓慢
	地振动时，步伐计数器也会认为它是步伐。为了找到真正的有节奏的步伐，必须排除这种无效振动。利用"时间窗口"和"计
	数规则"可以解决这个问题。
	*/
	if((vpp[0]>=vpp[1])&&(vpp[0]>=vpp[2]))   //x轴最活跃
	{
		//步伐迈出的条件定义为：当加速度曲线跨过动态阈值下方时，加速度曲线的斜率为负值(sample_new < sample_old)。 .
		//sample_new  < dc 表示已迈过动态阈值下方
		//sample_new  - sample_old  < 0 表示加速度的变化小于0,斜率为负数
		if((sample_old[0]>=dc[0])&&(sample_new[0]<dc[0])&&(bad_flag[0]==0))        
		{
			/*
			"时间窗口"用于排除无效振动。假设人们最快的跑步速度为每秒5步，最慢的步行速度为每2秒1步。这样，两个有
			效步伐的时间间隔在时间窗口[0.2 s - 2.0 s]之内，时间间隔超出该时间窗口的所有步伐都应被排除。
			采用interval的寄存器记录两步之间的数据更新次数。如果间隔值在10与100之间，则说明两步之间的时间在有效
			窗口之内；否则，时间间隔在时间窗口之外，步伐无效。
			*/
			/*----------时间窗口--------*/
			if((interval > 10) && (interval < 100))
			{
				/*-----------计数规则--------------*/
				/*
				"计数规则" 用于确定步伐是否是一个节奏模式的一部分。步伐计数器有两个工作状态：搜索规则和确认规则
				。步伐计数器以搜索规则模式开始工作。假设经过四个连续有效步伐之后，发现存在某种规则(in regulation)
				，那么步伐计数器就会刷新和显示结果，并进入"确认规则"工作模式。在这种模式下工作时，每经过一个有效
				步伐，步伐计数器就会更新一次。但是，如果发现哪怕一个无效步伐，步伐计数器就会返回搜索规则模式，重
				新搜索四个连续有效步伐。
				*/
				STEPS=STEPS+1;
				Regulation3 = Regulation2;
				Regulation2 = Regulation1;
				Regulation1 = Regulation0;
				Regulation0 = 1;
				if( Regulation3 && Regulation2 && Regulation1 && Regulation0) 		//达到四步以上 更新步数
					STEPS_DIS = 1;
			}
			else
			{		
				Regulation3 = 0;
				Regulation2 = 0;
				Regulation1 = 0;
				Regulation0 = 0;		
			}
			interval = 0;
		} 
	}
	else if((vpp[1]>=vpp[0])&&(vpp[1]>=vpp[2])) 	//y轴最活跃
	{
		if((sample_old[1]>=dc[1])&&(sample_new[1]<dc[1])&&(bad_flag[1]==0))        
		{
			/*----------时间窗口--------*/
			if((interval > 10) && (interval < 100))
			{
				STEPS=STEPS+1;
				/*-----------计数规则--------------*/
				Regulation3 = Regulation2;
				Regulation2 = Regulation1;
				Regulation1 = Regulation0;
				Regulation0 = 1;
				if( Regulation3 && Regulation2 && Regulation1 && Regulation0) 		//达到四步以上 更新步数
					STEPS_DIS = 1;	
			}
			else
			{
				Regulation3 = 0;
				Regulation2 = 0;
				Regulation1 = 0;
				Regulation0 = 0;
			}
			interval = 0;
		}
	}
	else if((vpp[2]>=vpp[1])&&(vpp[2]>=vpp[0]))    //z轴最活跃
	{
		if((sample_old[2]>=dc[2])&&(sample_new[2]<dc[2])&&(bad_flag[2]==0))        
		{
			/*----------时间窗口--------*/
			if((interval > 10) && (interval < 100))
			{
				/*-----------计数规则--------------*/
				STEPS=STEPS+1;	
				Regulation3 = Regulation2;
				Regulation2 = Regulation1;
				Regulation1 = Regulation0;
				Regulation0 = 1;
				if( Regulation3 && Regulation2 && Regulation1 && Regulation0)
					STEPS_DIS = 1;		
			}
			else
			{
				Regulation3 = 0;
				Regulation2 = 0;
				Regulation1 = 0;
				Regulation0 = 0;					 
			}	
			interval = 0;
		}
	}
	else
	{
		STEPS_DIS = 0;
	}
}
