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

#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; } 


// Define the constructor.
CAB300::CAB300(uint8_t port, uint8_t dashno)
{
	
	 timestamp = millis(); 
	 stepcount=0;
	 debug=0;
	 framecount=0;
	 if(port==1)  //If 1, initialize Can1.  If 0, initialize Can0.
	 	{
	 	if(dashno==10){Can1.begin(250000,48);}
	 		else {Can1.begin(500000,48);}
	 	if (dashno==0){Can1.setRXFilter(6, 0x3C0, 0x7FF, false);}
			else {Can1.setRXFilter(6, 0x3C2, 0x7FF, false);}	
				
		Can1.attachObj(this);
  		Can1.attachMBHandler(6);	
        Serial<<"\nCAN bus initialized on port "<<port<<" using enable pin 48 for CAB300 
             dash "<<dashno<<" \n";
 
		}
		
	 	else
	 		{
	 		if(dashno==10){Can0.begin(250000,50);}
	 			else {Can0.begin(500000,50);}
	 		if (dashno==0){Can0.setRXFilter(6, 0x3C0, 0x7FF, false);}
				else {Can0.setRXFilter(6, 0x3C2, 0x7FF, false);}	
			Can0.attachObj(this);
    		Can0.attachMBHandler(6);	
            Serial<<"\nCAN bus initialized on port "<<port<<"  using enable pin 50
              for CAB300 dash "<<dashno<<" \n";
            }
	
}

//Define destructor
CAB300::~CAB300() 
{
}

void CAB300::gotFrame(CAN_FRAME* frame, int mailbox) //Overrides gotFrame in parent CANlistener
{
    this->calcAmperes(frame);
}

void CAB300::calcAmperes(CAN_FRAME *frame)

{	
	framecount++;
 	stepcount++;
     inbox = (uint32_t)((frame->data.bytes[0] << 24) | (frame->data.bytes[1] << 16) | (frame->data.bytes[2] << 8) | (frame->data.bytes[3] << 0));
     milliamps=inbox;
     if(milliamps>2147483648) {milliamps-=2147483648;}
       else {milliamps=(2147483648-milliamps)*-1; }
            
    Amperes=milliamps/1000;
    elapsedtime = (millis() - timestamp);    
    timestamp = millis();                       // reset timestamp to current time
    ampseconds=((elapsedtime * Amperes)/3600000); //Number of amphours since last message
    AH+=ampseconds;
    printCAN(frame);
 }


void CAB300::resetAH()
{
	AH=0;
}

void CAB300::printCAN(CAN_FRAME *frame)
{
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
    frame->id, frame->data.bytes[0],frame->data.bytes[1],frame->data.bytes[2],
    frame->data.bytes[3],frame->data.bytes[4],frame->data.bytes[5],frame->data.bytes[6],frame->data.bytes[7],0);
    Serial<<"Received CAN frame: 0x"<<bigbuffer<<"\n";
    }
}
