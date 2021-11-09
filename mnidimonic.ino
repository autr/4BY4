#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "MUX74HC4067.h"

/* -------------- READ -------------- */

// http://adam-meyer.com/arduino/CD74HC4067

/* -------------- DEBUG -------------- */

#define DO_LCD
#define DO_MUX
#define DO_ROT
//#define DO_PRINT

/* -------------- MUX HC4067 -------------- */

// for HC4067

#define MUX_SIG_PIN A0
#define MUX_EN_PIN 7 // D7

#define MUX_S3_PIN 6
#define MUX_S2_PIN 5
#define MUX_S1_PIN 4
#define MUX_S0_PIN 3


#ifdef DO_MUX
  MUX74HC4067 mux(MUX_EN_PIN, MUX_S0_PIN, MUX_S1_PIN, MUX_S2_PIN, MUX_S3_PIN);
#endif


/* -------------- TFT SPI 1.8" -------------- */
// for SPI 1.8" TFT

// # LED
#define TFT_SCK_PIN 13  // [SCK] + Clock = Teensy2.0:1, Uno:D13
#define TFT_MOSI_PIN 11  // [SDA] +  MOSI + Data Output = Teensy2.0:2, Uno:D11
#define TFT_DC_PIN 9 // [A0] = DC = EDITABLE ~ must be PWM Uno:D9
#define TFT_RST_PIN 99 // [RESET] = EDITABLE:12 OR ARDUINO RESET PIN
#define TFT_CS_PIN 10 // [CS/SS/NSS] = Select = Teensy2.0:0, Uno:D10
// # GND
// # VCC

#ifdef DO_LCD

  Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN, TFT_SCK_PIN, TFT_RST_PIN);
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
  
#endif

/* -------------- ROTARY ENCODER -------------- */

#define ROT_CLK 12
#define ROT_DT 8
#define ROT_SW -1

// for reference

/* -------------- CONFIG / APPLICATION -------------- */

#define NUM_INPUTS 16
#define HEIGHT 130
#define WIDTH 150
#define SPACE 30
#define NUM_CHANNELS 4

int SQRT = sqrt(NUM_INPUTS); // square root of number of inputs (ie. 4)
int SIZE = min(WIDTH,HEIGHT); // minimum size
int _SMOOTHING = 90;
int _LAST_ROT;
int _CURRENT_ROT;
int _CURRENT_DT;
int _LAST_CHANNEL = -1;
bool _HAS_CHANGED = true;
bool _INITED = false;

/* -------------- GLOBAL REFERENCES -------------- */


int _LOOKUP[NUM_INPUTS * NUM_CHANNELS];
int _TIMESTAMPS[NUM_INPUTS * NUM_CHANNELS];

int _ANGLES[NUM_INPUTS];
int _VALUES[NUM_INPUTS];

int _CHANNEL = 0;
int _CENTER_X[NUM_INPUTS];
int _CENTER_Y[NUM_INPUTS];
int _STATUS[NUM_INPUTS];
int _DIRECTION[NUM_INPUTS];
word _COLOURS[4];
bool _BUTTON = false;


word COL_WHITE = colour(255,255,255);
word COL_BLACK = colour(0,0,0);
word COL_RED = colour(255,0,0);

void pointFromAngle(int src_x, int src_y, int angle, int distance, int & dest_x, int & dest_y) {
    dest_x = cos(angle * PI / 180) * distance + src_x;
    dest_y = sin(angle * PI / 180) * distance + src_y;
};

/* -------------- LOOP -------------- */

void drawTab( int idx ) {
  int sz = SIZE / SQRT;
  int xx = 0;
  int yy = (idx * sz) + 2;
  int hh = (SIZE / SQRT) - 4;
  int ww = WIDTH - SIZE;
  String str = "P" + String(idx + 1);
  tft.drawRect( xx, yy, ww, hh, _COLOURS[idx] );
  drawText( str, xx + 5, yy + 5 );
}

void drawDot( int use_channel, word col ) {
  int sz = SIZE / SQRT;
  int yb = (use_channel * sz) + 6;
  int ww = (WIDTH - SIZE)/2;
  int hh = (SIZE / SQRT) - 4;
  tft.fillCircle( ww, yb + (hh/2), 2, col );
}


void drawKnob( int idx) {
    int iy = SQRT - ( (idx - iy) / SQRT ) - 1;
    int ix = (idx%SQRT) + 1;
    
    int sz = SIZE / SQRT;
    
    int xx = sz * ix;
    int yy = sz * iy;
    
    int cx = _CENTER_X[idx] = xx + (sz/2);
    int cy = _CENTER_Y[idx] = yy + (sz/2);
    
    tft.drawCircle(cx,cy, (sz - 4)/2, _COLOURS[_CHANNEL]);

    int tri_a_x;
    int tri_a_y;
    int tri_b_x;
    int tri_b_y;

    pointFromAngle( cx, cy, 180 - SPACE - 90, sz/2, tri_a_x, tri_a_y);
    pointFromAngle( cx, cy, 180 + SPACE - 90, sz/2, tri_b_x, tri_b_y); 
    
//    tft.fillTriangle(cx,cy, tri_a_x, tri_a_y + 1, tri_b_x, tri_b_y + 1, colour(0,0,0));

//    String str = String(idx);
//    drawText(str, cx - 5, cy - 3 );
}

int getAngle( int value ) {
  return map(value, 1024, 0, SPACE * 2, 360) + 90 - SPACE;  
}

void drawLine( int idx, int value, word col ) {
  int xx = 0;
  int yy = 0;
  int sz = SIZE / SQRT;
  int angle = getAngle(value);
  pointFromAngle( _CENTER_X[idx], _CENTER_Y[idx], angle, (sz - 8)/2, xx, yy);
  tft.drawLine( _CENTER_X[idx], _CENTER_Y[idx], xx, yy, col);
}

bool isClose( int value, int ref ) {
  return (value >= ref - 4 && value <= ref + 4  );
}


/* -------------- SETUP -------------- */

void setup() {
  Serial.begin(9600);

  memset( _LOOKUP, 0, sizeof(_LOOKUP) );
  memset( _VALUES, 0, sizeof(_VALUES) );
  memset( _CENTER_X, 0, sizeof(_CENTER_X) );
  memset( _CENTER_Y, 0, sizeof(_CENTER_Y) );
  memset( _ANGLES, 0, sizeof(_ANGLES) );

  for (int i = 0; i <  sizeof(_LOOKUP); i++) _LOOKUP[i] = 512;
  for (int i = 0; i <  sizeof(_VALUES); i++) _VALUES[i] = 512;
  for (int i = 0; i <  sizeof(_ANGLES); i++) _ANGLES[i] = -999;
  

  // set mux lookup

    
  _COLOURS[0] = colour( 0,255,255 );
  _COLOURS[1] = colour( 255,255,0 );
  _COLOURS[2] = colour( 0,255,0 );
  _COLOURS[3] = colour( 0,0,255 );
  
  #ifdef DO_LCD
    tft.initR(INITR_BLACKTAB);
    tft.fillScreen(ST7735_BLACK);
    tft.setRotation(3);
    for (int i = 0; i < NUM_CHANNELS; i++) drawTab(i);
    drawDot(0, COL_WHITE);
    for (int i = 0; i < NUM_INPUTS; i++) drawKnob(i);
  #endif
  
  #ifdef DO_MUX
    mux.signalPin(MUX_SIG_PIN, INPUT, ANALOG);
  #endif
  
  #ifdef DO_ROT

    pinMode(ROT_CLK,INPUT);
    pinMode(ROT_DT,INPUT);

    
//    pinMode(ROT_SW, INPUT_PULLUP);
    
    _LAST_ROT = digitalRead( ROT_CLK );
    
  #endif
  tft.drawRect(22, 2, 5, SIZE - 6, COL_WHITE);

}

unsigned long timestamp;
int proposed_channel = 0;
int last_proposed_channel = 0;


void drawSmoothing() {
//   tft.fillRect(
}


void loop() {

  #ifdef DO_ROT

  _CURRENT_ROT = digitalRead(ROT_CLK);
  _CURRENT_DT = digitalRead(ROT_DT);
  bool changed = false;

  if (_CURRENT_ROT != _LAST_ROT && _CURRENT_ROT == 1) {
    timestamp = millis();
    if (_CURRENT_DT != _CURRENT_ROT) {
        proposed_channel++;
        if (proposed_channel >= NUM_CHANNELS) proposed_channel = 0;
    } else {
        proposed_channel--;
        if (proposed_channel < 0) proposed_channel = NUM_CHANNELS - 1;
    }
    if (proposed_channel != last_proposed_channel) {
      drawDot(last_proposed_channel, COL_BLACK);
      drawDot(proposed_channel, COL_WHITE);
      last_proposed_channel = proposed_channel;
    }
    changed = true;
    delay(1);
  }
  
  _LAST_ROT = _CURRENT_ROT;

//  if (changed) return;

  if ((millis() - timestamp > 300 && _CHANNEL != proposed_channel) || !_INITED) {
    _CHANNEL = proposed_channel;
    for (int i = 0; i < NUM_INPUTS; i++) {
      int IDX = (_CHANNEL * NUM_INPUTS) + i;

        drawKnob(i);

//      drawLine(i, _VALUES[i], COL_BLACK);
//      drawLine(i, _LOOKUP[IDX], COL_BLACK);

      _DIRECTION[i] = _VALUES[i] < _LOOKUP[IDX];
      _STATUS[i] = false;
    }
    _HAS_CHANGED = true;
  }
  
//
//  return;

  
  #endif

  #ifdef DO_MUX

    int value;
  
    for (byte i = 0; i < NUM_INPUTS; i++) {


      int IDX = (_CHANNEL * NUM_INPUTS) + i;
      int angle = getAngle(value);
      value = mux.read(i);

#if DO_PRINT
      String _idx = String(i);
      String _value = String(value);

      Serial.print( _idx + ":" + _value + " - ");
#endif
      if (!_INITED) {
        _VALUES[IDX] = value;
      }

      bool bCloseLookup = isClose(value, _LOOKUP[IDX] );
      bool bCloseValues = isClose(value, _VALUES[i] );
      bool bFlipped = value >= _LOOKUP[IDX];
      
      if ( !_STATUS[i] ) {


        if (bCloseLookup || bFlipped) {
        
          _STATUS[i] = true;

          
          drawLine(i, _VALUES[i], COL_BLACK);
//          drawLine(i, _LOOKUP[IDX], COL_BLACK);
          drawLine(i, value, COL_WHITE);
//          tft.fillCircle( _CENTER_X[i], _CENTER_Y[i], 2, COL_WHITE );
          _VALUES[i] = value;
          
        } else if (value != _VALUES[i]) {
          
            drawLine(i, _VALUES[i], COL_BLACK);
            drawLine(i, _LOOKUP[IDX], COL_RED);
//            drawLine(i, value, COL_WHITE);
//            tft.fillCircle( _CENTER_X[i], _CENTER_Y[i], 2, COL_RED );
            _VALUES[i] = value;
          
        }
        
        
      } else {


          if (value != _VALUES[i] || angle != _ANGLES[i]) {
            drawLine(i, _VALUES[i], COL_BLACK);
            drawLine(i, value, COL_WHITE);
//            tft.fillCircle( _CENTER_X[i], _CENTER_Y[i], 2, COL_WHITE );
            _VALUES[i] = value;
            _ANGLES[i] = angle;
          }
          _LOOKUP[IDX] = value;
          
        
      } // else
      
      
    }

#if DO_PRINT
     Serial.println("");
     delay(10);
#endif
     
    if (_HAS_CHANGED) _HAS_CHANGED = false;
    if (!_INITED) _INITED = true;
//    Serial.println("  ");

  #endif

}
