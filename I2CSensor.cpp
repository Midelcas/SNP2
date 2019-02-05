// Example program connecting to the TCS34725 Color Sensor to the K64F using I2C

#include "mbed.h"
#include "TCS34725.h"
#include "MMA8451Q.h"
#include "Si7021.h"
#define TCS_ADDR 0x52
#define MMA_ADDR 0x1D
#define SI7_ADDR 0x50

ColorData colorData;
AccelerometerData accData;
AmbientData ambData;
TCS34725 *tcs=0;
MMA8451Q *acc=0;
Si7021 *amb=0;

extern int count;

Thread threadI2C(osPriorityNormal, 1024); // 1K stack size

void I2C_thread();

void I2C_thread() {
		tcs = new TCS34725(PB_9,PB_8,TCS_ADDR, PB_14);
		acc = new MMA8451Q(PB_9,PB_8,MMA_ADDR);
		amb = new Si7021(PB_9,PB_8,SI7_ADDR);
	
		if(tcs->checkId()){
			tcs->initColorSensor();
		}
		while (true) {
			/*if(count==0){
				tcs->reset();
				acc->reset();
				amb->reset();
			}*/
			colorData = tcs->readRegisters();
			wait(0.1);
			accData = acc->getAccAllAxis();
			wait(0.1);
			ambData = amb->measure();
			threadI2C.signal_wait(0x1);
    }
		
}
