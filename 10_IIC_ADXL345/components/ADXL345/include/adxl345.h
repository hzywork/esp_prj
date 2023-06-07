#ifndef __ADXL345_H__
#define __ADXL345_H__
#include "stdint.h"
#include "stdbool.h"

#define DEVICE_ID		0X00 	// 器件ID,0XE5
#define THRESH_TAP		0X1D   	// 敲击阀值寄存器
#define OFSX			0X1E
#define OFSY			0X1F
#define OFSZ			0X20
#define DUR				0X21
#define Latent			0X22
#define Window  		0X23 
#define THRESH_ACT		0X24	// 运动阈值寄存器
#define THRESH_INACT	0X25 	// 静止阈值寄存器
#define TIME_INACT		0X26	// 静止时间			比例1 sec /LSB
#define ACT_INACT_CTL	0X27	// 启用运动/静止检测
#define THRESH_FF		0X28	// 自由下落阈值	建议采用300 mg与600 mg(0x05至0x09)之间的值 比例62.5 mg/LSB
#define TIME_FF			0X29 	// 自由下落时间	建议采用100 ms与350 ms(0x14至0x46)之间的值 比例5ms/LSB
#define TAP_AXES		0X2A  
#define ACT_TAP_STATUS  0X2B 
#define BW_RATE			0X2C 
#define POWER_CTL		0X2D 

#define INT_ENABLE		0X2E		// 设置中断配置
//DATA_READY	SINGLE_TAP		DOUBLE_TAP		Activity	Inactivity 		FREE_FALL Watermark Overrun
//新数据		加速度报警		两次加速度		活动		加速度值小于	自由下落
#define INT_MAP			0X2F
#define INT_SOURCE  	0X30
#define DATA_FORMAT	    0X31
#define DATA_X0			0X32
#define DATA_X1			0X33
#define DATA_Y0			0X34
#define DATA_Y1			0X35
#define DATA_Z0			0X36
#define DATA_Z1			0X37
#define FIFO_CTL		0X38
#define FIFO_STATUS		0X39




uint8_t Init_ADXL345(uint8_t sdaPin, uint8_t sclPin);
//自动校准
//xval,yval,zval:x,y,z轴的校准值
void ADXL345_AUTO_Adjust(char *xval,char *yval,char *zval);
// 获取芯片ID
uint8_t GetDevicesID(void);


void ADXL345_WR_Reg(uint8_t addr,uint8_t val);
uint8_t ADXL345_RD_Reg(uint8_t addr);


//读取ADXL的平均值
//x,y,z:读取10次后取平均值
void ADXL345_RD_Avval(short *x,short *y,short *z);
//读取3个轴的数据
//x,y,z:读取到的数据
void ADXL345_RD_XYZ(short *x,short *y,short *z);
void ADXL345_Read_Average(short *x,short *y,short *z,uint8_t times);
short ADXL345_Get_Angle(float x,float y,float z,uint8_t dir);
uint16_t Get_ADXL345_Average(void);

#endif
