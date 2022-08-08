#include <SdFat.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>

File data;
SdFat sd;

MCP_CAN CAN(9);

unsigned long millisecs;
int previous_state;

void setup()
{
  pinMode(A0, INPUT_PULLUP); // Button Pin
  pinMode(7, OUTPUT);        // LED Pin

  // Record toggle switch initial state
  if (digitalRead(A0) == HIGH)
  {
    previous_state = 1;
  }
  else
  {
    previous_state = 0;
  }

  // see if the card is present and can be initialized:
  if (!sd.begin(4))
  {
    while (1)
      ; // don't do anything more:
  }

  // Write when car turns on
  data = sd.open("data.txt", FILE_WRITE); // set up the file to write
  if (data)
  {
    data.print("\n\n*** POWER ON\n");
    data.close();
  }

  int canSSOffset = 0;

  while (CAN.begin(CAN_500KBPS) != CAN_OK)
  {
    canSSOffset ^= 1;
    delay(100);
    CAN = MCP_CAN(10 + canSSOffset);
  }

  // Initialize Filters
  CAN.init_Mask(0, 0, 0x7F8);
  CAN.init_Filt(0, 0, 0x620);

  CAN.init_Mask(1, 0, 0x7F4);
  CAN.init_Filt(2, 0, 0x500);
}

void loop()
{
  // SD Log switch variables
  int check = 1; // check if the button is on
  int buttonState = digitalRead(A0);

  // Check SD Log switch
  // If the switch has changed state, write to the file that it has been toggled on or off
  // Switch is on
  if (buttonState == HIGH)
  {
    check = 1;
    if (previous_state != check)
    {
      data = sd.open("data.txt", FILE_WRITE); // set up the file to write
      if (data)
      {
        data.print("*** SWITCH ON\n");
        data.close();
      }
      previous_state = 1;
    }
  }
  // Switch is off
  else
  {
    check = 0;
    if (previous_state != check)
    {
      data = sd.open("data.txt", FILE_WRITE); // set up the file to write
      if (data)
      {
        data.print("*** SWITCH OFF\n");
        data.close();
      }
      previous_state = 0;
    }
  }

  // Data writing variables
  millisecs = millis();
  int id1 = 0;
  byte len = 0;
  uint8_t buffer[8] = {0};

  // Wait for CAN message to populate buffer
  if (CAN.checkReceive() == CAN_MSGAVAIL)
  {
    // Store data from CAN header (id, length, data bytes 0-7)
    CAN.readMsgBuf(&len, buffer);
    id1 = CAN.getCanId();

    // Do not log speciifc IDs
    //      0x500: motor name
    //      0x620: battery name
    //      0x621: battery revision
    if (id1 == 0x500 || id1 == 0x620 || id1 == 0x621)
    {
      // ignore
    }
    // Check if SD Log switch is toggled ON
    else if (check == 1)
    {
      sd.begin(4);
      data = sd.open("data.txt", FILE_WRITE); // set up the file to write
      if (data)
      {
        digitalWrite(7, HIGH);

        // Count number of hex digits in the timestamp
        unsigned long temp_time = millisecs;
        int digits = 1;
        while ((temp_time >> 0x4) != 0x0)
        {
          digits++;
          temp_time = temp_time >> 0x4;
        }
        // Out of 8 characters, fill the non-timestamp characters with 'D'
        // e.g., timestamp is 0xABCD then the placeholder is 'DDDD'
        for (int i = 0; i < 8 - digits; i++)
        {
          data.print('N');
        }

        // Print data
        // Format: timestamp - id - length - date bytes 0-7
        // DDD12345 - 626 - 8 - 01 02 03 04 05 06 07 08
        data.print(millisecs, HEX);
        data.print(" - ");
        data.print(id1, HEX);
        data.print(" - ");
        data.print(len, HEX);
        data.print(" - ");

        // Print data bytes to be formatted as 2 hex characters with a space in between
        // E.g., if the nuber is 0x3 then place a zero in front "03"
        for (int i = 0; i < len; i++)
        {
          if (buffer[i] <= 0xF)
          {
            data.print("0");
            data.print(buffer[i], HEX);
          }
          else
          {
            data.print(buffer[i], HEX);
          }
          data.print(" ");
        }

        data.print("\n");
        data.close();
      }
    }
    // if the file isn't open or the button is closed, pop up an error:
    else
    {
      if (check == 0 && data == 0)
      {
        digitalWrite(7, LOW); // turn off the LED
      }
    }
  }
}