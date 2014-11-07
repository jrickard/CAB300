#include <SPI.h>
#include <Arduino.h>
#include <due_can.h>
#include <due_wire.h>
#include "variant.h"
#include <Wire_EEPROM.h>
#include <cab300.h>

 
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; } 
class EEPROMvariables {
   public:
	double AH;        //Use SERIAL port if 2, CAN0 if 0 or CAN1 if 1
	int port;
        int enablepin;
        int dashnumber;
        int goodEEPROM;
};
EEPROMvariables myVars;
uint16_t page=875;
float Version=1.00;
char buffer[30];
int stepcount=0;
int startime;
int elapsedtime;
int hours;
int minutes;
int seconds;
int milliseconds;



CAB300 Sensor;  //Instantiate a CAB300 object 

void gotCABFrame(CAN_FRAME *frame) 
//This routine catches our CAN frame interrupt and reroutes the frame to our library function to 
//calculate amperes and amp-hours from the frame. In this way, we only process frames on receipt
//and do NOT need our program loop to initiate this.
{
  Sensor.calcAmperes(frame); //bounce!
}




void setup() 
{
  startime=millis();
  Wire.begin(); 
  Serial.begin(115200);
  delay(5000);
  initEEPROM(); //Load our persistant variables from EEPROM.
  Sensor.AH=myVars.AH;  //Set the sensor AH variable to our saved amp hours
  initCAN(myVars.port,myVars.dashnumber,myVars.enablepin);  //Can port, CAB300 dash number (0 to 10), enable pin (255 if none used)
  Serial<<"CAN bus initialized \n";
  Serial<<"LEM CAB300-C Startup Successful \n";
  printMenu();
}

void loop()
{
        if(stepcount++>200000)  //We're only going to print and save our values every 200,000 cycles.
          {	
	    printimestamp();  
            Serial<<"ma: "<<Sensor.milliamps<<" ";
            sprintf(buffer,"%4.2f",Sensor.Amperes); 
            Serial<<"  Amps: "<<buffer<<" ";
            sprintf(buffer,"%4.4f",Sensor.AH); 
            Serial<<" Total AmpHours: "<<buffer; 
            Serial<<"  Frame Count: "<<Sensor.framecount<<" \n";
            myVars.AH=Sensor.AH;  //Copy sensor AH into our EEPROM page
            stepcount=0; 
            EEPROM.write(page, myVars);  //Save persistent variables to EEPROM
          }
         
        checkforinput(); //Check keyboard for user input 
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
           case 'Z':    //Zeroes ampere-hours
      		startime=millis();
      		break;
	 
	   case 'f':    //Zeroes frame count
      		Sensor.framecount=0;
      		break;
        
           case 'd':     //Causes CAB300 object to print incoming CAN messages for debugging
                 Sensor.debug=!Sensor.debug;
      		break;
           case 'c':     //Change our CAN port
             getPort();
      		break;
           case 'p':     //Change our ENABLE pin for the CAN bus transciever
             getEnablepin();
      		break;
           case 'v':     //Change the CAB300 part number suffix so we know what messsage ID to use
             getVersion();
      		break;
           case '?':     //Print a menu describing these functions
             printMenu();
      		break;
         
         
	  }    
      }
}
void getPort()

{
	Serial<<"\n Enter port selection:  c0=CAN0 c1=CAN1 ";
		while(Serial.available() == 0){}               
		int P = Serial.parseInt();	
		myVars.port=P;
		if(myVars.port>1) Serial<<"Entry out of range, enter 0 or 1 \n";
                  else {initCAN(myVars.port,myVars.dashnumber,myVars.enablepin);}           
}
void getEnablepin()

{
	Serial<<"\n Enter enable pin:  p48  ";
		while(Serial.available() == 0){}               
		int P = Serial.parseInt();	
		myVars.enablepin=P;
                pinMode(myVars.enablepin,OUTPUT);
		initCAN(myVars.port,myVars.dashnumber,myVars.enablepin);           
}
void getVersion()

{
	Serial<<"\n Enter LEM CAB300-C/SP-xxx dash number:  v010  ";
		while(Serial.available() == 0){}               
		int P = Serial.parseInt();	
		myVars.dashnumber=P;
                initCAN(myVars.port,myVars.dashnumber,myVars.enablepin);           
}


void printimestamp()
{
  //Prints a timestamp to the serial port
    elapsedtime=millis() - startime;
    
    milliseconds = (elapsedtime/1) %1000 ;
    seconds = (elapsedtime / 1000) % 60 ;
    minutes = ((elapsedtime / (1000*60)) % 60);
    hours   = ((elapsedtime / (1000*60*60)) % 24);
    char buffer[19]; 
    sprintf(buffer,"%02d:%02d:%02d.%03d", hours, minutes, seconds, milliseconds);
    Serial<<buffer<<" ";
}
void initCAN(int port, int dashnumber, int enablepin)
{
 if(port==1)  //If 1, initialize Can1.  If 0, initialize Can0.
	 	{
	 	if(dashnumber==10){Can1.begin(250000,enablepin);}
	 		else {Can1.begin(500000,enablepin);}
	 	if (dashnumber==0){Can1.setRXFilter(6, 0x3C0, 0x7FF, false);}
			else {Can1.setRXFilter(6, 0x3C2, 0x7FF, false);}	
				
			Can1.setCallback(6, gotCABFrame);
		}
		
	 	else
	 		{
	 		if(dashnumber==10){Can0.begin(250000,enablepin);}
	 			else {Can0.begin(500000,enablepin);}
	 		if (dashnumber==0){Can0.setRXFilter(6, 0x3C0, 0x7FF, false);}
				else {Can0.setRXFilter(6, 0x3C2, 0x7FF, false);}	
				
			Can0.setCallback(6, gotCABFrame);
                        }
}

void initEEPROM()
{
  //This routine normally just fetches our EEPROM page containing our persistent variables.
  //But on a new equipment or with a changed page, we need to initialize the variables and save
  //them to EEPROM to get started.  We detect this using the goodEEPROM variable which we simply 
  //write the arbitrary 222 value.  If we load from a random area of EEPROM without valid variables
  //It is unlikely for this to just happen to contain the 222 value.
  
  EEPROM.setWPPin(19); 
  EEPROM.read(page, myVars);
  if(!myVars.goodEEPROM==222)
    {
      myVars.AH=0;
      myVars.port=0;
      myVars.enablepin=62;
      myVars.dashnumber=0;
      myVars.goodEEPROM=222;
      EEPROM.write(page, myVars);
    }
}
void printMenu()
{
   Serial<<"\f\n=========== LEM CAB300-C/SP3-xxx Sample Program Version "<<Version<<" ==============\n************ List of Available Commands ************\n\n";
   Serial<<"  ?  - Print this menu\n ";
   Serial<<"  Z  - zero ampere-hours\n ";
   Serial<<"  F  - zero frame count\n ";
   Serial<<"  C  - CAN bus selector\n         C0=CAN0\n         C1=CAN1\n ";
   Serial<<"  D - toggles Debug off and on to print recieved CAN data traffic\n";
   Serial<<"  P - designates ENABLE pin for CAN port i.e. p48\n";
   Serial<<"  V - designates CAB300 version number (dash number)  ie v010\n";
   Serial<<"**************************************************************\n==============================================================\n\n";
   
}

