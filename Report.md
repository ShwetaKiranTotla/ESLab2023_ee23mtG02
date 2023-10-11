# Embedded Systems Lab Report

<!-- Insert your details here -->
* Divya Sharma [ee23mt005@iitdh.ac.in] 
* Shweta Totla [200030053@iitdh.ac.in]
* Group: 02 <br>
* Date 11-10-2023

### Problem Statement:
Program your microcontroller to transmit:  

"F0" if SW1 is pressed  

"AA" if SW2 is pressed  

over UART with baud rate 9600 and odd parity.  

Your program should also listen for incoming data on the UART with the same baud and parity config; if "AA" is received LED should be GREEN; if "F0" is recieved, the LED should be BLUE and if any error is detected LED should be RED. Test this by communicating with your neighboring group.  

### Solution Summary:

First a UART module (here module 5) is configured with baud rate 9600 and odd parity.  
For UART transmission, whenever any of the switches is pressed, an interrupt is generated and the corresponding interrupt handler transmits 0xF0 if SW1 is pressed and 0xAA if SW2 is pressed.  
For UART reception, whenever a data is received on the UART 5 module (which uses GPIO pins PE4 and PE5 for receive and transmit respectively), and interrupt is generated and depending on the the data is, the LED turns on. If "AA" is received LED is GREEN and if "F0" is recieved, the LED is BLUE.  
Also if there are any errors, then the LED turns RED.  

### Assumptions, Constraints and Requirements:

* Assuming that PE4 and PE5 are connected to each other for communication. In case of broken connection, an error is detected and the LED turns RED.  
* Variables are used to keep track of the number of receptions, tranmsmissions and errors in one communication cycle (till is reset occurs).  
* LEDs use a Read Modify Write (instead of Write), so if f0 is transmitted followed by aa, then the LED would be Green+Blue. To make the LED blue only, a write operation "GPIO_PORTF_DATA_R = BLUE_LED;" instead of "GPIO_PORTF_DATA_R |= BLUE_LED;" can be used.  

**Calculations**  
Bit Rate Division (BRD) Factor Calculation:
BRD = Clock_frequency/(ClkDiv * Baud Rate)  
Here ClkDiv = 16 because HSE (High Speed Enable) in UARTCTL is 0 (if it's 1, then ClkDiv=8).  
Clock_frequency = System Clock = 16 Mhz  
Baud Rate = 9600  
BRD = (16 M / 16 * 9600) = 104.1667  
IBRD = Integer(104.1667)= 104  
FBRD = Integer(Fractional(BRD) * 64 + 0.5)   
FBRD = Integer(0.1667 * 64 + 0.5 ) = 11  

### Block diagram / Flowchart:

![Flow Chart](./flow_chart.JPG)



### Measurements and Results:
Since UART transmits LSB first, 0xAA is transmitted as start bit (0), followed by A0 as LSB first (which is 0101_0101), followed by odd parity (which is 1 in this case as the parity of the data byte is even) ending with the stop byte (1). All of this is 0_0101_0101_11.  
Similarly, 0xF0 is transmitted as start bit (0), followed by F0 as LSB first (which is 0000_1111), followed by odd parity (which is 1 in this case as the parity of the data byte is even) ending with the stop byte (1). All of this is 0_0000_1111_11.  
Since UART data line is at VDD when idle, the start of transmission is seen as a transition from 1 to 0, this is followed by the payload, parity and stop bit.  
As the stop bit is 1 and the UART line goes back to 1 (or Vdd) after transmission, it is not possible to detect when the transmission is over. To record the end of transmission of this byte, a byte containing all zeroes is sent just after this.  
Each bit is expected to take 104us for transmission and the measured values match the expected values.  

**0xAA**  
![aa_1.jpg](aa_1) Start bit and first bit measurement.  
![aa_2.jpg](aa_2.jpg) Second bit measurement.  
![aa_3.jpg](aa_3.jpg) Third bit measurement. 
![aa_4.jpg](aa_4.jpg) 4th, 5th, 6th and 7th bit measurement.  
![aa_5.jpg](aa_5.jpg) 8th bit, parity and stop bit measurement.  

**0xF0**  
![f0_1.jpg](f0_1.jpg) Start bit and 1st nibble measurement.  
![f0_2.jpg](f0_1.jpg) 2nd nibble, parity and stop bit measurement.  
<br>
![0_byte.jpg](0_byte.jpg) Measurement of the 0 byte that is transmitted following the data byte.  

**LED**    
![Green LED](green_led.jpg)  
The Green LED turns on when AA is received.  
![Blue LED](blue_led.jpg)  
The Blue LED turns on when F0 is received.  
![Red LED](red_led.jpg)  
The Red LED turns on when an error occurs during the transmission.  

### Discussion and Conclusions:

UART transmits LSB first. When the byte (payload) has an even parity the parity bit (odd parity) is 1 and is 0 when the data byte has odd parity.
