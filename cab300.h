#ifndef CAB300_h
#define CAB300_h

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
    
    

#include "variant.h"
#include <due_can.h>

#define SWAP_UINT32(x) (((x) >> 24) | (((x) & 0x00FF0000) >> 8) | (((x) & 0x0000FF00) << 8) | ((x) << 24))


class CAB300 : public CANListener
{
	public:
		CAB300();
       ~CAB300();
				
		void resetAH();
		double Amperes;       // Floating point with current in Amperes
		double AH;            //Floating point with accumulated ampere-hours
		int debug;
		int framecount;
		unsigned long timestamp;
		double milliamps;
		void calcAmperes(CAN_FRAME *frame);
		void printCAN(CAN_FRAME *frame);
		void gotFrame(CAN_FRAME *frame, int mailbox); //overrides the parent version so we can actually do something
		

	private:
      	CAN_FRAME  frame;
		unsigned long elapsedtime;
		int AHaddress;						//Address in EEPROM to store AH.
		
		double  ampseconds;
		int stepcount;
		int milliseconds ;
		int seconds;
		int minutes;
		int hours;
		char buffer[9];
		char bigbuffer[90];
		uint32_t inbox;
		
		
		
};

#endif /* CAB300_h */
