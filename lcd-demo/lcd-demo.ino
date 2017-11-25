// this is a test sketch for the Newhaven NHD-320420WX-CoTFH-V#I041 LCD screen
// it prints 'UBC Solar' repeatedly
// David Schwartz
// 7 Oct 2017

const int pin_A0 = 2;
const int pin_WR = 3; // active low
const int pin_RD = 4; // active low
const int pin_DB0 = 5;
const int pin_DB1 = 6;
const int pin_DB2 = 7;
const int pin_DB3 = 8;
const int pin_DB4 = 9;
const int pin_DB5 = 10;
const int pin_DB6 = 11;
const int pin_DB7 = 12;
const int pin_CS = 13;
const int pin_RES = 14;

void writeByteToDataBus(byte outputByte) {
  
  digitalWrite(pin_DB0, (outputByte & B00000001) >> 0);
  digitalWrite(pin_DB1, (outputByte & B00000010) >> 1);
  digitalWrite(pin_DB2, (outputByte & B00000100) >> 2);
  digitalWrite(pin_DB3, (outputByte & B00001000) >> 3);
  digitalWrite(pin_DB4, (outputByte & B00010000) >> 4);
  digitalWrite(pin_DB5, (outputByte & B00100000) >> 5);
  digitalWrite(pin_DB6, (outputByte & B01000000) >> 6);
  digitalWrite(pin_DB7, (outputByte & B10000000) >> 7);
}

void transmitCommand(byte commandCode) {
  
  digitalWrite(pin_A0, HIGH);
  digitalWrite(pin_WR, LOW);
  writeByteToDataBus(commandCode);
  delayMicroseconds(1);
  digitalWrite(pin_WR, HIGH);
  delayMicroseconds(1);
}

void transmitCommandParameter(byte parameterCode) {
  digitalWrite(pin_A0, LOW);
  digitalWrite(pin_WR, LOW);
  writeByteToDataBus(parameterCode);
  delayMicroseconds(1);
  digitalWrite(pin_WR, HIGH);
  delayMicroseconds(1);
}

void setup() {

  pinMode(pin_A0, OUTPUT);
  pinMode(pin_WR, OUTPUT);
  pinMode(pin_RD, OUTPUT);
  pinMode(pin_DB0, OUTPUT);
  pinMode(pin_DB1, OUTPUT);
  pinMode(pin_DB2, OUTPUT);
  pinMode(pin_DB3, OUTPUT);
  pinMode(pin_DB4, OUTPUT);
  pinMode(pin_DB5, OUTPUT);
  pinMode(pin_DB6, OUTPUT);
  pinMode(pin_DB7, OUTPUT);
  pinMode(pin_CS, OUTPUT);
  pinMode(pin_RES, OUTPUT);

  digitalWrite(pin_A0, HIGH);
  digitalWrite(pin_WR, HIGH);
  digitalWrite(pin_RD, HIGH);
  digitalWrite(pin_CS, LOW);

  digitalWrite(pin_RES, LOW);
  delay(5);
  digitalWrite(pin_RES, HIGH);
  delay(10);

  // system setup (section 6-2-1 on raio datasheet)
  transmitCommand(0x40);
    delay(5);
    // 0 0 IV 1 W/S M2 M1 M0
    transmitCommandParameter(B00110100);
    // WF 0 0 0 0 FX FX FX
    transmitCommandParameter(B10000111);
    // 0 0 0 0 FY FY FY FY
    transmitCommandParameter(B00000111);
    // C/R (horizontal bytes per line)
    transmitCommandParameter(39);
    // TC/R (horizontal bytes per line, incl blanking)
    transmitCommandParameter(47);
    // L/F (lines per frame)
    transmitCommandParameter(239);
    // APL (horizontal address range, LSB)
    transmitCommandParameter(40);
    // APH (horizontal address range, MSB)
    transmitCommandParameter(0);

  // scroll parameters
  transmitCommand(0x44);
    transmitCommandParameter(0); // SAD 1L
    transmitCommandParameter(0); // SAD 1H 
    transmitCommandParameter(240); // SL1
    transmitCommandParameter(176); // SAD 2L
    transmitCommandParameter(4); // SAD 2H
    transmitCommandParameter(240); // SL2
    transmitCommandParameter(0); // SAD 3L
    transmitCommandParameter(0); // SAD 3H
    transmitCommandParameter(0); // SAD 4
    transmitCommandParameter(0); // SAD 4

  // set horizontal scroll position
  transmitCommand(0x5A);
    transmitCommandParameter(0);

  // set display overlay format
  transmitCommand(0x5B);
    transmitCommandParameter(0);

  // turn off the display and configure cursor
  transmitCommand(0x58);
    transmitCommandParameter(B01010111);

  // set cursor size & type
  transmitCommand(0x5D);
    transmitCommandParameter(B00000100);
    transmitCommandParameter(B10000100);

  // set cursor direction
  transmitCommand(0x4C);

  // turn on the display and configure cursor
  transmitCommand(0x59);
    transmitCommandParameter(B00000111);
    
  delay(5);
}

void loop() {

  // reset cursor address
  transmitCommand(0x46);
    transmitCommandParameter(0);
    transmitCommandParameter(0);
  
  // write to display memory
  transmitCommand(0x42);
  for (int i=0; i<30; i++) {

    for (int j=0; j<4; j++) {
      transmitCommandParameter(0x55); // U
      transmitCommandParameter(0x42); // B
      transmitCommandParameter(0x43); // C
      transmitCommandParameter(0x20); // 
      transmitCommandParameter(0x53); // S
      transmitCommandParameter(0x6F); // o
      transmitCommandParameter(0x6C); // l
      transmitCommandParameter(0x61); // a
      transmitCommandParameter(0x72); // r
      transmitCommandParameter(0x20); // 
    }
  }

  delay(1000);

  // reset cursor address
  transmitCommand(0x46);
    transmitCommandParameter(0);
    transmitCommandParameter(0);

  // write to display memory
  transmitCommand(0x42);
  for (int i=0; i<1200; i++) {
    transmitCommandParameter(0x20); 
  }

  delay(1000);
}
