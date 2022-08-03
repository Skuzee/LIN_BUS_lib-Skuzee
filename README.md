# About This Project:
*** Since I've converted this to an Arduino library, I have not been able to test it on real hardware.***  
This is a LIN BUS decoder library I wrote for use with a MCP2003-E/P. It should work with other TTL level UART based transceivers.  
It uses hardware serial to check for a Frame Error in Register UCSR1A, so it might not work with all types of ARM processors. (I am using ATMEGA32U4)    
Does not have a transmit function yet.  