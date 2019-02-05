/**
 * _SOIL_ 
 */
#ifndef _SOIL_
#define _SOIL_

#include "mbed.h"


typedef struct{
	float soil;
	float maxSoil;
	float minSoil;
	float meanSoil;
}SoilData;

class SOIL {
public:
  
  SOIL(PinName AIN);

  /**
  * HW5P_1 destructor
  */
  ~SOIL();

	SoilData measure(void);
	void reset(void);
private:
  AnalogIn a_in;
  SoilData sData;
	long count;
	float accSoil;
	void addReg (float soil);
} ;

#endif /* _SOIL_ */
