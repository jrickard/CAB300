
#include <Arduino.h>
#include <due_wire.h>
#include <due_can.h>
#include <eeprom.h>
#include <eepromAny.h>
#include <cab300.h>

 
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; } 

char buffer[30];
int stepcount=0;
int AHaddress=100;

CAB300 Sensor(100);  //Instantiate a CAB300 object


void gotCABFrame(CAN_FRAME *frame) 
{
  Sensor.calcAmperes(frame); //bounce!
}
void gotAnyFrame(CAN_FRAME *frame) 
{
  Sensor.printCAN(frame); //bounce!
}



void setup() 
{
  delay(5000);
  EEPROM_read(AHaddress,Sensor.AH);
  Serial.begin(115200);
  Serial3.begin(2400);
 
  if (CAN.init(CAN_BPS_500K)) 
	{
	Serial<<"CAN bus initialized \n";
	CAN.setRXFilter(3, 0x3C0, 0x7F0, false);// Look for info from address 0x3C0
	CAN.setCallback(3, gotCABFrame);
	CAN.setGeneralCallback(gotAnyFrame); //Print stray CAN messages

	Serial<<"LEM CAB300-C Startup Successful \n";
  	}
	else {
    		Serial<<"CAN0 initialization (sync) ERROR \n";
	     }
     
     
				
}

void loop()
{
        if(stepcount++>200000)
          {	
	    printimestamp();  
            Serial<<"Milliamps: "<<Sensor.milliamps<<" ";
            sprintf(buffer,"%4.2f",Sensor.Amperes); 
            Serial<<"Amps: "<<buffer<<" ";
            sprintf(buffer,"%4.3f",Sensor.AH); 
            Serial<<"Total AmpHours: "<<buffer; 
            Serial<<"  Frame Count: "<<Sensor.framecount<<" \n";
            EEPROM_write(AHaddress,Sensor.AH);
            stepcount=0;   
          }
         
        checkforinput(); //Check keyboard    
}
   
 

void checkforinput()

{
  
  //Checks for keyboard input from Serial Port 1 
   if (Serial.available()) 
     {
      int inByte = Serial.read();
      switch (inByte)
       	{
       	   case 'z':    //Zeroes ampere-hours
      		Sensor.AH=0;
      		break;
	  		 case 'f':    //Zeroes ampere-hours
      		Sensor.framecount=0;
      		break;
           case 'd':     //Causes CAB300 object to print incoming CAN messages for debugging
                 Sensor.debug=!Sensor.debug;
      		break;
         
	  }    
      }
}


void printimestamp()
{
  //Prints a timestamp to the serial port
    int milliseconds = (int) (millis()/1) %1000 ;
    int seconds = (int) (millis() / 1000) % 60 ;
    int minutes = (int) ((millis() / (1000*60)) % 60);
    int hours   = (int) ((millis() / (1000*60*60)) % 24);
    char buffer[19]; 
    sprintf(buffer,"%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);
    Serial<<buffer<<" ";
}


