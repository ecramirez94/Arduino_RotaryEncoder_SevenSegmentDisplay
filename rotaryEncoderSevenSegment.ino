#define encClk 2  // ATMega's first hardware interrupt pin
#define encButton 3 // ATMega's second hardware interrupt pin
#define encQuad 4 // Attach quadrature pin to another DI/O pin
#define latchPin 5  // Shift register latch pin
#define dataPin 6 // Data pin 
#define clockPin 7  // Clock pin

volatile int decCount = 0;  // Global variables used in ISR's must
volatile int onesCount = 0; // be volatile.
volatile int tensCount = 0; 
byte numArray[10]; // Contains the BCD reprentation of 0-9
byte digitArray[3]; // Selects which digit to display
byte output; // Output to shift register and BCD driver

void setup() {

  pinMode(encClk, INPUT_PULLUP);  // Alternatively could use an external 10K
  pinMode(encQuad, INPUT_PULLUP); // pullup and set as INPUT
  pinMode(encButton, INPUT_PULLUP);
  pinMode(latchPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(encClk), rotaryEncISR, RISING); // Declare ISR pins and functions
  attachInterrupt(digitalPinToInterrupt(encButton), buttonISR, FALLING);

  numArray[0] = 0x00;  //0b00000000
  numArray[1] = 0x80;  //0b00010000 --> 0b10000000
  numArray[2] = 0x40;  //0b00100000 --> 0b01000000
  numArray[3] = 0xC0;  //0b00110000 --> 0b11000000
  numArray[4] = 0x20;  //0b01000000 --> 0b00100000
  numArray[5] = 0xA0;  //0b01010000 --> 0b10100000
  numArray[6] = 0x60;  //0b01100000 
  numArray[7] = 0xE0;  //0b01110000 --> 0b11100000
  numArray[8] = 0x10;  //0b10000000 --> 0b00010000
  numArray[9] = 0x90;  //0b10010000
  // The BCD representation of each digit must be inverted in the upper nibble
  // to preserve endianness since the bits are being shifted out LSB first. This allows
  // for A -> A, B -> B, etc. connection between the pins of the shift register
  // and BCD 7-seg driver.
  
  digitArray[0] = 0x02; //0b00000010
  digitArray[1] = 0x05; //0b00000101 --> MSbit selects the segment, LSbit lights the decimal dot
  digitArray[2] = 0x08; //0b00001000
  
  // The upper four bits of the numArray[] are meaningful and the lower four bits of
  // the digitArray[] are meaningful. This allows for a fast logical OR operation
  // between the two bytes for multiplexing. Addition would also work.

}

void loop() 
{
  displayOut();
}

void rotaryEncISR(void) 
{
  if (digitalRead(encClk) == digitalRead(encQuad))  // If both are HIGH, then are in phase 
  {                                                 // and rotation is clockwise
    countUp();
  }
  else if (digitalRead(encClk) != digitalRead(encQuad)) // If encClk is HIGH and encQuad is LOW
  {                                                     // then they are in quadrature or 90 deg.
    countDown();                                        // out of phase and rotation is counter-
  }                                                     // clockwise
}

void buttonISR(void)
{
  // Put code to run when the button on the rotary encoder is pressed
}

void countUp(void) 
{
  decCount++; // Increment the least significant digit
  if (decCount > 9) // Once it exceeds 9
  {
    decCount = 0; // Roll over to 0
    onesCount++;  // Increment next digit
  }
  if (onesCount > 9)  // Same deal
  {
    decCount = 0;
    onesCount = 0;
    tensCount++;
  }
  if (tensCount > 9)  // And again
  {
    tensCount = 0;
  }
  if (decCount > 9 && onesCount == 9 && tensCount == 9) // Once the maximum number
  {                                                     // 99.9 is surpassed, roll 
    decCount = 0;                                       // them all over and start 
    onesCount = 0;                                      // again
    tensCount = 0;
  }
}

void countDown(void)  // Everything the same as countUp() but backwards
{
  decCount--; 
  if (decCount < 0)
  {
    decCount = 9;
    onesCount--;
  }
  if (onesCount < 0)
  {
    decCount = 9;
    onesCount = 9;
    tensCount--;
  }
  if (tensCount < 0)
  {
    tensCount = 9;
  }
  if (decCount < 0 && onesCount == 0 && tensCount == 0)
  {
    decCount = 9;
    onesCount = 9;
    tensCount = 9;
  }
}

void displayOut(void)  // Multiplexing routine
{
  output = numArray[decCount] | digitArray[0];  // Take logical OR between the two arrays to combine them
  digitalWrite(latchPin, LOW);  // Latch pin of shift register is active LOW
  shiftOut(dataPin, clockPin, LSBFIRST, output);  // Ship it out
  digitalWrite(latchPin, HIGH); // Lock it in place

  output = numArray[onesCount] | digitArray[1]; // Repeat for next digit
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, output);
  digitalWrite(latchPin, HIGH);

  output = numArray[tensCount] | digitArray[2]; // And for this one too
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, LSBFIRST, output);
  digitalWrite(latchPin, HIGH);
}
