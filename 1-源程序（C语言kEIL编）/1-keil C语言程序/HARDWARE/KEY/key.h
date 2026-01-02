#ifndef __KEY_H
#define __KEY_H	 
#include "sys.h"
 

#define KEY0  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_12)//  按键
#define KEY1  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_13)//  按键
#define KEY2  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_14)//  按键
#define KEY3  GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_15)//  按键


void KEY_Init(void);//IO初始化		

void Key_set(void);
#endif
