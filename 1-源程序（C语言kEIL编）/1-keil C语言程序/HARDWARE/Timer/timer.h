#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"

#define true 1
#define false 0

extern int IBI;          //相邻节拍时间
extern int BPM;          //心率值             
extern int Signal;       //原始信号值            
extern unsigned char QS; //发现心跳标志

#define HEART_MAX_ERROR		160		//心率的不可到值，超过此值表示传感器出错
#define HEART_MIN_ERROR		40		//心率的不可到值，低于此值表示传感器出错


void TIM2_Int_Init(u16 arr,u16 psc);
void TIM3_Int_Init(u16 arr,u16 psc);
	
#endif
