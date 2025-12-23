/******************************************************************************

 ******************************************************************************
*  文 件 名   : ADXL345.c
ADXL345子程序
******************************************************************************/
#include "adxl345.h"
#include "sys.h"
#include "delay.h"
#include "math.h"   

unsigned char BUF[6];                         //接收数据缓存区 


void SDA_IN(void)  
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//使能GPIOB时钟
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU ;   		//上拉输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

}
void SDA_OUT(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//使能GPIOB时钟
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   	//推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}
//初始化IIC
void IIC_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//使能GPIOB时钟
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   	//
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_10|GPIO_Pin_11); 				//PB10,PB11 输出高
}
/************************************************************************
* 函数: void ADXL345_Start(void)
* 描述: IIC 起始信号
* 参数: none.
* 返回: none.
************************************************************************/ 
void ADXL345_Start(void)
{
    
    SDA_OUT();
    IIC_SDA = 1;	                //拉高数据线
    IIC_SCL = 1;	               	//拉高时钟线
    delay_us(5);	                //延时
    IIC_SDA = 0;	                //产生下降沿
    delay_us(5);	                //延时
    IIC_SCL = 0;	                //拉低时钟线
}
/************************************************************************
* 函数: void ADXL345_Stop(void)
* 描述: IIC 停止信号
* 参数: none.
* 返回: none.
************************************************************************/
void ADXL345_Stop(void)
{
	SDA_OUT();
    IIC_SDA = 0;                    //拉低数据线
    IIC_SCL = 1;                    //拉高时钟线
    delay_us(5);                 	//延时
    IIC_SDA = 1;                    //产生上升沿
	delay_us(5);                 	//延时
  
}
/************************************************************************
* 函数: void ADXL345_SendACK(unsigned char ack)
* 描述: 发送应答信号
* 参数: ack (0:ACK 1:NAK)
* 返回: none.
************************************************************************/
void ADXL345_SendACK(unsigned char ack)
{
	SDA_OUT();
    IIC_SDA = ack;                 //写应答信号
    IIC_SCL = 1;                   //拉高时钟线
    delay_us(5);                   //延时
    IIC_SCL = 0;                   //拉低时钟线
    delay_us(5);                   //延时
}

/************************************************************************
* 函数: unsigned char ADXL345_RecvACK(void)
* 描述: 接收应答信号
* 参数: none.
* 返回: none.
************************************************************************/
unsigned char ADXL345_RecvACK(void)
{
	static unsigned char err;
    SDA_IN();					  //切换为输入
	IIC_SCL = 1;                  //拉高时钟线
    delay_us(2);				  //延时
    IIC_SCL = 1;       			  //拉高时钟线
    if(READ_SDA== 1)   			  //CY = SDA;  读应答信号
    {
		err = 1;
    }
    else
    {
		err = 0;
    }
    IIC_SCL = 0 ;				 //拉低时钟线
    delay_us(5);	             //延时
    SDA_OUT();					 //设置SDA为输出
    return err;
}
/************************************************************************
* 函数: void ADXL345_SendByte(unsigned char dat)
* 描述: 向IIC总线发送一个字节数据
* 参数: dat:要发送的数据
* 返回: none.
************************************************************************/
void ADXL345_SendByte(unsigned char dat)
{
    unsigned char i;
    SDA_OUT();
    for (i=0; i<8; i++)         //8位计数器
    {
        delay_us(5);            //延时
        if(dat&0x80)  			//SDA = CY; 送数据口
        {
			IIC_SDA = 1;
		}
        else
        {
			IIC_SDA = 0;
		}       
        delay_us(5);           	//延时
        IIC_SCL=1;			   	//拉高时钟线
        delay_us(5);   		   	//延时
        IIC_SCL = 0;			//拉低时钟线
        dat <<= 1;              //移出数据的最高位
    }
    ADXL345_RecvACK();			//等待响应
}
/************************************************************************
* 函数: void ADXL345_SendByte(unsigned char dat)
* 描述: 从IIC总线接收一个字节数据
* 参数: none.
* 返回: dat:收到的数据
************************************************************************/
unsigned char ADXL345_RecvByte(void)
{
    unsigned char i;
    unsigned char Mid;
    unsigned char dat = 0;
    SDA_IN();					//设置为输入
    for (i=0; i<8; i++)         //8位计数器
    {
        dat <<= 1;
        delay_us(5);            //延时
        IIC_SCL = 1;

		if(READ_SDA== 1)   		//CY = SDA;读应答信号
		{
			Mid = 1;
		}
		else
		{
			Mid = 0;
		}
        if(Mid) dat++;			//读数据
        delay_us(5);     		//延时
        IIC_SCL = 0;			//拉低时钟线
		delay_us(5);     		//延时
    }
    return dat;
}

/************************************************************************
* 函数: void ADXL345_Write_Single(unsigned char REG_Address,unsigned char REG_data)
* 描述: 单字节写入
* 参数: REG_Address：要写入的寄存器地址  REG_data:要写入的数据的数据
* 返回: none.
************************************************************************/
void ADXL345_Write_Single(unsigned char REG_Address,unsigned char REG_data)
{
    ADXL345_Start();                  //起始信号
    ADXL345_SendByte(ADXL_WRITE);     //发送设备地址+写信号
    ADXL345_SendByte(REG_Address);    //内部寄存器地址，请参考中文pdf22页 
    ADXL345_SendByte(REG_data);       //内部寄存器数据，请参考中文pdf22页 
    ADXL345_Stop();                   //发送停止信号
}
/************************************************************************
* 函数: unsigned char Single_Read_ADXL345(unsigned char REG_Address)
* 描述: 单字节读取
* 参数: REG_Address：要读取的寄存器地址
* 返回: REG_data:读到的数据
************************************************************************/
unsigned char ADXL345_Read_Single(unsigned char REG_Address)
{  
	unsigned char REG_data;
	ADXL345_Start();                    //起始信号
	ADXL345_SendByte(ADXL_WRITE);       //发送设备地址+写信号
	ADXL345_SendByte(REG_Address);      //发送存储单元地址，从0开始	
	ADXL345_Start();                    //起始信号
	ADXL345_SendByte(ADXL_READ);        //发送设备地址+读信号
	REG_data=ADXL345_RecvByte();        //读出寄存器数据
	ADXL345_SendACK(1);   				//发送nack
	ADXL345_Stop();                     //停止信号
	return REG_data; 
}
/************************************************************************
* 函数: void ADXL345_Init(void)
* 描述: 初始化ADXL345
* 参数: none.
* 返回: 0：ADXL345正常  1：ADXL345异常
************************************************************************/
u8 ADXL345_Init(void)
{				  
	IIC_Init();									//初始化IIC总线	
	if(ADXL345_Read_Single(DEVICE_ID)==0XE5)	//读取器件ID
	{  
		ADXL345_Write_Single(INT_ENABLE,0X00);  //不使用中断  开机时 先关闭中断 后面再打开	
		ADXL345_Write_Single(DATA_FORMAT,0x2B); //低电平中断输出,13位全分辨率,输出数据右对齐,16g量程 
		ADXL345_Write_Single(BW_RATE,0x09);   	//位4为0:正常功率运行  速率设定为50HZ 
		ADXL345_Write_Single(POWER_CTL,0x08);   //测量位设置为1,置于测量模式  器件处于正常工作模式
		ADXL345_Write_Single(INT_MAP,0X00);		//1000 0000将DATA_READY中断映射到INT1
		ADXL345_Write_Single(INT_ENABLE,0X80);  //1000 0000使能 DATA_READY 中断		
		ADXL345_Write_Single(OFSX,0x00);   		//X 偏移量 根据测试传感器的状态写入pdf29页
		ADXL345_Write_Single(OFSY,0x00);   		//Y 偏移量 根据测试传感器的状态写入pdf29页
		ADXL345_Write_Single(OFSZ,0x00);   		//Z 偏移量 根据测试传感器的状态写入pdf29页
		return 0;
	}			
	return 1;	   								  
}   
/************************************************************************
* 函数: void ADXL345_RD_XYZ(void)
* 描述: 连续读出ADXL345内部加速度数据，地址范围0x32~0x37
* 参数: none.
* 返回: pBuffer[0...6]:读到的数据
************************************************************************/
void ADXL345_RD_XYZ(unsigned char *pBuffer)
{
	unsigned char i;
    ADXL345_Start();                          //起始信号
    ADXL345_SendByte(ADXL_WRITE);          	  //发送设备地址+写信号
    ADXL345_SendByte(0x32);                   //发送存储单元地址，从0x32开始	
    ADXL345_Start();                          //起始信号
    ADXL345_SendByte(ADXL_READ);         	  //发送设备地址+读信号
	for (i=0; i<6; i++)                       //连续读取6个地址数据，存储中BUF
    {
		*pBuffer = ADXL345_RecvByte();        //BUF[0]存储0x32地址中的数据
		pBuffer++;
		if (i == 5)
			ADXL345_SendACK(1);               //最后一个数据需要回NOACK
		else		
			ADXL345_SendACK(0);               //回应ACK
   }
   ADXL345_Stop();                            //停止信号   
}  
