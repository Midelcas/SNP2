

#include "mbed.h"
#include "TCS34725.h"

#define COMMAND 		0x80
#define ENABLE 			0x00
#define ATIME 			0x01
#define WTIME 			0x03
#define AILTL 			0x04
#define AILTH 			0x05
#define AIHTL 			0x06
#define AIHTH 			0x07
#define PERS 				0x0C
#define CONFIG 			0x0D
#define CONTROL 		0x0F
#define ID 					0x12
#define STATUS 			0x13
#define CDATAL 			0x14
#define CDATAH 			0x15
#define RDATAL 			0X16
#define RDATAH 			0X17
#define GDATAL 			0X18
#define GDATAH 			0X19
#define BDATAL 			0X1A
#define BDATAH 			0X1B
#define TCS34725ID 	0x44
#define RED		3
#define BLUE	6
#define GREEN	5



TCS34725::TCS34725(PinName sda, PinName scl, int addr, PinName led) : m_i2c(sda, scl), LED(led) {
	//m_i2c.frequency(100000);
	m_addr=addr;
	LED=0;
	color.red_value=0;
	color.green_value=0;
	color.blue_value=0;
	color.acc_red=0;
	color.acc_green=0;
	color.acc_blue=0;
	color.dominant=0;
	color.hourDominant="";
	count=0;
}

TCS34725::~TCS34725() { }

bool TCS34725::checkId(void){
	char id_regval[1] = {COMMAND|ID};//0x12 device ID
	char data[1] = {0};//response buffer
	m_i2c.write(m_addr,id_regval,1, true);//sets the control to reg 0x12
	m_i2c.read(m_addr,data,1,false);//reads from 0x12
	
	return (data[0]==TCS34725ID);
}

void TCS34725::initColorSensor(void){
	char timing_register[2] = {COMMAND|ATIME,0x00};
	m_i2c.write(m_addr,timing_register,2,false);
	
	char control_register[2] = {COMMAND|CONTROL,0x00};
	m_i2c.write(m_addr,control_register,2,false);
	
	char enable_register[2] = {COMMAND|ENABLE,3};
	m_i2c.write(m_addr,enable_register,2,false);
}

ColorData TCS34725::readRegisters(void){
		LED = 1;
		wait(0.5);
		char color_reg[1]={COMMAND|CDATAL};
		char color_data[8] = {0};
		m_i2c.write(m_addr, color_reg, 1, true);
		m_i2c.read(m_addr,color_data,8,false);
		LED= 0;
			
		color.clear_value = ((int)color_data[1]<<8) | color_data[0];
		color.red_value = ((int)color_data[3] << 8) | color_data[2];
		color.green_value = ((int)color_data[5] <<8)| color_data[4];
		color.blue_value = ((int)color_data[7] <<8) | color_data[6];
		dominantColor();
		return color;
}

void TCS34725::dominantColor(){
	if(count==120){
		reset();
	}
	if(color.red_value>color.green_value){
		if(color.red_value>color.blue_value){
			color.acc_red++;
			color.dominant=RED;
		}else{
			color.acc_blue++;
			color.dominant=BLUE;
		}
	}else if(color.green_value>color.blue_value){
		color.acc_green++;
		color.dominant=GREEN;
	}else{
		color.acc_blue++;
		color.dominant=BLUE;
	}
	
	if(color.acc_blue>color.acc_red){
		if(color.acc_blue>color.acc_green){
			color.hourDominant="Blue";
		}else{
			color.hourDominant="Green";
		}
	}else if(color.acc_red>color.acc_green){
		color.hourDominant="Red";
	}else{
		color.hourDominant="Green";
	}
	
	count++;
}

void TCS34725::reset(void){
	color.acc_red=0;
	color.acc_green=0;
	color.acc_blue=0;
	count=0;
}
