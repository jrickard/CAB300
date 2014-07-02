/*  This library supports the LEM CAB series of fluxgate hall effect sensors
    It was tested on the LEM CAB300-C/SP3-000 version of this sensor.  The CAB series
    provides very low offset and very low temperature drift compared to normal Hall
    effect current sensors and can measure currents up to +/- 400A operating on automotive
    +12vdc and frame ground.  It broadcasts current data on message id 0x3C0 and does
    not require queries or acknowledgment.
    
    This library was written by Jack Rickard of EVtv - http://www.evtv.me
    copyright 2014
    You are licensed to use this library for any purpose, commercial or private, 
    without restriction.
    
*/

#include "Arduino.h"
#include "cab300.h"
#include <eeprom.h>
#include <eepromAny.h>


#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; } 


// Define the constructor.
CAB300::CAB300 ()
{

}


CAB300::~CAB300() //Define destructor
{
	EEPROM_write(AHaddress,AH);
}


void CAB300::begin(int ahadd)
{
	AHaddress=ahadd;
	EEPROM_read(AHaddress,AH); //Get our previously saved AH if any
	Serial.begin(115200);
 	//Wire.begin();
    Serial3.begin(2400);
    if (CAN.init(CAN_BPS_500K)) {Serial<<"CAN-DO=OK \n";}
	 	else {
    		Serial<<"CAN initialization (sync) ERROR \n";
    		}
	
	 //CAN.setRXFilter(0, TESTADDRESS, 0x7FF, false);
  
	CAN.setRXFilter(0x3C0, 0x7F0, false);//  Set to look for info from address 0x3C0
	 timestamp = millis(); 
	 stepcount=0;
	 debug=0;
}

double CAB300::getamps()
{
	if (CAN.rx_avail()) 
  	 	{
  	 		CAN.get_rx_buff(incoming);
  	 		calcAmperes();
  	 		if(debug) {printCAN();}
  	 		return(Amperes);
  	 	}
  	 else {return(0);}
}


void CAB300::calcAmperes()

{
 
     inbox = (uint32_t)((incoming.data.bytes[0] << 24) | (incoming.data.bytes[1] << 16) | (incoming.data.bytes[2] << 8) | (incoming.data.bytes[3] << 0));
     milliamps=inbox;
     if(milliamps>2147483648) {milliamps-=2147483648;}
       else {milliamps=(2147483648-milliamps)*-1; }
            
    Amperes=milliamps/1000;
    elapsedtime = (millis() - timestamp);    
    timestamp = millis();                       // reset timestamp to current time
    ampseconds=((elapsedtime * Amperes)/3600000); //Number of amphours since last message
    AH+=ampseconds;
    if(stepcount++ >20)     // This bit uses a step counter to write to EEPROM every 20th read.  In this way, we don't wear out our EEPROM.
    						//But our retrieved AH later will be a little short.
    	{					
    	 EEPROM_write(AHaddress,AH);
    	 stepcount=0;
    	 }
 }


void CAB300::resetAH()
{
	AH=0;
}

void CAB300::printCAN()
{

   //In debug mode, this routine simply prints a timestamp and the contents of the 
   //incoming CAN message
   
	milliseconds = (int) (millis()/1) %1000 ;
	seconds = (int) (millis() / 1000) % 60 ;
    minutes = (int) ((millis() / (1000*60)) % 60);
	hours   = (int) ((millis() / (1000*60*60)) % 24);
	sprintf(buffer,"%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);
	Serial<<buffer<<" ";
    sprintf(bigbuffer,"%02X %02X %02X %02X %02X %02X %02X %02X %02X", 
    incoming.id, incoming.data.bytes[0],incoming.data.bytes[1],incoming.data.bytes[2],
    incoming.data.bytes[3],incoming.data.bytes[4],incoming.data.bytes[5],incoming.data.bytes[6],incoming.data.bytes[7],0);
    Serial<<"Received from Current Sensor: 0x"<<bigbuffer<<"\n";
}
