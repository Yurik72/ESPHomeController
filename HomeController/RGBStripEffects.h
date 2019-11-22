#ifndef RGBStripEffets_h
#define RGBStripEffets_h
#include "Ticker.h"
class StripWrapper;

#define SEGMENTS 1
typedef  uint32_t CRGB;

class RGBStripEffect {
public:
	RGBStripEffect(StripWrapper* p, uint8_t matrixwidth, uint16_t len,double interval);
	virtual ~RGBStripEffect();
	virtual void run();
	void oncallback();
	virtual void reset();
	static void callback(RGBStripEffect* self);
	virtual void runcycle() = 0;
	void stop();
	void drawPixelXY(int8_t x, int8_t y, CRGB color);
	void drawPixel(uint32_t pix, CRGB color);
	uint32_t getPixColor(int thisSegm);
	uint32_t getPixColorXY(int8_t x, int8_t y);
	uint16_t getPixelNumber(int8_t x, int8_t y);
	void set_delay(float val) { delay_interval = val; };
protected:
	Ticker cycleTicker;
	float delay_interval ;
	StripWrapper* pStrip;
	uint8_t mwidth;
	uint8_t mheight;
	uint16_t mlen;
	int rgb_start_led;
	unsigned char matrixValue[8][16];

};
#define SPARKLES 1        // вылетающие угольки вкл выкл


//these values are substracetd from the generated values to give a shape to the animation
const unsigned char valueMask[8][16] PROGMEM = {
  {32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 , 32 , 0  , 0  , 0  , 0  , 0  , 0  , 32 },
  {64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 , 64 , 0  , 0  , 0  , 0  , 0  , 0  , 64 },
  {96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 , 96 , 32 , 0  , 0  , 0  , 0  , 32 , 96 },
  {128, 64 , 32 , 0  , 0  , 32 , 64 , 128, 128, 64 , 32 , 0  , 0  , 32 , 64 , 128},
  {160, 96 , 64 , 32 , 32 , 64 , 96 , 160, 160, 96 , 64 , 32 , 32 , 64 , 96 , 160},
  {192, 128, 96 , 64 , 64 , 96 , 128, 192, 192, 128, 96 , 64 , 64 , 96 , 128, 192},
  {255, 160, 128, 96 , 96 , 128, 160, 255, 255, 160, 128, 96 , 96 , 128, 160, 255},
  {255, 192, 160, 128, 128, 160, 192, 255, 255, 192, 160, 128, 128, 160, 192, 255}
};

//these are the hues for the fire,
//should be between 0 (red) to about 25 (yellow)
const unsigned char hueMask[8][16] PROGMEM = {
  {1 , 11, 19, 25, 25, 22, 11, 1 , 1 , 11, 19, 25, 25, 22, 11, 1 },
  {1 , 8 , 13, 19, 25, 19, 8 , 1 , 1 , 8 , 13, 19, 25, 19, 8 , 1 },
  {1 , 8 , 13, 16, 19, 16, 8 , 1 , 1 , 8 , 13, 16, 19, 16, 8 , 1 },
  {1 , 5 , 11, 13, 13, 13, 5 , 1 , 1 , 5 , 11, 13, 13, 13, 5 , 1 },
  {1 , 5 , 11, 11, 11, 11, 5 , 1 , 1 , 5 , 11, 11, 11, 11, 5 , 1 },
  {0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 , 0 , 1 , 5 , 8 , 8 , 5 , 1 , 0 },
  {0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 , 0 , 0 , 1 , 5 , 5 , 1 , 0 , 0 },
  {0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 1 , 0 , 0 , 0 }
};
#define MODE_AMOUNT 3
struct {
	byte brightness = 50;
	byte speed = 30;
	byte scale = 40;
} modes[MODE_AMOUNT];

class RGBStripFireEffect :public RGBStripEffect
{
public :
	RGBStripFireEffect(StripWrapper* p, uint8_t matrixwidth, uint16_t len,double interval);
	virtual ~RGBStripFireEffect();
	virtual void runcycle();

protected :
	void generateLine();
	void shiftUp();
	void drawFrame(int pcnt);
	unsigned char* pline;
	int majorcycle ;
};
class RGBStripSnowEffect :public RGBStripEffect
{
public:
	RGBStripSnowEffect(StripWrapper* p, uint8_t matrixwidth, uint16_t len, double interval);
	virtual ~RGBStripSnowEffect();
	virtual void runcycle();
};
#endif