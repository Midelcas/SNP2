/**
 * _HW5P_1_ 
 */
#ifndef _HW5P_1_
#define _HW5P_1_

#include "mbed.h"


typedef struct{
	float light;
	float maxLight;
	float minLight;
	float meanLight;
}LightData;

class HW5P_1 {
public:
  
  HW5P_1(PinName AIN);

  /**
  * HW5P_1 destructor
  */
  ~HW5P_1();

	LightData measure(void);
	void reset(void);
private:
  AnalogIn a_in;
  LightData lData;
	long count;
	float accLight;
	void addReg (float light);
} ;

#endif /* _HW5P_1_ */
