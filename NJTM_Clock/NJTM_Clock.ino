#define SPEAKING_TIME 300000
#define BUTTON_HOLD_MINIMUM_TIME 200

#define COUNTDOWN_ROW 8

#define STATE_ATTRACTMODE 0
#define STATE_SPEAKING 1

#include "Charliplexing.h"
#include "Font.h"

// TODO: Put this in PROGMEM
static char attractString[] = "NJ TECH MEETUP   HOBOKEN MAKERBAR   ";
static int attractStringLength = 36;
// TODO: Put the rest of the strings here

// Interrupts may be called during other functions.
// The volatile keyword says that the value may change at any time.
volatile unsigned long buttonClickStart;
// The following values tell the various loops to break ASAP.
volatile boolean abortCurrentState = false;
//volatile boolean togglePause = false;

//int stringOffset = 13; // Remember to reset to 13 instead of 0 so strings start from the right

//////////////////////////////////////////////////////////////////
// 7 segment display code, should be moved to a library eventually

int digitPin = 22;
int segmentPin = 26;
int dotsPin = 33;

int digits[10][7] = {
  {1,1,1,1,1,1,0}, // 0
  {0,1,1,0,0,0,0}, // 1
  {1,1,0,1,1,0,1}, // 2
  {1,1,1,1,0,0,1}, // 3
  {0,1,1,0,0,1,1}, // 4
  {1,0,1,1,0,1,1}, // 5
  {0,0,1,1,1,1,1}, // 6
  {1,1,1,0,0,0,0}, // 7
  {1,1,1,1,1,1,1}, // 8
  {1,1,1,0,0,1,1}  // 9
};

void segInitialize() {
  for (int i = digitPin; i < digitPin + 4; i++) {
    pinMode(i, OUTPUT);
  }
  for (int i = segmentPin; i < segmentPin + 7; i++) {
    pinMode(i, OUTPUT);
  }
  pinMode(dotsPin, OUTPUT);
  segReset();
}

void segReset() {
  // set all digits off (LOW)
  for (int p = digitPin; p < digitPin + 4; p++) {
    digitalWrite(p, LOW);
  }
  
  // set all segments off (HIGH)
  for (int p = segmentPin; p < segmentPin + 7; p++) {
    digitalWrite(p, HIGH);
  }
  
  // set all dots off (HIGH)
  digitalWrite(dotsPin, HIGH);
}

void fail() {
  // something is bad, turn off every pin, and infinite loop
  segReset();
  while(1) {
    delay(1000);
  }
}

void lightSeg(int digit, int segment) {
  if ((digit < 1) || (digit > 4) || (segment < 0) || (segment > 6)) {
    fail();
  }
  int digPin = digit - 1 + digitPin; // digits are 1-indexed, so subtract 1
  int segPin = segment + segmentPin; // segments are 0 indexed
  digitalWrite(digPin, HIGH);
  digitalWrite(segPin, LOW);
  digitalWrite(digPin, LOW);
  digitalWrite(segPin, HIGH);
}

void lightDig (int digit, int numeral) {
  // digit is which set of segments you want to light up
  // numeral is the number you want to display on those segments
  if ((digit > 4) || (numeral > 9)) {
    fail();
  }
  for (int seg = 0; seg < 7; seg++) {
    if (digits[numeral][seg]) {
      lightSeg(digit, seg);
    }
  }
}

void flashDots() {
  // have to make sure only one dot is on a time to ensure brightness is the same
  digitalWrite(dotsPin, LOW);
  digitalWrite(digitPin + 1, HIGH);
  digitalWrite(digitPin + 1, LOW);
  digitalWrite(digitPin + 2, HIGH);
  digitalWrite(digitPin + 2, LOW);
  digitalWrite(dotsPin, HIGH);
}

// end 7 segment display code
///////////////////////////////////////////////////////////////////

byte state = STATE_ATTRACTMODE;

unsigned long benchmarkTime;
unsigned long stateTime;

void setup() {
  // This is a hacky technique to use a digital input (button) without using an external pullup resistor.
  pinMode(21, INPUT);      // Turns on internal 20K pullup resistor
  digitalWrite(21, HIGH);  // Sends 5v to pin. 
  // Now when attached button/trigger pulls pin 21 to ground, it will register as a logic low!

  attachInterrupt(2, triggerPulled, LOW);

  LedSign::Init();  //Initializes the screen

  benchmarkTime = millis();
  
  segInitialize();
}

void loop() {
  switch (state)
  {
  case STATE_ATTRACTMODE:
    {
      int len = 36;
      char text[] = "NJ TECH MEETUP   HOBOKEN MAKERBAR   ";
      int x=0,x2=0;
      for(int j=13;j>-175;j--) {
        if (abortCurrentState) break;
        x=j;
        LedSign::Clear();
        for(int i=0;i<len;i++) {
          if (abortCurrentState) break;
          x2=Font::Draw(text[i],x,0);
          x+=x2;
          if (x>=13) break;
        }
        if (!abortCurrentState) delay(100);
      }

      if (abortCurrentState) {
        LedSign::Clear();
        state = STATE_SPEAKING;
        stateTime = millis();
        abortCurrentState = false;
      }
      break;
    }

  case STATE_SPEAKING:
    {
      // put 5 mins on the clock!
      lightDig(2, 5);
      lightDig(3, 0);
      lightDig(4, 0);
      segReset();
      
      /* Lead in */
      int len = 11;
      char text[] = "5 MINS  GO!";
      int x=0,x2=0;
      for(int j=13; j > -55; j--) {
        if (abortCurrentState) break;
        x=j;
        LedSign::Clear();
        for(int i=0;i < len;i++) {
          if (abortCurrentState) break;
          x2=Font::Draw(text[i],x,0);
          x += x2;
          if (x >= 13) break;
        }  
        if (!abortCurrentState) delay(100);
      }
      
      int leds;
      int time = 0;
      int timeRemaining = 300;
      char newChar, newChar1, newChar2;
      char oldChar = '0'; // has to be something Font::Draw can erase the first time round
      char oldChar1 = '7';
      char oldChar2 = '0';
      
      while (timeRemaining > 0) {
        if (abortCurrentState) break;
        
        // calculate timeRemaining
        time = (millis() - stateTime) / 1000;
        timeRemaining = (5*60) - time;
        
        // calculate state of bottom row of leds and display numbers on lolshield
        if (timeRemaining >= 60) { // more than a minute left
          leds = 6;
          leds += (timeRemaining - 60) / 30;
          
          // draw remaining minutes on lolshield
          newChar = (timeRemaining / 60) + '0' + 1;
          if (newChar != oldChar) {
            Font::Draw(oldChar, 5, -1, 0); // turn off oldChar
            Font::Draw(newChar, 5, -1);
            oldChar = newChar;
          }
        } else {
          // less than 1 min left
          leds = timeRemaining / 10;
          
          // draw remaining seconds on lolshield
          if (oldChar1 == '7') {
            // first time through (can't have 70 seconds in a minute)
            // erase '1' from last round
            Font::Draw(oldChar, 5, -1, 0);
          }
          newChar1 = ((timeRemaining % 60) / 10) + '0';
          newChar2 = (timeRemaining % 10) + '0';
          if ((newChar2 != oldChar2) || (newChar1 != oldChar1)) {
            // erase old charaters
            int offset = Font::Draw(oldChar1, 2, -1, 0);
            Font::Draw(oldChar2, 2 + offset, -1, 0);
            // draw new chars
            offset = Font::Draw(newChar1, 2, -1);
            Font::Draw(newChar2, offset + 2, -1);
            oldChar1 = newChar1;
            oldChar2 = newChar2;
          }
        }
        
        // display bottom row of leds
        for (int i = 0; i < 14; i++) {
          if (i <= leds) {
            LedSign::Set(13 - i, COUNTDOWN_ROW, 1);
          } else {
            LedSign::Set(13 - i, COUNTDOWN_ROW, 0);
          }
        }
        
        // update 7 segment display
        // we never use digit 1, since we're only running for 5 mins
        lightDig(2, timeRemaining / 60);
        lightDig(3, (timeRemaining % 60) / 10);
        lightDig(4, timeRemaining % 10);
        // blink the dots between digits every other second
        if ((millis() % 2000) > 1000) {
          flashDots();
        }
        segReset(); // ensure the 7 seg is fully off
        delayMicroseconds(500); // keep it off for 1 ms
      } // end timing loop

      LedSign::Clear();
      /* Blink '00' a few times for effect' */
      for (int i = 0; i < 5; i++)
      {
        if (abortCurrentState) break;

        Font::Draw('0', 2, 0);
        Font::Draw('0', 8, 0);

        for (int i = 0; i < 500; i++) {
          lightDig(1, 0);
          lightDig(2, 0);
          lightDig(3, 0);
          lightDig(4, 0);
          segReset();
          delay(1);
        }

        LedSign::Clear();
        if (abortCurrentState) break;

        delay(500);
      }

      /* Time's up! */
      do {
        int len = 11;
        char text[] = "TIME UP!   ";
        int x=0,x2=0;
        for(int j=13;j>-50;j--) {
          if (abortCurrentState) break;
          x=j;
          LedSign::Clear();
          for(int i=0;i<len;i++) {
            if (abortCurrentState) break;
            x2=Font::Draw(text[i],x,0);
            x+=x2;
            if (x>=13) break;
          }
          if (!abortCurrentState) delay(100);
        }
      } while (!abortCurrentState);

      /* Fire the bell! Thie doesn't seem to work, the bell sucks so much power the Arduino disables itself.
       digitalWrite(20, HIGH);
       delay(2000);
       digitalWrite(20, LOW);
       */

      if (abortCurrentState) {
        LedSign::Clear();
        state = STATE_ATTRACTMODE;
        stateTime = millis();
        abortCurrentState = false;
      }
      break;
    }
  }
}

void triggerPulled() {
  unsigned long time = millis();

  if (time - buttonClickStart > BUTTON_HOLD_MINIMUM_TIME) {
    abortCurrentState = true;
    buttonClickStart = time;
  }
}
