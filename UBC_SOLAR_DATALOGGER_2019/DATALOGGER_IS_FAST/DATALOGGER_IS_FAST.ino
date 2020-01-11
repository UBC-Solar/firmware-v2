#include <SPI.h> 
#include <SdFat.h>

File data;
SdFat sd;
#define BITRATE 7 //make sure the bitrate of the transmitter is the same as the reciever
//MCP_CAN CAN(9); //initial pin setup for the CAN
unsigned long millisecs;

void setup () 
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
    //Serial.print("Initializing SD card...");
    // see if the card is present and can be initialized:
  if (!sd.begin(10)) {
    //Serial.println("Card failed, or not present");
    while (1); // don't do anything more:
  }
    //Turn the LED OFF initially 
   digitalWrite(LED_BUILTIN, LOW);
}


void loop () {
  //start reading the time 
  millisecs = millis();
    
  int id1 = 0;
  byte len = 0;

  id1 = 10; //random for testing 
  len = 8;
  char buffer [8] = "SD";
  sd.begin(10);
  data = sd.open("test_speed_faster.txt", O_CREAT | O_WRITE); //set up the file to write
     
  if (data) {
   // digitalWrite(13, HIGH); //turn led on - SHOULD BE 13?
   // data.print(id1,DEC); //write the ID of the CAN Message
   // Serial.print(id1,DEC); 
   // data.print(len,DEC); //write the length of the CAN Message
   // Serial.print(len,DEC);
    
    //  for (int i = 0; i < len; i++) {                           
    //  data.print(buffer[i]); //write the actual data in the message
    //  Serial.print(buffer[i]);
    //     }
    //  data.print("-");
    //  Serial.print("-");
    
       unsigned long millisecs1 = millis();
       while((millisecs1-millisecs)<=10000){
          millisecs1 = millis(); 
          data.print(millisecs1); //write the time stamp in milliseconds when the data was recieved
         // Serial.print(millisecs1);
          }
        data.close();
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  // if the file isn't open or the button is closed, pop up an error:
  else {
    Serial.print("error opening test.txt or button closed\n");
    }

} 
