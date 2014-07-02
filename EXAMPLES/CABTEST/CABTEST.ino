
#include <Arduino.h>
#include <due_can.h>
#include <due_wire.h>
#include <eeprom.h>
#include <eepromAny.h>
#include <cab300.h>

 
template<class T> inline Print &operator <<(Print &obj, T arg) { obj.print(arg); return obj; } 

CAB300 Sensor;//Instantiate CAB300 object as "Sensor" and set EEPROM storage address for AH to 100
char buffer[90];
double Amps;

void setup() 
{
    Sensor.begin(100);//Initialize CAB300 to store AH at EEPROM address provided
     Serial.begin(115200);
     Serial.println(" LEM CAB 300=C Startup successful.   Version 1.00");  
}

void loop()
{
  	Amps=Sensor.getamps();  //Checks for CAN messages.  You must call this periodically to receive CAN messages
		
	printimestamp();  //Prints values in milliamperes, Amperes, and accummulated ampere hours derived from CAB300 CAN messages
        Serial<<"Milliamps: "<<Sensor.milliamps<<" ";
          sprintf(buffer,"%4.2f",Sensor.Amperes); 
        Serial<<"Amps: "<<buffer<<" ";
          sprintf(buffer,"%4.3f",Sensor.AH); 
        Serial<<"Total AmpHours: "<<buffer<<" \n";  
       
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
           case 'd':     //Causes CAB300 object to print incoming CAN messages for debugging
      		if(Sensor.debug==1){Sensor.debug--;}
                  else{Sensor.debug++;}
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


