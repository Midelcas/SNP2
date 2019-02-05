//gps.cpp
//for use with Adafruit Ultimate GPS
//Reads in and parses GPS data

#include "mbed.h"
#include "SerialGPS.h"
#include "rtos.h"

#define USART1_TX PA_9
#define USART1_RX PA_10
#define GPS_BAUD 	9600
#define GPS_ENABLE PB_13


//SerialGPS myGPS(PA_9,PA_10, 9600);
SerialGPS myGPS(USART1_TX,USART1_RX, GPS_BAUD);
GPSData	B1gps;
Thread threadGPS(osPriorityNormal, 1024); // 1K stack size

void GPS_thread();


void GPS_thread(){
   while (true){
		 myGPS.sample();
		 B1gps = myGPS.getGPSData();
		 threadGPS.signal_wait(0x2);	
	 } 	
}







