/* mbed GPS Module Library
 * Copyright (c) 2008-2010, sford
 * Copyright (c) 2013, B.Adryan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
 
#include "SerialGPS.h"

SerialGPS::SerialGPS(PinName tx, PinName rx, int Baud) : _gps(tx, rx) {
    _gps.baud(Baud);    
    gpsData.longitude = 0.0;
    gpsData.latitude = 0.0;
}

int SerialGPS::sample() {
    char ns, ew, unit;
    int lock;
		gpsData.fix=0;

    while(1) {        
        getline();

        // Check if it is a GPGGA msg (matches both locked and non-locked msg)
			
        if(sscanf(msg, "GPGGA,%f,%f,%c,%f,%c,%d,%d,%f,%f,%c,%f", &time, &latitude, &ns, &longitude, &ew, &lock, &sats, &hdop, &alt, &unit, &geoid) >= 1) { 
            if(!lock) {
                time = 0.0;
                longitude = 0.0;
                latitude = 0.0;
                sats = 0;
                hdop = 0.0;
                alt = 0.0;
                geoid = 0.0;
								gpsData.fix=0;
                return 0;
            } else {
                //GPGGA format according http://aprs.gids.nl/nmea/#gga
                // time (float), lat (f), (N/S) (c), long (f), (E/W) (c), fix (d), sats (d),
                // hdop (float), altitude (float), M, geoid (float), M, , ,  
                //GPGGA,092010.000,5210.9546,N,00008.8913,E,1,07,1.3,9.7,M,47.0,M,,0000*5D
                
                if(ns == 'S') {    latitude  *= -1.0; }
                if(ew == 'W') {    longitude *= -1.0; }
                
								
								int degs=latitude/100;
								float mins = latitude-(degs*100);
								latitude=degs+(mins/60);
								degs=longitude/100;
								mins = longitude-(degs*100);
								longitude=degs+(mins/60);
								
								int h=time/10000;
								int m=(time/100)-(h*100);
								int s=time-(h*10000)-(m*100);
								int ms=(time-(h*10000)-(m*100)-s)*1000;
								if(h==23){
									h=0;
								}else{
									h++;
								}
								gpsData.h=h;
								gpsData.m=m;
								gpsData.s=s;
								gpsData.ms=ms;
								
								gpsData.fix=1;
                return 1;
            }
        }
    }
}

float SerialGPS::trunc(float v) {
    if(v < 0.0) {
        v*= -1.0;
        v = floor(v);
        v*=-1.0;
    } else {
        v = floor(v);
    }
    return v;
}

void SerialGPS::getline() {
    while(_gps.getc() != '$');    // wait for the start of a line

    for(int i=0; i<256; i++) {
        msg[i] = _gps.getc();
        if(msg[i] == '\r') {
            msg[i] = 0;
            return;
        }
    }
    error("Overflowed message limit");
}

GPSData SerialGPS::getGPSData(void){
	gpsData.latitude=latitude;
	gpsData.longitude=longitude;
	gpsData.time=time;
	gpsData.sats=sats;
	gpsData.hdop=hdop;
	gpsData.geoid=geoid;
	gpsData.alt=alt;
	return gpsData;
}

