#include "Charliplexing.h"
#include "Font.h"

static int COUNTDOWN_ROW = 8;

void setup() {
  LedSign::Init();  //Initializes the screen
  for (int led = 0; led < 14; led++) {
    LedSign::Set(led, COUNTDOWN_ROW);
  }
  for (int i = 0; i < 8; i++) {
    delay(30000);
    LedSign::Set(i, COUNTDOWN_ROW, 0);
  }
  for (int i = 8; i <= 13; i++) {
    delay(10000);
    LedSign::Set(i, COUNTDOWN_ROW, 0);
  }
}

void loop() {
  for (int i = 0; i <= 10; i++) {
    for (int row = 0; row <= 9; row++) {
      for (int col = 0; col <= 14; col++) {
        LedSign::Set(col, row, 1);
      }
    }
    delay(100);
    for (int row = 0; row <= 9; row++) {
      for (int col = 0; col <= 14; col++) {
        LedSign::Set(col, row, 0);
      }
    }
    delay(100);
  }
  
  int len = 7;
  char text[] = "TIME UP";
  int8_t x=0,x2=0;
  for(int8_t j=13;j>-100;j--) {
    x=j;
    LedSign::Clear();
    for(int i=0;i<len;i++) {
      x2=Font::Draw(text[i],x,0);
      x+=x2;
      if (x>=13) break;
    }  
    delay(100);
  }
}
