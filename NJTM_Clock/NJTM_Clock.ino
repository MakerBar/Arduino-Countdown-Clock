#define SPEAKING_TIME 300000
//#define QUESTION_TIME 300000
#define BUTTON_HOLD_MINIMUM_TIME 200

#define COUNTDOWN_ROW 8

#define STATE_ATTRACTMODE 0
#define STATE_SPEAKING 1
//#define STATE_QUESTIONS 2

#include "Charliplexing.h"
#include "Font.h"

// TODO: Replace delays with displaying on LED's

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

byte state = STATE_ATTRACTMODE;
//boolean isPaused = false;

unsigned long benchmarkTime;
unsigned long currentTime;

void setup() {
  // This is a hacky technique to use a digital input (button) without using an external pullup resistor.
  pinMode(21, INPUT);      // Turns on internal 20K pullup resistor
  digitalWrite(21, HIGH);  // Sends 5v to pin. 
  // Now when attached button/trigger pulls pin 21 to ground, it will register as a logic low!

  //pinMode(20, OUTPUT);

  attachInterrupt(2, triggerPulled, LOW);

  LedSign::Init();  //Initializes the screen

  benchmarkTime = millis();
}

void loop() {
  currentTime = millis();

  switch (state)
  {
  case STATE_ATTRACTMODE:
    {
      //togglePause = false; // Doesn't do anything in this mode

        int len = 36;
      char text[] = "NJ TECH MEETUP   HOBOKEN MAKERBAR   ";
      int x=0,x2=0;
      for(int j=13;j>-175;j--) {
        if (abortCurrentState) break;
        x=j;
        LedSign::Clear();
        for(int i=0;i<len;i++) {
          if (abortCurrentState) break;
          x2=Font::Draw(text[i],x,-1);
          x+=x2;
          if (x>=13) break;
        }  
        if (!abortCurrentState) delay(100);
      }

      if (abortCurrentState)
      {
        LedSign::Clear();
        state = STATE_SPEAKING;
        abortCurrentState = false;
      }
      break;
    }

  case STATE_SPEAKING:
  //case STATE_QUESTIONS:
    {
      /* Lead in */
      int len = 44;
        char text[] = "PRESENTATION TIME!   5 MINS!   READY SET GO!";
        int x=0,x2=0;
        for(int j=13;j>-225;j--) {
          if (abortCurrentState) break;
          x=j;
          LedSign::Clear();
          for(int i=0;i<len;i++) {
            if (abortCurrentState) break;
            x2=Font::Draw(text[i],x,-1);
            x+=x2;
            if (x>=13) break;
          }  
          if (!abortCurrentState) delay(100);
        }
      
      /* Count down with 10 LED's */
      for (int led = 0; led < 14; led++) {
        if (abortCurrentState) break;
        LedSign::Set(led, COUNTDOWN_ROW);
      }
      for (int i = 0; i < 8; i++) {
        if (abortCurrentState) break;
        if (i > 0) Font::Draw((5 - ((i - 1) / 2) + 48), 5, -1, 0); // Erase the last character by drawing it in "black".
        Font::Draw((5 - i / 2) + 48, // The ASCII char for '0' is 48. Therefore, we add 48 to the current number of minutes left to get the char to draw. 
        5, -1);
        
        if (abortCurrentState) break;
        for (unsigned int j = 0; j < (SPEAKING_TIME - 60000) / 80; j++) if (!abortCurrentState) delay(10);
        
        LedSign::Set(i, COUNTDOWN_ROW, 0);
      }

      Font::Draw('2', 5, -1, 0); // Erase that pesky '2' left after that countdown

      /* Last minute countdown */
      for (int i = 8; i <= 13; i++) {
        for (int j = 0; j < 10; j++)
        {
          if (abortCurrentState) break;
          byte seconds = 60 - ((i - 8) * 10) - j;
          byte x2;

          if (seconds != 60)
          {
            x2 = Font::Draw((seconds + 1) / 10 + 48, 2, -1, 0);
            Font::Draw(((seconds + 1) % 10) + 48, 2 + x2, -1, 0);
          }

          x2 = Font::Draw(seconds / 10 + 48, 2, -1);
          Font::Draw((seconds % 10) + 48, 2 + x2, -1);
          if (abortCurrentState) break;
          delay(1000);
        }
        LedSign::Set(i, COUNTDOWN_ROW, 0);
      }

      LedSign::Clear();

      /* Blink '00' a few times for effect' */
      for (int i = 0; i < 5; i++)
      {
        if (abortCurrentState) break;

        Font::Draw('0', 2, -1);
        Font::Draw('0', 8, -1);

        delay(500);

        LedSign::Clear();
        if (abortCurrentState) break;

        delay(500);
      }

      /* Time's up! */
      do
      {
        int len = 40;
        char text[] = "TIME UP!   ARMING TRAP DOOR!      LOL JK";
        int x=0,x2=0;
        for(int j=13;j>-200;j--) {
          if (abortCurrentState) break;
          x=j;
          LedSign::Clear();
          for(int i=0;i<len;i++) {
            if (abortCurrentState) break;
            x2=Font::Draw(text[i],x,-1);
            x+=x2;
            if (x>=13) break;
          }  
          if (!abortCurrentState) delay(100);
        }
      } 
      while (!abortCurrentState);

      /* Fire the bell! Thie doesn't seem to work, the bell sucks so much power the Arduino disables itself.
       digitalWrite(20, HIGH);
       delay(2000);
       digitalWrite(20, LOW);
       */

      if (abortCurrentState)
      {
        LedSign::Clear();
        state = STATE_ATTRACTMODE;
        abortCurrentState = false;
      }
      break;
    }
  }
}

/*
void triggerPulled()
 {
 boolean triggerIsDown = digitalRead(21);
 unsigned long time = millis();
 
 if (triggerIsDown) buttonClickStart = time;
 else if (time - buttonClickStart < BUTTON_HOLD_MINIMUM_TIME) togglePause = true;
 else abortCurrentState = true;
 
 if (triggerIsDown && time - buttonClickStart < BUTTON_HOLD_MINIMUM_TIME)
 {
 abortCurrentState = true;
 buttonClickStart = time;
 }
 }
 */

void triggerPulled()
{
  unsigned long time = millis();

  if (time - buttonClickStart > BUTTON_HOLD_MINIMUM_TIME)
  {
    abortCurrentState = true;
    buttonClickStart = time;
  }
}








