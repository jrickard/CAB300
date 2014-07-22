This software allows you to usefully access the LEM CAB300-C/SP3
Hall effect device to measure current.  You can receive current reports
over a CAN network from an Arduino Due board with CAN shield.

The program and library relies on some other libraries, most notably the
due_can library by Collin Kidder.

Example program 2 best illustrates the use of this.  due_can receives CAN messages
but also allows you to set filters and interrupt call backs.  We set a filter for
the CAB300 message ID and when it is received we will receive an interrupt.  We pass 
the CAN frame then through a vector function back to the CAB300 library for processing.

We can then access public variables such as CAB300.AH, CAB300.Amperes, CAB300.milliamperes, and so forth at will and on our own time.

We can also set an interrupt for CAN messages received that do NOT match the filter and we've illustrated this with the gotAnyFrame function which passes the frame back to the CAB300 library for processing.

In operation, due_can throws interrupts if and only if and when a CAN message is received.  This is simply routed through our Arduino program to the CAB300 library with NO action on the part of the parent program.  CAB300 processes these received CAN messages, extracting the current and calculating ampere hours for example and storing this in variables.

If the CAB300.debug variable is set true, CAB300 will print out the received CAN frames to the screen.

The public variables in CAB300 are available all the time.  With the program loop in our Arduino program, we only access these variables every 200,000th loop.  But as many as 65 CAN frames may have been received and processed and the variables updated with each message received in that course of time.  

In this way, the CAB library not only decodes the CAN messages and calculates the results for you, but you are decoupled in time from its doing so.  You can write your program to do anything else without real interruption or overhead, with the CAN receipt and processing going on in the background automagically.

