/**
 * Si7021 
 */
#ifndef _SI7021_H_
#define _SI7021_H_

#include "mbed.h"

typedef struct{
	float humidity;
	float maxHumidity;
	float minHumidity;
	float meanHumidity;
	float temperature;
	float maxTemperature;
	float minTemperature;
	float meanTemperature;
}AmbientData;

class Si7021 {
public:
  /**
  * Si7021 constructor
  *
  * @param sda SDA pin
  * @param sdl SCL pin
  * @param addr 7bit addr of the I2C peripheral
  */
  Si7021(PinName sda, PinName scl, int addr);

  /**
  * TCS34725 destructor
  */
  ~Si7021();

	bool check(void);
	AmbientData measure(void);
	void reset(void);
private:
  I2C m_i2c;
  int m_addr;
  uint8_t  rx_buff[8];
  uint8_t  tx_buff[2];
	AmbientData ambData;
	long count;
	float accHum;
	float accTmp;
	void addReg (float hum, float tmp);
} ;

#endif /* _SI7021_H_ */
