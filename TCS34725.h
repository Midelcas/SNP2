/**
 * TCS34725 
 */
#ifndef _TCS34725_H_
#define _TCS34725_H_

#include "mbed.h"
#include <string>

typedef struct{
	int clear_value;
	int red_value;
	int green_value;
	int blue_value;
	int acc_red;
	int acc_green;
	int acc_blue;
	int dominant;
	string hourDominant;
}ColorData;

class TCS34725 {
public:
  /**
  * TCS34725 constructor
  *
  * @param sda SDA pin
  * @param sdl SCL pin
  * @param addr 7bit addr of the I2C peripheral
  */
  TCS34725(PinName sda, PinName scl, int addr, PinName led);

  /**
  * TCS34725 destructor
  */
  ~TCS34725();

	void initColorSensor(void);
	bool checkId(void);
	ColorData readRegisters(void);
	void reset(void);
private:
  I2C m_i2c;
  int m_addr;
	DigitalOut LED;
	ColorData color;
	void dominantColor();
	int count;
} ;

#endif /* _TCS34725_H_ */
