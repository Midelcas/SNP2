#include "mbed.h"
#include "HW5P-1.h"
#define DEFAULT 100

HW5P_1::HW5P_1(PinName AIN) : a_in(AIN){
	lData.light=0;
	lData.maxLight=DEFAULT;
	lData.meanLight=0;
	lData.minLight=DEFAULT;
	accLight=0;
	count=0;
}

HW5P_1::~HW5P_1() { }

LightData HW5P_1::measure(void){
	lData.light=a_in;
	lData.light*=100;
	addReg(lData.light);
	
	return lData;
}

void HW5P_1::addReg (float light){
	if(count==120){
		reset();
	}
	accLight+=lData.light;
	count++;
	
	if(lData.maxLight==DEFAULT)
		lData.maxLight=lData.light;
	if(lData.minLight==DEFAULT)
		lData.minLight=lData.light;
	
	if(lData.light>lData.maxLight)
		lData.maxLight=lData.light;
	if(lData.light<lData.minLight)
		lData.minLight=lData.light;
	
	lData.meanLight=(float)accLight/count;
	
}

void HW5P_1::reset(void){
		lData.light=0;
		lData.maxLight=DEFAULT;
		lData.minLight=DEFAULT;
		lData.meanLight=0;
		accLight=0;
		count=0;
}
