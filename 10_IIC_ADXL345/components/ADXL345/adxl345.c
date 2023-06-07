#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "stdint.h"
#include "stdbool.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "math.h"  
#include "string.h"  
#include "../../build/config/sdkconfig.h"

#include "adxl345.h"
static const char *TAG = "adxl345";

#define SensorAdd						(0x1D)//0x53//(0x53)(0xA7>>1)
#define I2C_MASTER_TX_BUF_DISABLE		0	// I2C master do not need buffer
#define I2C_MASTER_RX_BUF_DISABLE		0	// I2C master do not need buffer
#define ACK_CHECK_EN					0x1	// I2C master will check ack from slave
#define ACK_CHECK_DIS					0x0	// I2C master will not check ack from slave
#define ACK_VAL							0x0	// I2C ack value
#define NACK_VAL						0x1	// I2C nack value


uint8_t Init_ADXL345(uint8_t sdaPin, uint8_t sclPin) 
{
	// I2C 配置结构体
	i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,				// I2C 模式
		.sda_io_num = sdaPin,					// SDA IO映射
		.sda_pullup_en = GPIO_PULLUP_ENABLE,	// SDA IO模式
		.scl_io_num = sclPin,					// SCL IO映射
		.scl_pullup_en = GPIO_PULLUP_ENABLE,	// SCL IO模式
		.master.clk_speed = 200000,				// I2C CLK频率
	};
	i2c_param_config(I2C_NUM_1, &conf);			// 设置I2C1
	// 注册I2C1服务即使能
	i2c_driver_install(I2C_NUM_1, conf.mode,I2C_MASTER_RX_BUF_DISABLE,I2C_MASTER_TX_BUF_DISABLE,0);
	if(GetDevicesID()==0XE5){					// 读取器件ID
		ADXL345_WR_Reg(INT_ENABLE,0x00);		// 先关闭中断
		ADXL345_WR_Reg(DATA_FORMAT,0X0B);		// 禁用自测力 4线spi模式 高电平中断有效,13位全分辨率,输出数据右对齐,16g量程 1B
		ADXL345_WR_Reg(BW_RATE,0x1A);			// 低功耗模式，最低速度   数据输出速度为100Hz 12.5
		ADXL345_WR_Reg(POWER_CTL,0x08);			// 自动休眠模式 休眠时以8HZ频率采样 INT_ENABLE 38 08
		ADXL345_WR_Reg(INT_MAP,0xA0);			// THRESH_TAP/ACTIVITY映射到INT1引脚
		ADXL345_WR_Reg(THRESH_ACT,125);			// 检测活动(ACTIVITY)的阈值，62.5mg/LSB ，其中0x10代表16, 16*62.5=1000mg, 也就是1g， 0x04表示4*62.5 = 250mg
		ADXL345_WR_Reg(THRESH_INACT,0x40);		// 设置静止检测阈值inactivity 62.5mg/LSB
		ADXL345_WR_Reg(TIME_INACT,0x01);		// 当小于inactivity值时间超过0X02 2s后进入睡眠模式
		ADXL345_WR_Reg(ACT_INACT_CTL,0x77);		// 直流触发配置，XYZ使能触发配置，所有轴都参与  CC
		ADXL345_WR_Reg(THRESH_FF,0x08);			// 自由跌落阈值，0x05-0x09 300mg-600mg 所有轴都参与判断加速度 62.5mg/LSB
		ADXL345_WR_Reg(TIME_FF,0x14);			// 自由跌落时间 0x14-0x46 100ms-350ms 5ms/LBS
		ADXL345_WR_Reg(INT_ENABLE,0x14);		// 中断使能 使能Activity中断,使能Free_Fall中断 14	
		return 0;
	}
	return 1;
}

//自动校准
//xval,yval,zval:x,y,z轴的校准值
void ADXL345_AUTO_Adjust(char *xval,char *yval,char *zval)
{
	uint8_t i;
	short tx,ty,tz,offx=0,offy=0,offz=0;
	ADXL345_WR_Reg(POWER_CTL,0x00);		// 先进入休眠模式.
	vTaskDelay(100 / portTICK_RATE_MS);
	ADXL345_WR_Reg(DATA_FORMAT,0X2B);	// 低电平中断输出,13位全分辨率,输出数据右对齐,16g量程 
	ADXL345_WR_Reg(BW_RATE,0x0A);		// 数据输出速度为100Hz
	ADXL345_WR_Reg(POWER_CTL,0x28);		// 链接使能,测量模式
	ADXL345_WR_Reg(INT_ENABLE,0x00);	// 不使用中断

	ADXL345_WR_Reg(OFSX,0x00);
	ADXL345_WR_Reg(OFSY,0x00);
	ADXL345_WR_Reg(OFSZ,0x00);
	vTaskDelay(12 / portTICK_RATE_MS);
	for(i=0;i<10;i++){
		ADXL345_RD_Avval(&tx,&ty,&tz);
		offx+=tx;
		offy+=ty;
		offz+=tz;
	}
	offx/=10;
	offy/=10;
	offz/=10;
	*xval=-offx/4;
	*yval=-offy/4;
	*zval=-(offz-256)/4;
 	ADXL345_WR_Reg(OFSX,*xval);
	ADXL345_WR_Reg(OFSY,*yval);
	ADXL345_WR_Reg(OFSZ,*zval);
} 
// 获取芯片ID
uint8_t GetDevicesID(void)
{
	esp_err_t ret = 0;
	uint8_t data = 0;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (SensorAdd << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, DEVICE_ID, ACK_CHECK_EN);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (SensorAdd << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
	i2c_master_read_byte(cmd, &data, NACK_VAL);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	ESP_LOGI(TAG,"Init_ADXL355: I2C_MASTER_WRITE  %02X\r\n", (SensorAdd << 1) | I2C_MASTER_WRITE);
	ESP_LOGI(TAG,"Init_ADXL355: I2C_MASTER_READ   %02X\r\n",(SensorAdd << 1) | I2C_MASTER_READ);
	return data;
}

//写ADXL345寄存器
//addr:寄存器地址
//val:要写入的值
//返回值:无
void ADXL345_WR_Reg(uint8_t addr,uint8_t val) 
{
	esp_err_t ret = 0;
	uint8_t data = 0;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, SensorAdd << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, addr, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, val, ACK_CHECK_EN);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return data;
}
//读ADXL345寄存器
//addr:寄存器地址
//返回值:读到的值
uint8_t ADXL345_RD_Reg(uint8_t addr)
{
	esp_err_t ret = 0;
	uint8_t data = 0;
	i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, SensorAdd << 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	i2c_master_write_byte(cmd, addr, ACK_CHECK_EN);
	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, SensorAdd << 1 | I2C_MASTER_READ, ACK_CHECK_EN);
	i2c_master_read_byte(cmd, &data, NACK_VAL);
	i2c_master_stop(cmd);
	ret = i2c_master_cmd_begin(I2C_NUM_1, cmd, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd);
	return data;
}
//读取ADXL的平均值
//x,y,z:读取10次后取平均值
void ADXL345_RD_Avval(short *x,short *y,short *z)
{
	short tx=0,ty=0,tz=0;
	uint8_t i;
	for(i=0;i<10;i++){
		ADXL345_RD_XYZ(x,y,z);
		vTaskDelay(10 / portTICK_RATE_MS);
		tx+=(short)*x;
		ty+=(short)*y;
		tz+=(short)*z;
	}
	*x=tx/10;
	*y=ty/10;
	*z=tz/10;
} 
//读取3个轴的数据
//x,y,z:读取到的数据
void ADXL345_RD_XYZ(short *x,short *y,short *z)
{
	uint8_t buf[6];
	uint8_t i;
	esp_err_t ret = 0;
	uint8_t data = 0;
	for(i=0;i<6;i++){
		buf[i] = ADXL345_RD_Reg(0x32+i);
		//vTaskDelay(10 / portTICK_RATE_MS);
	}
	*x=(short)(((uint16_t)buf[1]<<8)+buf[0]);
	*y=(short)(((uint16_t)buf[3]<<8)+buf[2]);
	*z=(short)(((uint16_t)buf[5]<<8)+buf[4]);
	//printf("ADXL345->ADXL345_RD_XYZ X:%ls	Y:%ls	Z:%ls \r\n",x,y,z);		//显示加速度原始值
 
}
//读取ADXL345的数据times次,再取平均
//x,y,z:读到的数据
//times:读取多少次
void ADXL345_Read_Average(short *x,short *y,short *z,uint8_t times)
{
	uint8_t i;
	short tx,ty,tz;
	*x=0;
	*y=0;
	*z=0;
	if(times){//读取次数不为0
		for(i=0;i<times;i++){//连续读取times次
			ADXL345_RD_XYZ(&tx,&ty,&tz);
			*x+=tx;
			*y+=ty;
			*z+=tz;
			vTaskDelay(5 / portTICK_RATE_MS);
		}
		*x/=times;
		*y/=times;
		*z/=times;
	}
}
//得到角度
//x,y,z:x,y,z方向的重力加速度分量(不需要单位,直接数值即可)
//dir:要获得的角度.0,与Z轴的角度;1,与X轴的角度;2,与Y轴的角度.
//返回值:角度值.单位0.1°.
short ADXL345_Get_Angle(float x,float y,float z,uint8_t dir)
{
	float temp;
	float res=0;
	switch(dir){
		case 0://与自然Z轴的角度
			temp=sqrt((x*x+y*y))/z;
			res=atan(temp);
			break;
		case 1://与自然X轴的角度
			temp=x/sqrt((y*y+z*z));
			res=atan(temp);
			break;
		case 2://与自然Y轴的角度
			temp=y/sqrt((x*x+z*z));
			res=atan(temp);
			break;
	}
	return res*1800/3.14159;
}

// ADXL345 角度采集
uint16_t Get_ADXL345_Average(void)
{
	short x,y,z;
	short angx,angy,angz;
	uint8_t xbuf[15], ybuf[15], zbuf[15];
	//得到X,Y,Z轴的加速度值(原始值)
	ADXL345_Read_Average(&x, &y,&z,10);	//读取X,Y,Z三个方向的加速度值 
	//printf("ADXL345-> X:%d	Y:%d	Z:%d \r\n",x,y,z);		//显示加速度原始值
	//得到角度值,并显示
	angx=ADXL345_Get_Angle(x,y,z,1);
	angy=ADXL345_Get_Angle(x,y,z,2);
	angz=ADXL345_Get_Angle(x,y,z,0);
	memset(xbuf,0,15);
	memset(ybuf,0,15);
	memset(zbuf,0,15);
	if(angx<0){
		angx = -angx;
		sprintf((char *)xbuf,"-%d.%d",(uint16_t)angx/10,(uint16_t)angx%10);
	}else{
		sprintf((char *)xbuf,"%d.%d",(uint16_t)angx/10,(uint16_t)angx%10);
	}
	if(angy<0){
		angy = -angy;
		sprintf((char *)ybuf,"-%d.%d",(uint16_t)angy/10,(uint16_t)angy%10);
	}else{
		sprintf((char *)ybuf,"%d.%d",(uint16_t)angy/10,(uint16_t)angy%10);
	}
	if(angz<0){
		angz = -angz;
		sprintf((char *)zbuf,"-%d.%d",(uint16_t)angz/10,(uint16_t)angz%10);
	}else{
		sprintf((char *)zbuf,"%d.%d",(uint16_t)angz/10,(uint16_t)angz%10);
	}
	printf("ADXL345->ANG X:%s	Y:%s	Z:%s \r\n",xbuf,ybuf,zbuf);			//显示加速度原始值
	return (uint16_t)angz/10;
}
