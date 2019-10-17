#include <SPI.h> 
#include <SdFat.h>
#include <mcp_can.h> 
#include <mcp_can_dfs.h> 
File data;
SdFat sd;
#define BITRATE 7 //make sure the bitrate of the transmitter is the same as the reciever
MCP_CAN CAN(9); //initial pin setup for the CAN
unsigned long millisecs;
void setup () 
{
  pinMode(A0, INPUT_PULLUP); //Button Pin
  pinMode(7, OUTPUT); //LED Pin
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
//Serial.print("Initializing SD card...");
// see if the card is present and can be initialized:
  if (!sd.begin(4)) {
  //Serial.println("Card failed, or not present");
    while (1); // don't do anything more:
  }
//Serial.println("card initialized.");

  int canSSOffset = 0;

  while (CAN.begin (BITRATE) != CAN_OK)
  {
// Loop to setup the CAN, 
//incrementing by 1 each time until the right one is found
// Log error
//Serial.println("CAN BUS Shield init fail");
//Serial.print("Init CAN BUS Shield again with SS pin ");
    canSSOffset ^= 1;
//Serial . println (10 + canSSOffset ); 
    delay (100);
    CAN = MCP_CAN(10 + canSSOffset);
  }
// Log successful CAN connection
//Serial.println("CAN BUS Shield init ok!");
//Serial.println(CAN.checkReceive());
//Serial.println(CAN_MSGAVAIL);
}


void loop () {
  int check = 1; //check if the button is on
  int buttonState = digitalRead(A0);

  if (buttonState == HIGH) {
    check = 0;
  } else {
    check = 1;
  }
  millisecs = millis();
  int id1 = 0;
  byte len = 0;
  uint8_t buffer [8] = {0};
  
  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    CAN.readMsgBuf(&len ,buffer);
    id1 = CAN.getCanId();
    if (check == 1) {
      sd.begin(4);
      data = sd.open("test.txt", FILE_WRITE); //set up the file to write
      
      // if the file is available, write to it:
      if (data) {
        digitalWrite(7, HIGH); //turn led on
        data.print(id1,HEX); //write the ID of the CAN Message
        //Serial.print(id1,HEX); 
        data.print(len,HEX); //write the length of the CAN Message
        //Serial.print(len,HEX);
        
        for (int i = 0; i < len; i++) {                           
          data.print(buffer[i]); //write the actual data in the message
          //Serial.print(buffer[i]);
          }
      data.print("-");
      //Serial.print("-");
      data.print(millisecs); //write the time stamp in milliseconds when the data was recieved
      //Serial.print(millisecs);
      data.print("\n");
      //Serial.print("\n");
      data.close();
      }
    }
  // if the file isn't open or the button is closed, pop up an error:
  else {
    //Serial.println("error opening test.txt or button closed");
    if(check == 0 && data == 0) {
    digitalWrite(7, LOW); //turn off the LED
      }
    }
  }
}
