
#include "RGBStripController.h"
#include "RGBStripEffects.h"
#include "config.h"
#include "Utilities.h"


RGBStripEffect::RGBStripEffect(StripWrapper* p, uint8_t matrixwidth, uint16_t len, double interval) {
	
	delay_interval = interval;
	mlen = len;
	mwidth = matrixwidth;
	if (mwidth == 0)
		mwidth = 8;
	mheight = len / mwidth;
	if (delay_interval == 0.0)
		delay_interval = 0.01;
	pStrip = p;
	rgb_start_led = p->get_rgb_startled();
	_matrixType = p->get_matrixtype();
	//DBG_OUTPUT_PORT.println("RGBStripEffect");
	//DBG_OUTPUT_PORT.println(_matrixType);
}
RGBStripEffect::~RGBStripEffect() {
	DBG_OUTPUT_PORT.println("RGBStripEffect deleted");
}
void RGBStripEffect::drawPixelXY(int8_t x, int8_t y, CRGB color) {
	if (x < 0 || x > mwidth - 1 || y < 0 || y > mheight - 1) return;
	int thisPixel = getPixelNumber(x, y) * SEGMENTS;
	for (byte i = 0; i < SEGMENTS; i++) {
		//if ((thisPixel + i) >= rgb_start_led)
		//if (rgb_start_led<0 || (thisPixel + i) <= (uint32_t)rgb_start_led)
		//	color = GRB_TO_RGB(color);
		
		pStrip->setPixelColor(thisPixel + i, color);
	}
}
void RGBStripEffect::drawPixel(uint32_t pix, CRGB color) {
	 
	//if (pix >= rgb_start_led)
	//	color = GRB_TO_RGB(color);
	//pStrip->setPixelColor(pix, GRB_TO_RGB(color));
	//if (rgb_start_led<0  ||  pix <= (uint32_t)rgb_start_led)
	//	color = GRB_TO_RGB(color);
	pStrip->setPixelColor(pix, color);
}
// функци€ получени€ цвета пиксел€ по его номеру
uint32_t RGBStripEffect::getPixColor(int thisSegm) {
	int thisPixel = thisSegm * SEGMENTS;
	if (thisPixel < 0 || thisPixel > mlen - 1) return 0;
	uint32_t color = pStrip->getPixelColor(thisPixel);
//	if (thisPixel >= rgb_start_led)
	//if (rgb_start_led<0  ||  thisPixel <= (uint32_t)rgb_start_led)
	//	color = GRB_TO_RGB(color);
	return color;// (((uint32_t)leds[thisPixel].r << 16) | ((long)leds[thisPixel].g << 8) | (long)leds[thisPixel].b);
}

// функци€ получени€ цвета пиксел€ в матрице по его координатам
uint32_t RGBStripEffect::getPixColorXY(int8_t x, int8_t y) {
	return getPixColor(getPixelNumber(x, y));
}
uint16_t RGBStripEffect::getPixelNumber(int8_t x, int8_t y) {
	int8_t thisx=x;
	int8_t thisy = y;
	int8_t thiswidth = mwidth;
	int8_t thisheight = mheight;
	if ((_matrixType & NEO_MATRIX_AXIS) == NEO_MATRIX_COLUMNS) {   // to do matrix type
		 
		thisx = y;
		thisy = x;
		thiswidth = mheight;
		thisheight = mwidth;


	}
	if ((_matrixType & NEO_MATRIX_SEQUENCE) == NEO_MATRIX_ZIGZAG) {
		if ((/*THIS_Y*/thisy % 2 == 0) /*|| MATRIX_TYPE*/) {               // если чЄтна€ строка
			return (/*THIS_Y*/thisy * /*_WIDTH*/thiswidth + /*THIS_X*/thisx);
		}
		else {                                              // если нечЄтна€ строка
			return (/*THIS_Y*/thisy * /*_WIDTH*/thiswidth + /*_WIDTH*/thiswidth - /*THIS_X*/thisx - 1);
		}
	}
	else {
		return (/*THIS_Y*/thisy * /*_WIDTH*/thiswidth + /*THIS_X*/thisx);
	}
}

void RGBStripEffect::run() {
	this->reset();

	this->oncallback();
}
void RGBStripEffect::stop() {
	cycleTicker.detach();
}
void RGBStripEffect::oncallback() {

	runcycle();
	cycleTicker.once<RGBStripEffect*>(delay_interval, RGBStripEffect::callback, this);
}

void RGBStripEffect::reset() {

}

void RGBStripEffect::callback(RGBStripEffect* self) {
	self->oncallback();
}

RGBStripFireEffect::RGBStripFireEffect(StripWrapper* p, uint8_t matrixwidth, uint16_t len,double interval)
	: RGBStripEffect(p, matrixwidth,len, interval) {
	pline = new unsigned char[mwidth];
	majorcycle = 0;
	DBG_OUTPUT_PORT.println("RGBStripFireEffect started");
}
RGBStripFireEffect::~RGBStripFireEffect() {

	delete pline;
	DBG_OUTPUT_PORT.println("RGBStripFireEffect deleted");
}
void RGBStripFireEffect::runcycle() {

if (majorcycle >= 100) {
	shiftUp();
	generateLine();
	majorcycle = 0;
}
drawFrame(majorcycle);
majorcycle += 30;
pStrip->trigger();
}
void RGBStripFireEffect::generateLine() {
	for (uint8_t x = 0; x < mwidth; x++) {
		pline[x] = random(64, 255);
	}
}
void RGBStripFireEffect::shiftUp() {
	uint16_t height = mheight;
	for (uint8_t y = height - 1; y > 0; y--) {
		for (uint8_t x = 0; x < mwidth; x++) {
			uint8_t newX = x;
			if (x > 15) newX = x - 15;
			if (y > 7) continue;
			matrixValue[y][newX] = matrixValue[y - 1][newX];
		}
	}

	for (uint8_t x = 0; x < mwidth; x++) {
		uint8_t newX = x;
		if (x > 15) newX = x - 15;
		matrixValue[0][newX] = pline[newX];
	}
}


void RGBStripFireEffect::drawFrame(int pcnt) {
	uint16_t height =mheight;
	int nextv;

	//each row interpolates with the one before it
	for (unsigned char y = height - 1; y > 0; y--) {
		for (unsigned char x = 0; x < mwidth; x++) {
			uint8_t newX = x;
			if (x > 15) newX = x - 15;
			if (y < 8) {
				nextv =
					(((100.0 - pcnt) * matrixValue[y][newX]
						+ pcnt * matrixValue[y - 1][newX]) / 100.0)
					- pgm_read_byte(&(valueMask[y][newX]));
				
				CRGB color = /*CHSV*//*HSVColor*/HSVColor_f_int_int(
					//modes[1].scale * 2.5 + pgm_read_byte(&(hueMask[y][newX])), // H
					pgm_read_byte(&(hueMask[y][newX]))*1.0, // yk hue
					255, // S
					(uint8_t)max(0, nextv) // V
				);

				//leds[getPixelNumber(x, y)] = color;
				drawPixel(getPixelNumber(x, y), color);
				
			}
			else if (y == 8 && SPARKLES) {
				if (random(0, 20) == 0 && getPixColorXY(x, y - 1) != 0) drawPixelXY(x, y, getPixColorXY(x, y - 1));
				else drawPixelXY(x, y, 0);
			}
			else if (SPARKLES) {

				// стара€ верси€ дл€ €ркости
				if (getPixColorXY(x, y - 1) > 0)
					drawPixelXY(x, y, getPixColorXY(x, y - 1));
				else drawPixelXY(x, y, 0);

			}
		}
	}

	//first row interpolates with the "next" line
	for (unsigned char x = 0; x < mwidth; x++) {
		uint8_t newX = x;
		if (x > 15) newX = x - 15;
		
		CRGB color = /*CHSV*//*HSVColor*/HSVColor_f_int_int(
			//modes[1].scale * 2.5 + pgm_read_byte(&(hueMask[0][newX])), // H
			pgm_read_byte(&(hueMask[0][newX]))*1.0, //YK H
			255,           // S
			(uint8_t)(((100.0 - pcnt) * matrixValue[0][newX] + pcnt * pline[newX]) / 100.0) // V
		);
		//leds[getPixelNumber(newX, 0)] = color;
		drawPixel(getPixelNumber(newX, 0), color);
		
	}
}

RGBStripSnowEffect::RGBStripSnowEffect(StripWrapper* p, uint8_t matrixwidth, uint16_t len, double interval)
	: RGBStripEffect(p, matrixwidth, len, interval) {
}
RGBStripSnowEffect::~RGBStripSnowEffect() {


}
void RGBStripSnowEffect::runcycle() {

	for (byte x = 0; x < mwidth; x++) {
		for (byte y = 0; y < mheight - 1; y++) {
			drawPixelXY(x, y, getPixColorXY(x, y + 1));
		}
	}

	for (byte x = 0; x < mwidth; x++) {
		// заполн€ем случайно верхнюю строку
		// а также не даЄм двум блокам по вертикали вместе быть
		if (getPixColorXY(x, mheight - 2) == 0 && (random(0, 40) == 0))
			drawPixelXY(x, mheight - 1, 0xE0FFFF - 0x101010 * random(0, 4));
		else
			drawPixelXY(x, mheight - 1, 0x000000);
	}

	pStrip->trigger();
}
