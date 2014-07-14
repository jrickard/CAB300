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
#include "due_can"
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


void CAB300::begin(int ahadd, int canbus)

//Looking for CAB300.begin(ahadd,canbus)specifying an EEPROM address to accumulate
//ampere-hours persistently, and a CANbus to monitor for current data from the 
//CAB300.  You must use this to initialize the CAB300 to do anything useful.

{
	if(ahadd){AHaddress=ahadd;}
		else{ahadd=32000;}
	if(canbus){CANbus=canbus;}
		else{CANbus=0;}
	EEPROM_read(AHaddress,AH); //Get our previously saved AH if any
	Serial.begin(115200);
 	//Wire.begin();
    Serial3.begin(2400);
    if(CANbus) //If we received no CANbus or they set it to 0, use CAN
    	{
		if (CAN.init(CAN_BPS_500K)) 
			{
				Serial<<"CAN-DO=OK \n";
				CAN.setRXFilter(3,0x3C0, 0x7F0, false);// Look for info from address 0x3C0
				CAN.setCallback(3, calcAmperes);
				CAN.setGeneralCallback(printCAN); //Print stray CAN messages
  	
			}
	 		else {
    				Serial<<"CAN0 initialization (sync) ERROR \n";
				 }
		}	
		else  //If we received any non-zero CANbus use CAN2
			{
			if (CAN2.init(CAN_BPS_500K)) 
				{
				Serial<<"CAN-DO=OK \n";
				CAN2.setRXFilter(3, 0x3C0, 0x7F0, false);//Look for info from address 0x3C0
				CAN2.setCallback(3, calcAmperes);
				CAN2.setGeneralCallback(printCAN); //Print stray CAN messages
  				}
	 		else {
    				Serial<<"CAN1 initialization (sync) ERROR \n";
    			}
			}
	 timestamp = millis(); 
	 stepcount=0;
	 debug=0;
}

/*
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
*/

void CAB300::calcAmperes(CAN_FRAME *frame)
//This routine is invoked by interrupt any time we receive a valid CAN frame 
//from the addresses we are looking for. It calculates the local current and 
//amp-hour data from those frames and stores it in EEPROM

{
 
     inbox = (uint32_t)((frame.data.bytes[0] << 24) | (frame.data.bytes[1] << 16) | (frame.data.bytes[2] << 8) | (frame.data.bytes[3] << 0));
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
    printCAN(CAN_FRAME *frame);  //This will only print if debug anyway
 }


void CAB300::resetAH()
//A way to zero our amphours.  Could also use CAB300.AH=0
{
	AH=0;
}

void CAB300::printCAN(CAN_FRAME *frame)
{
//If debug variable is set to anything but zero, this routine prints a timestamp
//and the data contained in the CAN frame.  This is used for sought frames and 
// stray frames alike.

if(debug)
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
    frame.id, frame.data.bytes[0],frame.data.bytes[1],frame.data.bytes[2],
    frame.data.bytes[3],frame.data.bytes[4],frame.data.bytes[5],frame.data.bytes[6],frame.data.bytes[7],0);
    Serial<<"Received from Current Sensor: 0x"<<bigbuffer<<"\n";
    }
}
