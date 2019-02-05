#include "mbed.h"
#include "SOIL.h"
#define DEFAULT 100

SOIL::SOIL(PinName AIN) : a_in(AIN){
	sData.soil=0;
	sData.maxSoil=DEFAULT;
	sData.meanSoil=0;
	sData.minSoil=DEFAULT;
	accSoil=0;
	count=0;
}

SOIL::~SOIL() { }

SoilData SOIL::measure(void){
	sData.soil=a_in;
	sData.soil*=100;
	addReg(sData.soil);
	
	return sData;
}

void SOIL::addReg (float light){
	if(count==120){
		reset();
	}
	accSoil+=sData.soil;
	count++;
	
	if(sData.maxSoil==DEFAULT)
		sData.maxSoil=sData.soil;
	if(sData.minSoil==DEFAULT)
		sData.minSoil=sData.soil;
	
	if(sData.soil>sData.maxSoil)
		sData.maxSoil=sData.soil;
	if(sData.soil<sData.minSoil)
		sData.minSoil=sData.soil;
	
	sData.meanSoil=(float)accSoil/count;
	
}

void SOIL::reset(void){
	sData.soil=0;
	sData.maxSoil=DEFAULT;
	sData.minSoil=DEFAULT;
	sData.meanSoil=0;
	accSoil=0;
	count=0;
}