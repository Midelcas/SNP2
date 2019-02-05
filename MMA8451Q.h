/**
 * MMA8451Q 3-Axis, 14-bit/8-bit Digital Accelerometer
 */
#ifndef _MMA8451Q_H_
#define _MMA8451Q_H_

#include "mbed.h"

typedef struct{
	float x;
	float x_Max;
	float x_Min;
	float y;
	float y_Max;
	float y_Min;
	float z;
	float z_Max;
	float z_Min;
}AccelerometerData;

class MMA8451Q {
public:
  /**
  * MMA8451Q constructor
  *
  * @param sda SDA pin
  * @param sdl SCL pin
  * @param addr 7bit addr of the I2C peripheral
  */
  MMA8451Q(PinName sda, PinName scl, int addr);

  /**
  * MMA8451Q destructor
  */
  ~MMA8451Q();
  
  /**
   * Get the value of the WHO_AM_I register
   *
   * @returns WHO_AM_I value
   */
  uint8_t getWhoAmI();

  /**
   * Get X axis acceleration
   *
   * @returns X axis acceleration
   */
  float getAccX();

  /**
   * Get Y axis acceleration
   *
   * @returns Y axis acceleration
   */
  float getAccY();

  /**
   * Get Z axis acceleration
   *
   * @returns Z axis acceleration
   */
  float getAccZ();

  /**
   * Get XYZ axis acceleration
   *
   * @param res array where acceleration data will be stored
   */
  AccelerometerData getAccAllAxis(void);
	void reset(void);
private:
  I2C m_i2c;
  int m_addr;
	AccelerometerData accData;
	int count;
  void readRegs(int addr, uint8_t * data, int len);
  void writeRegs(uint8_t * data, int len);
  int16_t getAccAxis(uint8_t addr);

};

#endif /* _MMA8451Q_H_ */
