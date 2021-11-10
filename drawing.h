
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS_PIN, TFT_DC_PIN, TFT_MOSI_PIN, TFT_SCK_PIN, TFT_RST_PIN);

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
	tft.drawRect( xx, yy, ww, hh, _colours[idx] );
	drawText( str, xx + 5, yy + 5 );
}

void drawDot( int use_channel, word col ) {
	int sz = SIZE / SQRT;
	int yb = (use_channel * sz) + 6;
	int ww = (WIDTH - SIZE)/2;
	int hh = (SIZE / SQRT) - 4;
	tft.fillCircle( ww, yb + (hh/2), 2, col );
}


void drawKnob( int idx, word col ) {

	int iy = SQRT - ( (idx - iy) / SQRT ) - 1;
	int ix = (idx%SQRT) + 1;
	
	int sz = SIZE / SQRT;
	
	int xx = sz * ix;
	int yy = sz * iy;
	
	int cx = _centersX[idx] = xx + (sz/2);
	int cy = _centersY[idx] = yy + (sz/2);
	
	tft.drawCircle(cx,cy, (sz - 4)/2, col);

	int tri_a_x;
	int tri_a_y;
	int tri_b_x;
	int tri_b_y;

	pointFromAngle( cx, cy, 180 - SPACE - 90, sz/2, tri_a_x, tri_a_y);
	pointFromAngle( cx, cy, 180 + SPACE - 90, sz/2, tri_b_x, tri_b_y); 
	
}


void drawKnob( int idx) {
	drawKnob(idx, _colours[_channel]);
}

int getAngle( int value ) {
	return map(value, 1024, 0, SPACE * 2, 360) + 90 - SPACE;  
}

void drawLine( int idx, int value, word col ) {
	int xx = 0;
	int yy = 0;
	int sz = SIZE / SQRT;
	int angle = getAngle(value);
	pointFromAngle( _centersX[idx], _centersY[idx], angle, (sz - 8)/2, xx, yy);
	tft.drawLine( _centersX[idx], _centersY[idx], xx, yy, col);
}
