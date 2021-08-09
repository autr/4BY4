#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "MUX74HC4067.h"

/* -------------- DEFINES -------------- */

// for HC4067

#define MUX_SIG_PIN A0
#define MUX_EN_PIN 7
#define MUX_S0_PIN 8
#define MUX_S1_PIN 9
#define MUX_S2_PIN 10
#define MUX_S3_PIN 11

// for SPI 1.8" TFT

// ~ LED
#define TFT_SCK_PIN 1  // [SCK] + Clock = Teensy2.0:1, Uno:13
#define TFT_MOSI_PIN 2  // [SDA] + MOSI + Data Output = Teensy2.0:2, Uno:11
#define TFT_DC_PIN 9 // [A0] = DC = EDITABLE ~ must be PWM
#define TFT_RST_PIN 10 // [RESET] = EDITABLE
#define TFT_CS_PIN 0 // [CS/SS/NSS] = Select = Teensy2.0:0, Uno:10
// ~ GND
// ~ VCC


// for reference

#define PLEX_COUNT 16
#define HEIGHT 130
#define WIDTH 130

/* -------------- GLOBAL OBJECTS -------------- */

int lookup[128];
int timestamps[128];
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN, TFT_SCK_PIN, TFT_RST_PIN);
// MUX74HC4067 mux(MUX_EN_PIN, MUX_S0_PIN, MUX_S1_PIN, MUX_S2_PIN, MUX_S3_PIN);

/* -------------- HELPER FUNCTIONS -------------- */

word colour( byte R, byte G, byte B){
  return ( ((R & 0xF8) << 8) | ((G & 0xFC) << 3) | (B >> 3) );
}

void drawText(char *text, int xx, int yy, uint16_t color = ST7735_WHITE) {
  tft.setCursor(xx, yy);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}

void drawText( String & str, int xx, int yy, uint16_t color = ST7735_WHITE ) {
  char copy[50];
  str.toCharArray(copy, 50);
  drawText(copy, xx,yy, color);
}  

word randomColor() {
  return colour( random(0,255), random(0,255), random(0,255));
}

/* -------------- SETUP -------------- */

void setup() {
  Serial.begin(9600);
  Serial.print(F("setup"));

  for(int i = 0; i < sizeof(lookup); ++i) lookup[i] = 0;
  
  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(3);
  
//  mux.signalPin(MUX_SIG_PIN, INPUT, ANALOG);
}

/* -------------- LOOP -------------- */

void loop() {
  //generate a random color
  int r = random(0, 255);
  int g = random (0, 255);
  int b = random (0, 255);

  int SQRT = sqrt(PLEX_COUNT);
  int LEN = min(WIDTH,HEIGHT);
  
  for (int x = 0; x < SQRT; x++) {
    for (int y = 0; y < SQRT; y++) {
      int ww = WIDTH / SQRT;
      int hh = HEIGHT / SQRT;
      int xx = ww * x;
      int yy = hh * y;
      int idx = x + (y * SQRT);
      int cx = xx + (ww/2);
      int cy = yy + (hh/2);
      String str = String(idx) ;
      tft.drawCircle(cx,cy, ww/3, randomColor());
      tft.drawRect(xx,yy,ww,hh, randomColor());
      drawText(str, cx, cy );
    }
  }

//  
//  int data;
//
//  for (byte i = 0; i < 4; ++i)
//  {
//    // Reads from channel i. Returns a value from 0 to 1023
//    data = mux.read(i);
//    int v = data;
//    Serial.print(i);
//    Serial.print(" ");
//    Serial.print(v);
//    Serial.print(" - ");
//  }
//    delay( 100 );
//  Serial.println("  ");


}
