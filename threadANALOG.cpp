#include "mbed.h"
#include "rtos.h"
#include "HW5P-1.h"
#include "SOIL.h"

LightData lightData;
SoilData soilData;
Thread threadANALOG(osPriorityNormal, 512); // 1K stack size

void ANALOG_thread(); 
HW5P_1 *lightSensor=0;
SOIL *soilSensor=0;
extern int count;

void ANALOG_thread() {
	lightSensor=new HW5P_1(PA_4);
	soilSensor=new SOIL(PA_0);
	while (true) {
		/*if(count ==0){
			soilSensor->reset();
			lightSensor->reset();
		}*/
		soilData=soilSensor->measure();
		lightData=lightSensor->measure();
		threadANALOG.signal_wait(0x1);		
	}
	
}
