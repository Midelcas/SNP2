/**
 * Copyright (c) 2017, Arm Limited and affiliates.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>

#include "lorawan/LoRaWANInterface.h"
#include "lorawan/system/lorawan_data_structures.h"
#include "events/EventQueue.h"

// Application helpers
#include "DummySensor.h"
#include "trace_helper.h"
#include "lora_radio_helper.h"
#include "mbed.h"
#include "HW5P-1.h"
#include "SOIL.h"
#include "TCS34725.h"
#include "MMA8451Q.h"
#include "Si7021.h"
#include "SerialGPS.h"

#define TCS_ADDR 0x52
#define MMA_ADDR 0x1D
#define SI7_ADDR 0x50
#define GREEN				5
#define RED					3
#define LEDOFF			7

using namespace events;

BusOut leds(PH_0, PH_1, PB_13);

LightData lightData;
SoilData soilData;
ColorData colorData;
AccelerometerData accData;
AmbientData ambData;

HW5P_1 *lightSensor=0;
SOIL *soilSensor=0;
TCS34725 *tcs=0;
MMA8451Q *acc=0;
Si7021 *amb=0;

extern Thread threadGPS;
extern void GPS_thread();
extern GPSData B1gps;


// Max payload size can be LORAMAC_PHY_MAXPAYLOAD.
// This example only communicates with much shorter messages (<30 bytes).
// If longer messages are used, these buffers must be changed accordingly.
uint8_t tx_buffer[30];
uint8_t rx_buffer[30];

/*
 * Sets up an application dependent transmission timer in ms. Used only when Duty Cycling is off for testing
 */
#define TX_TIMER                        10000

/**
 * Maximum number of events for the event queue.
 * 10 is the safe number for the stack events, however, if application
 * also uses the queue for whatever purposes, this number should be increased.
 */
#define MAX_NUMBER_OF_EVENTS            10

/**
 * Maximum number of retries for CONFIRMED messages before giving up
 */
#define CONFIRMED_MSG_RETRY_COUNTER     3

/**
 * Dummy pin for dummy sensor
 */
#define PC_9                            0

/**
 * Dummy sensor class object
 */
DS1820  ds1820(PC_9);

/**
* This event queue is the global event queue for both the
* application and stack. To conserve memory, the stack is designed to run
* in the same thread as the application and the application is responsible for
* providing an event queue to the stack that will be used for ISR deferment as
* well as application information event queuing.
*/
static EventQueue ev_queue(MAX_NUMBER_OF_EVENTS * EVENTS_EVENT_SIZE);

/**
 * Event handler.
 *
 * This will be passed to the LoRaWAN stack to queue events for the
 * application which in turn drive the application.
 */
static void lora_event_handler(lorawan_event_t event);

/**
 * Constructing Mbed LoRaWANInterface and passing it down the radio object.
 */
static LoRaWANInterface lorawan(radio);

/**
 * Application specific callbacks
 */
static lorawan_app_callbacks_t callbacks;

void setup_sensors(void){
	leds=LEDOFF;
	threadGPS.start(GPS_thread);
	lightSensor=new HW5P_1(PA_4);
	soilSensor=new SOIL(PA_0);
	tcs = new TCS34725(PB_9,PB_8,TCS_ADDR, PB_14);
	acc = new MMA8451Q(PB_9,PB_8,MMA_ADDR);
	amb = new Si7021(PB_9,PB_8,SI7_ADDR);
	
	if(tcs->checkId()){
		tcs->initColorSensor();
	}
}

void measure (void){
	threadGPS.signal_set(0x2);
	wait(1);
	soilData=soilSensor->measure();
	lightData=lightSensor->measure();
	colorData = tcs->readRegisters();
	wait(0.1);
	accData = acc->getAccAllAxis();
	wait(0.1);
	ambData = amb->measure();
	printf("HUM:%f", ambData.humidity);
}


/**
 * Entry point for application
 */
int main (void)
{
		setup_sensors();
	
    // setup tracing
    setup_trace();

    // stores the status of a call to LoRaWAN protocol
    lorawan_status_t retcode;

    // Initialize LoRaWAN stack
    if (lorawan.initialize(&ev_queue) != LORAWAN_STATUS_OK) {
        printf("\r\n LoRa initialization failed! \r\n");
        return -1;
    }

    printf("\r\n Mbed LoRaWANStack initialized \r\n");

    // prepare application callbacks
    callbacks.events = mbed::callback(lora_event_handler);
    lorawan.add_app_callbacks(&callbacks);

    // Set number of retries in case of CONFIRMED messages
    if (lorawan.set_confirmed_msg_retries(CONFIRMED_MSG_RETRY_COUNTER)
                                          != LORAWAN_STATUS_OK) {
        printf("\r\n set_confirmed_msg_retries failed! \r\n\r\n");
        return -1;
    }

    printf("\r\n CONFIRMED message retries : %d \r\n",
           CONFIRMED_MSG_RETRY_COUNTER);

    // Enable adaptive data rate
    if (lorawan.enable_adaptive_datarate() != LORAWAN_STATUS_OK) {
        printf("\r\n enable_adaptive_datarate failed! \r\n");
        return -1;
    }

    printf("\r\n Adaptive data  rate (ADR) - Enabled \r\n");
		

    retcode = lorawan.connect();

    if (retcode == LORAWAN_STATUS_OK ||
        retcode == LORAWAN_STATUS_CONNECT_IN_PROGRESS) {
    } else {
        printf("\r\n Connection error, code = %d \r\n", retcode);
        return -1;
    }

    printf("\r\n Connection - In Progress ...\r\n");

    // make your event queue dispatching events forever
    ev_queue.dispatch_forever();

    return 0;
}


void prepare_tx_buffer(void){
	int16_t temp_tx = ambData.temperature*100;
	tx_buffer[0]=(char)((temp_tx&0xff00)>>8);
	tx_buffer[1]=(char)(temp_tx&0x00ff);

	int16_t hum_tx = ambData.humidity*100;
	tx_buffer[2]=(char)((hum_tx&0xff00)>>8);
	tx_buffer[3]=(char)(hum_tx&0x00ff);
	
	int16_t light_tx = lightData.light*100;
	tx_buffer[4]=(char)((light_tx&0xff00)>>8);
	tx_buffer[5]=(char)(light_tx&0x00ff);
	
	int16_t soil_tx = soilData.soil*100;
	tx_buffer[6]=(char)((soil_tx&0xff00)>>8);
	tx_buffer[7]=(char)(soil_tx&0x00ff);
	
	int16_t x_tx = accData.x*100;
	tx_buffer[8]=(char)((x_tx&0xff00)>>8);
	tx_buffer[9]=(char)(x_tx&0x00ff);
	
	int16_t y_tx = accData.y*100;
	tx_buffer[10]=(char)((y_tx&0xff00)>>8);
	tx_buffer[11]=(char)(y_tx&0x00ff);
	
	int16_t z_tx = accData.z*100;
	tx_buffer[12]=(char)((z_tx&0xff00)>>8);
	tx_buffer[13]=(char)(z_tx&0x00ff);
	
	uint16_t clear_tx = colorData.clear_value;
	tx_buffer[14]=(char)((clear_tx&0xff00)>>8);
	tx_buffer[15]=(char)(clear_tx&0x00ff);
	
	uint16_t red_tx = colorData.red_value;
	tx_buffer[16]=(char)((red_tx&0xff00)>>8);
	tx_buffer[17]=(char)(red_tx&0x00ff);
	
	uint16_t green_tx = colorData.green_value;
	tx_buffer[18]=(char)((green_tx&0xff00)>>8);
	tx_buffer[19]=(char)(green_tx&0x00ff);
	
	uint16_t blue_tx = colorData.blue_value;
	tx_buffer[20]=(char)((blue_tx&0xff00)>>8);
	tx_buffer[21]=(char)(blue_tx&0x00ff);
	
	uint8_t bytes1[4];
	uint8_t bytes2[4];
	*(float*)(bytes1) = B1gps.latitude;
	*(float*)(bytes2) = B1gps.longitude;
	tx_buffer[22] = bytes1[0];
	tx_buffer[23] = bytes1[1];
	tx_buffer[24] = bytes1[2];
	tx_buffer[25] = bytes1[3];
	tx_buffer[26] = bytes2[0];
	tx_buffer[27] = bytes2[1];
	tx_buffer[28] = bytes2[2];
	tx_buffer[29] = bytes2[3];
	for(int i=0; i<sizeof(tx_buffer); i++){
			printf("%02x", tx_buffer[i]);
	}
}

/**
 * Sends a message to the Network Server
 */
static void send_message()
{
		int16_t retcode;
		uint16_t packet_len=30;
		measure();
    prepare_tx_buffer();

    retcode = lorawan.send(MBED_CONF_LORA_APP_PORT, tx_buffer, packet_len,
                           MSG_CONFIRMED_FLAG);

    if (retcode < 0) {
        retcode == LORAWAN_STATUS_WOULD_BLOCK ? printf("send - WOULD BLOCK\r\n")
                : printf("\r\n send() - Error code %d \r\n", retcode);
        return;
    }

    printf("\r\n %d bytes scheduled for transmission \r\n", retcode);
    memset(tx_buffer, 0, sizeof(tx_buffer));
}

/**
 * Receive a message from the Network Server
 */
static void receive_message()
{
    int16_t retcode;
		uint8_t red[3]={'R','e','d'};
		uint8_t green[6]={'G','r','e','e','n'};
		uint8_t off[3]={'O','F','F'};
    retcode = lorawan.receive(MBED_CONF_LORA_APP_PORT, rx_buffer,
                              sizeof(rx_buffer),
                              MSG_CONFIRMED_FLAG|MSG_UNCONFIRMED_FLAG);

    if (retcode < 0) {
        printf("\r\n receive() - Error code %d \r\n", retcode);
        return;
    }

    printf(" Data:");

    for (uint8_t i = 0; i < retcode; i++) {
        printf("%c", rx_buffer[i]);
    }
		if(memcmp(red, rx_buffer, 3)==0){
			leds=RED;
		}else if(memcmp(off, rx_buffer, 3)==0){
			leds=LEDOFF;
		}else if(memcmp(green, rx_buffer, 6)==0){
			leds=GREEN;
		}
    printf("\r\n Data Length: %d\r\n", retcode);

    memset(rx_buffer, 0, sizeof(rx_buffer));
}

/**
 * Event handler
 */
static void lora_event_handler(lorawan_event_t event)
{
    switch (event) {
        case CONNECTED:
            printf("\r\n Connection - Successful \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            } else {
                ev_queue.call_every(TX_TIMER, send_message);
            }

            break;
        case DISCONNECTED:
            ev_queue.break_dispatch();
            printf("\r\n Disconnected Successfully \r\n");
            break;
        case TX_DONE:
            printf("\r\n Message Sent to Network Server \r\n");
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        case TX_TIMEOUT:
        case TX_ERROR:
        case TX_CRYPTO_ERROR:
        case TX_SCHEDULING_ERROR:
            printf("\r\n Transmission Error - EventCode = %d \r\n", event);
            // try again
            if (MBED_CONF_LORA_DUTY_CYCLE_ON) {
                send_message();
            }
            break;
        case RX_DONE:
            printf("\r\n Received message from Network Server \r\n");
            receive_message();
            break;
        case RX_TIMEOUT:
        case RX_ERROR:
            printf("\r\n Error in reception - Code = %d \r\n", event);
            break;
        case JOIN_FAILURE:
            printf("\r\n OTAA Failed - Check Keys \r\n");
            break;
        default:
            MBED_ASSERT("Unknown Event");
    }
}

// EOF
