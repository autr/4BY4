#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#include "MUX74HC4067.h"

/* -------------- READ -------------- */

// http://adam-meyer.com/arduino/CD74HC4067

/* -------------- DEBUG -------------- */

#define DO_HACK // <-- ATTN: REMOVE THIS (knob I wired wrong)
#define DO_LCD
#define DO_MUX
#define DO_ROT

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



/* -------------- ROTARY ENCODER -------------- */

#define ROT_CLK 12
#define ROT_DT 8
#define ROT_SW -1

// for reference

/* -------------- CONFIG / APPLICATION -------------- */

#define NUM_INPUTS 16
#define NUM_CHANNELS 4
#define NUM_TOTAL 64
#define HEIGHT 130
#define WIDTH 150
#define SPACE 30

int SQRT = sqrt(NUM_INPUTS); // square root of number of inputs (ie. 4)
int SIZE = min(WIDTH,HEIGHT); // minimum size

int _lastClk;
int _currentClk;
int _currentDT;

int _inited = 0;
bool _changed = false;

/* -------------- GLOBAL REFERENCES -------------- */


int _lookup[NUM_TOTAL];

int _angles[NUM_INPUTS];
int _values[NUM_INPUTS];

int _centersX[NUM_INPUTS];
int _centersY[NUM_INPUTS];

int _statuses[NUM_INPUTS];
int _directions[NUM_INPUTS];

int _channel = 0;
word _colours[4];


/* -------------- INCLUDES -------------- */

#ifdef DO_LCD
	#include "drawing.h"
#endif


bool isClose( int value, int ref ) {
	return (value >= ref - 3 && value <= ref + 3  );
}


/* -------------- SETUP -------------- */

void setup() {
	Serial.begin(9600);

		

	for (int i = 0; i <  NUM_INPUTS * NUM_CHANNELS; i++) {
		_lookup[i] = 512;
	}

	for (int i = 0; i <  NUM_INPUTS; i++) {
		_values[i] = 512;
		_angles[i] = -999;
	}
	

	// set mux lookup

	
	#ifdef DO_LCD
		
		_colours[0] = colour( 0,255,255 );
		_colours[1] = colour( 255,255,0 );
		_colours[2] = colour( 0,255,0 );
		_colours[3] = colour( 0,0,255 );
	
		tft.initR(INITR_BLACKTAB);
		tft.fillScreen(ST7735_BLACK);
		tft.setRotation(3);
		for (int i = 0; i < NUM_CHANNELS; i++) drawTab(i);
		drawDot(0, COL_WHITE);
	#endif
	
	#ifdef DO_MUX
		mux.signalPin(MUX_SIG_PIN, INPUT, ANALOG);
	#endif
	
	#ifdef DO_ROT

		pinMode(ROT_CLK,INPUT);
		pinMode(ROT_DT,INPUT);

		// attachInterrupt(0,onEncoder,CHANGE);
		// attachInterrupt(1,onEncoder,CHANGE);
		
		_lastClk = digitalRead( ROT_CLK );
		
		// tft.drawRect(22, 2, 5, SIZE - 6, COL_WHITE);
	#endif

}

unsigned long timestamp;
int proposed_channel = 0;
int last_proposed_channel = 0;

void drawFromCenter( int i, int v, word col ) {
	return;
	int xx = 0;
	int yy = 0;
	int sz = SIZE / SQRT / 2;
	int angle = (360 - map(v, 0, 1024, SPACE, 360-SPACE)) + 90;
	pointFromAngle( _centersX[i], _centersY[i], angle, sz - 6, xx, yy);
	tft.fillCircle(xx,yy,1,col);
}

int getIdx( int idx ) {

	#ifdef DO_HACK

		// I wired these the wrong way around, lol

		if (idx == 7) return 8;
		if (idx == 8) return 7;

	#endif

	return idx;

}

void loop() {


	#ifdef DO_ROT

		_currentClk = digitalRead(ROT_CLK);
		_currentDT = digitalRead(ROT_DT);
		bool changed = false;

		if (_currentClk != _lastClk && _currentClk == 1) {
			timestamp = millis();
			if (_currentDT != _currentClk) {
				proposed_channel--;
			} else {
				proposed_channel++;
			}
			if (proposed_channel < 0) proposed_channel = NUM_CHANNELS - 1;
			if (proposed_channel >= NUM_CHANNELS) proposed_channel = 0;
			if (proposed_channel != last_proposed_channel) {
				for (int i = 0; i < NUM_CHANNELS; i++) drawDot(i, COL_BLACK);
				drawDot(proposed_channel, COL_WHITE);
				last_proposed_channel = proposed_channel;
			}
			Serial.print("info:propose:");
			Serial.println(proposed_channel);
			changed = true;
			delay(1);
		}
		
		_lastClk = _currentClk;
		if (changed) return;

		if ((millis() - timestamp > 600 && _channel != proposed_channel) || _inited == 1) {
			for (int i = 0; i < NUM_INPUTS; i++) {

				int idx = i;
				int IDX = (_channel * NUM_INPUTS) + idx;
				drawLine(idx, _values[idx], COL_BLACK);
				drawLine(idx, _lookup[IDX], COL_BLACK);
				drawFromCenter(idx, _values[idx], COL_BLACK);
				drawFromCenter(idx, _lookup[IDX], COL_BLACK);
			}
			_channel = proposed_channel;
			for (int i = 0; i < NUM_INPUTS; i++) {
				int idx = i;
				int IDX = (_channel * NUM_INPUTS) + idx;
				drawKnob(idx);
				_directions[idx] = _values[idx] < _lookup[IDX];
				_statuses[idx] = false;
			}
			Serial.print("info:page:");
			Serial.println(_channel);
			if (_inited != 2) _inited = 2;
			_changed = true;
			return;
		}
		

	
	#endif
	#ifdef DO_MUX

		int value;
		int lookupVal;
		int pastVal;
		bool status;

	
		for (byte i = 0; i < NUM_INPUTS; i++) {


			int idx = i;

			int IDX = (_channel * NUM_INPUTS) + idx;
			int angle = getAngle(value);
			value = mux.read(getIdx(i));

			String _i = String(idx);
			String _IDX = String(IDX);

			lookupVal = _lookup[IDX];
			pastVal = _values[idx];
			status = _statuses[idx];


			if (_inited == 0) {
				_values[idx] = value;
				_angles[idx] = -999;
				_lookup[IDX] = 512;
				Serial.print("info:inited:");
				Serial.println(_i);
			} else {


				bool bCloseLookup = isClose(value, lookupVal );
				bool bCloseValues = isClose(value, pastVal );
				bool bFlipped = _directions[idx] ? value >= lookupVal : value < lookupVal; 
				
				// Serial.println(_i + ":" + _value + ":" + _lookup );

				if ( !status ) {


					// Serial.println(bCloseLookup);
					// Serial.println(bFlipped);
					// Serial.println(value);
					// Serial.println(_lookup[IDX]);
					// Serial.println("---");

					if (bCloseLookup || bFlipped) {
					
						/* RESET */

						_statuses[idx] = true;

						// Serial.print(_i + " A = ");
						// Serial.print(lookupVal);
						// Serial.println(pastVal);

						drawLine( idx, pastVal, COL_BLACK );
						drawFromCenter( idx, pastVal, COL_BLACK );
						drawLine(idx, lookupVal, COL_BLACK);
						drawFromCenter(idx, lookupVal, COL_BLACK);
						drawLine( idx, value, COL_WHITE );
						drawFromCenter( idx, value, COL_WHITE );
						drawKnob( idx );

						_values[idx] = value;
						// tft.fillCircle( _centersX[idx], _centersY[idx], 2, COL_WHITE );c
						
					} else {
						
						/* DISABLED */

						// Serial.print(_i + " B = ");
						// Serial.print(lookupVal);
						// Serial.println(pastVal);

						// drawLine(i, pastVal, COL_BLACK);
						// drawLine(i, lookupVal, COL_RED);
						// drawLine(i, value, COL_WHITE);

						if (!bCloseValues || _changed) {

							drawLine( idx, pastVal, COL_BLACK );
							drawFromCenter( idx, pastVal, COL_BLACK );
							drawLine(idx, lookupVal, COL_RED);
							drawFromCenter(idx, lookupVal, COL_RED);
							drawLine( idx, value, COL_WHITE );
							drawFromCenter( idx, value, COL_WHITE );
							drawKnob( idx, COL_RED );
							_values[idx] = value;
						}

						
					}
					
					
				} else {

					// drawLine(i, pastVal, COL_BLACK);
					// drawLine(i, value, COL_WHITE);

					if (!bCloseValues || _changed) {
						drawLine( idx, pastVal, COL_BLACK );
						drawFromCenter( idx, pastVal, COL_BLACK );
						drawLine( idx, value, COL_WHITE );
						drawFromCenter( idx, value, COL_WHITE );
						_values[idx] = value;
						_lookup[IDX] = value;
						Serial.print("change:");
						Serial.print(_IDX);
						Serial.print(":");
						Serial.println(_lookup[IDX]);

					}


						
					
				}
			}

			
		}


		 
//    Serial.println("  ");

	#endif
	if (!_inited) _inited = 1;
	_changed = false;

}
