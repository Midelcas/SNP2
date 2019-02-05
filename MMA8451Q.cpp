/**
 * MMA8451Q 3-Axis, 14-bit/8-bit Digital Accelerometer
 */
 
#include "mbed.h"
#include "MMA8451Q.h"

#define REG_STATUS           0x00 // when F_MODE = 00
#define REG_FIFO_STATUS      0x00 // when F_MODE > 0
#define REG_XYZ_FIFO         0x01 // Root pointer to XYZ FIFO data
#define REG_OUT_X_MSB        0x01 // 8 MSBs of 14-bit sample
#define REG_OUT_X_LSB        0x02 // 6 LSBs of 14-bit sample
#define REG_OUT_Y_MSB        0x03 
#define REG_OUT_Y_LSB        0x04
#define REG_OUT_Z_MSB        0x05
#define REG_OUT_Z_LSB        0x06
#define REG_F_SETUP          0x09 // FIFO setup
#define REG_TRIG_CFG         0x0A // Map of FIFO daa capture events
#define REG_SYSMOD           0x0B // Current System Mode
#define REG_INT_SOURCE       0x0C // Interrupt status
#define REG_WHO_AM_I         0x0D // Device ID (0x1A)
#define REG_XYZ_DATA_CFG     0x0E // Dynamic Range Settings
#define REG_HP_FILTER_CUTOFF 0x0F // Cutoff freq is set to 16Hz@800Hz
#define REG_PL_STATUS        0x10 // Landscape/Portrait orientation status
#define REG_PL_CFG           0x11 // Landscape/Portrait configuration
#define REG_PL_COUNT         0x12 // Landscape/Portrait debounce counter 
#define REG_PL_BF_ZCOMP      0x13 // Back/Front, Z-Lock Trip threshold
#define REG_P_L_THS_REG      0x14 // Portrait to Landscape Trip Angle is 29 degree
#define REG_FF_MT_CFG        0x15 // Freefall/Motion function block configuration
#define REG_FF_MT_SRC        0x16 // Freefall/Motion event source register
#define REG_FF_MT_THS        0x17 // Freefall/Motion threshold register
#define REG_FF_MT_COUNT      0x18 // Freefall/Motion debounce counter
// TRANSIENT
#define REG_TRANSIENT_CFG    0x1D // Transient functional block configuration
#define REG_TRANSIENT_SRC    0x1E // Transient event status register
#define REG_TRANSIENT_THS    0x1F // Transient event threshold
#define REG_TRANSIENT_COUNT  0x20 // Transient debounce counter
// PULSE
#define REG_PULSE_CFG        0x21 // ELE, Double_XYZ or Single_XYZ
#define REG_PULSE_SRC        0x22 // EA, Double_XYZ or Single_XYZ
#define REG_PULSE_THSX       0x23 // X pulse threshold
#define REG_PULSE_THSY       0x24 // Y pulse threshold
#define REG_PULSE_THSZ       0x25 // Z pulse threshold
#define REG_PULSE_TMLT       0x26 // Time limit for pulse
#define REG_PULSE_LTCY       0x27 // Latency time for 2nd pulse
#define REG_PULSE_WIND       0x28 // Window time for 2nd pulse
#define REG_ASLP_COUNT       0x29 // Counter setting for Auto-SLEEP
// Control Registers
#define REG_CTRL_REG1        0x2A // ODR = 800Hz, STANDBY Mode
#define REG_CTRL_REG2        0x2B // Sleep Enable, OS Modes, RST, ST
#define REG_CTRL_REG3        0x2C // Wake from Sleep, IPOL, PP_OD
#define REG_CTRL_REG4        0x2D // Interrupt enable register
#define REG_CTRL_REG5        0x2E // Interrupt pin (INT1/INT2) map
// User Offset
#define REG_OFF_X            0x2F // X-axis offset adjust
#define REG_OFF_Y            0x30 // Y-axis offset adjust
#define REG_OFF_Z            0x31 // Z-axis offset adjust

// Value definitions
#define BIT_TRIG_TRANS       0x20  // Transient interrupt trigger bit
#define BIT_TRIG_LNDPRT      0x10  // Landscape/Portrati Orientation
#define BIT_TRIG_PULSE       0x08  // Pulse interrupt trigger bit
#define BIT_TRIG_FF_MT       0x04  // Freefall/Motion trigger bit
#define UINT14_MAX					 16383
#define DEFAULT 						 100

MMA8451Q::MMA8451Q(PinName sda, PinName scl, int addr) : m_i2c(sda, scl), m_addr(addr<<1) {
    // activate the peripheral
    uint8_t data[2] = {REG_CTRL_REG1, 0x01};
    writeRegs(data, 2);
		accData.x=DEFAULT;
		accData.y=DEFAULT;
		accData.z=DEFAULT;
		accData.x_Max=DEFAULT;
		accData.x_Min=DEFAULT;
		accData.y_Max=DEFAULT;
		accData.y_Min=DEFAULT;
		accData.z_Max=DEFAULT;
		accData.z_Min=DEFAULT;
		count=0;
}

MMA8451Q::~MMA8451Q() { }

uint8_t MMA8451Q::getWhoAmI() {
    uint8_t who_am_i = 0;
    readRegs(REG_WHO_AM_I, &who_am_i, 1);
    return who_am_i;
}

float MMA8451Q::getAccX() {
    return (float(getAccAxis(REG_OUT_X_MSB))/4096.0);
}

float MMA8451Q::getAccY() {
    return (float(getAccAxis(REG_OUT_Y_MSB))/4096.0);
}

float MMA8451Q::getAccZ() {
    return (float(getAccAxis(REG_OUT_Z_MSB))/4096.0);
}

AccelerometerData MMA8451Q::getAccAllAxis(void) {
	if(count==120){
		reset();
	}
    accData.x = getAccX();
    accData.y = getAccY();
    accData.z = getAccZ();
	if(accData.x_Max==DEFAULT){
		accData.x_Min=accData.x;
		accData.x_Max=accData.x;
	}
	if(accData.y_Max==DEFAULT){
		accData.y_Min=accData.y;
		accData.y_Max=accData.y;
	}
	if(accData.z_Max==DEFAULT){
		accData.z_Min=accData.z;
		accData.z_Max=accData.z;
	}
	
	if(accData.x>accData.x_Max)
		accData.x_Max=accData.x;
	if(accData.x<accData.x_Min)
		accData.x_Min=accData.x;
	if(accData.y>accData.y_Max)
		accData.y_Max=accData.y;
	if(accData.y<accData.y_Min)
		accData.y_Min=accData.y;
	if(accData.z>accData.z_Max)
		accData.z_Max=accData.z;
	if(accData.z<accData.z_Min)
		accData.z_Min=accData.z;
	
	count++;
	return accData;
}

int16_t MMA8451Q::getAccAxis(uint8_t addr) {
    int16_t acc;
    uint8_t res[2];
    readRegs(addr, res, 2);

    acc = (res[0] << 6) | (res[1] >> 2);
    if (acc > UINT14_MAX/2)
        acc -= UINT14_MAX;

    return acc;
}

void MMA8451Q::readRegs(int addr, uint8_t * data, int len) {
    char t[1] = {addr};
    m_i2c.write(m_addr, t, 1, true);
    m_i2c.read(m_addr, (char *)data, len);
}

void MMA8451Q::writeRegs(uint8_t * data, int len) {
    m_i2c.write(m_addr, (char *)data, len);
}

void MMA8451Q::reset(void){
	accData.x_Max=DEFAULT;
	accData.x_Min=DEFAULT;
	accData.y_Max=DEFAULT;
	accData.y_Min=DEFAULT;
	accData.z_Max=DEFAULT;
	accData.z_Min=DEFAULT;
	count=0;
}
