#ifndef _Pedometer_H
#define _Pedometer_H



extern _Bool ADXL345_FLAG;						//人走路步频在0.2-2s之间   标志位
extern _Bool STEPS_DIS ;						//步数显示更新

extern unsigned short int STEPS;
extern unsigned short int interval;				//时间窗口	
								
extern float Height; 							//身高默认1.7m
extern unsigned char Weight;					//体重默认 60kg
extern float StepLen;							//步长								
extern float Speed;								//行走瞬时速度
extern float Dist;								//行走距离
extern float Calories; 							//卡路里

void step_counter(void);
//外部中断0服务程序
void EXTIX_Init(void);
#endif
