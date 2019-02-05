#include "mbed.h"
#include "Si7021.h"

#define READ_TEMP        0xE0 /* Read previous T data from RH measurement command*/
/** Si7012 Read RH Command */
#define READ_RH          0xE5 /* Perform RH (and T) measurement. */
 
/** Si7012 Read ID */
#define READ_ID1_1       0xFA
#define READ_ID1_2       0x0F
#define READ_ID2_1       0xFC
#define READ_ID2_2       0xC9
 
/** Si7012 Read Firmware Revision */
#define READ_FWREV_1     0x84
#define READ_FWREV_2     0xB8
 
/** I2C device address for Si7021 */
#define ADDR    0x80
 
/** Device ID value for Si7021 */
#define DEVICE_ID 0x15
#define DEFAULT 						 100

Si7021::Si7021(PinName sda, PinName scl, int addr) : m_i2c(sda, scl){
	m_addr=addr;
	ambData.humidity=0;
	ambData.maxHumidity=DEFAULT;
	ambData.minHumidity=DEFAULT;
	ambData.meanHumidity=0;
	ambData.temperature=0;
	ambData.maxTemperature=DEFAULT;	
	ambData.minTemperature=DEFAULT;
	ambData.meanTemperature=0;
	accHum=0;
	accTmp=0;
	count=0;
}

Si7021::~Si7021() { }

bool Si7021::check(void){
	tx_buff[0] = READ_ID2_1;
	tx_buff[1] = READ_ID2_2;
	if(m_i2c.write(ADDR, (char*)tx_buff, 2) != 0) return 0;
	if(m_i2c.read(ADDR, (char*)tx_buff, 2) != 0) return 0;
	
	if(rx_buff[0] ==DEVICE_ID)
		return 1;
	else return 0;
}

AmbientData Si7021::measure(void){
	tx_buff[0] = READ_RH;
	if(m_i2c.write(ADDR, (char*)tx_buff, 1) == 0){
		if(m_i2c.read(ADDR, (char*)rx_buff, 2) == 0){
			ambData.humidity = ((uint32_t)rx_buff[0] << 8) + (rx_buff[1] & 0xFC);
			ambData.humidity = ((((uint32_t)ambData.humidity) * 15625L) >> 13) - 6000;
			ambData.humidity = ambData.humidity/1000;
			//ambData.humidity = (((uint32_t)ambData.humidity) * 125/65536) - 6;
		}
	}

	tx_buff[0] = READ_TEMP;
	if(m_i2c.write(ADDR, (char*)tx_buff, 1) == 0){
		if(m_i2c.read(ADDR, (char*)rx_buff, 2) == 0){
			ambData.temperature = ((uint32_t)rx_buff[0] << 8) + (rx_buff[1] & 0xFC);
			ambData.temperature = ((((uint32_t)ambData.temperature) * 21965L) >> 13) - 46850;
			ambData.temperature = ambData.temperature/1000;
			//ambData.temperature = (((uint32_t)ambData.temperature) * 175.72/65536) - 46.85;
		}
	}
	
	addReg(ambData.humidity, ambData.temperature);
	
	return ambData;
}

void Si7021::addReg (float hum, float tmp){
	if(count==120){
		reset();
	}
	accHum+=hum;
	accTmp+=tmp;
	count++;
	
	if(ambData.maxTemperature==DEFAULT)
		ambData.maxTemperature=ambData.temperature;
	if(ambData.minTemperature==DEFAULT)
		ambData.minTemperature=ambData.temperature;
	if(ambData.maxHumidity==DEFAULT)
		ambData.maxHumidity=ambData.humidity;
	if(ambData.minHumidity==DEFAULT)
		ambData.minHumidity=ambData.humidity;
	
	if(ambData.temperature>ambData.maxTemperature)
		ambData.maxTemperature=ambData.temperature;
	if(ambData.temperature<ambData.minTemperature)
		ambData.minTemperature=ambData.temperature;
	if(ambData.humidity>ambData.maxHumidity)
		ambData.maxHumidity=ambData.humidity;
	if(ambData.humidity<ambData.minHumidity)
		ambData.minHumidity=ambData.humidity;
	
	ambData.meanHumidity=(float)accHum/count;
	ambData.meanTemperature=(float)accTmp/count;
	
}

void Si7021::reset(void){
	ambData.humidity=0;
	ambData.maxHumidity=DEFAULT;
	ambData.minHumidity=DEFAULT;
	ambData.meanHumidity=0;
	ambData.temperature=0;
	ambData.maxTemperature=DEFAULT;	
	ambData.minTemperature=DEFAULT;
	ambData.meanTemperature=0;
	accHum=0;
	accTmp=0;
	count=0;
}