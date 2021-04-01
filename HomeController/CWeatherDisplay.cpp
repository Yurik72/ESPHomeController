
#include "config.h"
#include "BaseController.h"
#include "CWeatherClient.h"
#include "HTTPSimpleClient.h"
#include "CWeatherDisplay.h"

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const unsigned char *)(addr))
#endif

#include "weathericons.h"

#define CUT_SEVENSEGMENTSFONT_LASTC   //to cut font

#include "fonts.h"
#include "time.h"
#include "Wire.h"

#ifdef ESP32

#include "driver/gpio.h"

#endif

const uint16_t yellowpalette[]={0x0C23D4,0xFFE0,0x001F};
const uint16_t blackpalette[] = { 0x000000,0x000000,0x000000 };
const size_t bufferSize = JSON_OBJECT_SIZE(200);
#ifndef DISABLE_WEATHERDISPLAY
REGISTER_CONTROLLER_FACTORY(WeatherDisplayController)
#endif

WeatherDisplayController::WeatherDisplayController(){
  pDisplay7735=NULL;
  pDisplay9341=NULL;
  pDisplay=NULL;
  disptype=ILI9341;
  orientation=Hor;
  icon_palette = (uint16_t*)yellowpalette;
 // this->coreMode = Core;
 // this->core = 0;
  //recalclayout();
}
String  WeatherDisplayController::serializestate() {

	DynamicJsonDocument jsonBuffer(bufferSize);
	JsonObject root = jsonBuffer.to<JsonObject>();
	root[FPSTR(szisOnText)] = this->get_state().isOn;

	root[FPSTR(szbrightnessText)] = this->get_state().brigthness;
	root["time"] = this->get_state().now;
	String json;

	serializeJson(root, json);

	return json;
}
bool  WeatherDisplayController::deserializestate(String jsonstate, CmdSource src) {

	DynamicJsonDocument jsonBuffer(bufferSize);
	DeserializationError error = deserializeJson(jsonBuffer, jsonstate);
	if (error) {
		DBG_OUTPUT_PORT.print(FPSTR(szParseJsonFailText));
		DBG_OUTPUT_PORT.println(this->get_name());
		DBG_OUTPUT_PORT.println(error.c_str());
		return false;
	}
	JsonObject root = jsonBuffer.as<JsonObject>();
	WeatherDisplayState newState;
	newState.isOn = root[FPSTR(szisOnText)];//get_json_bool(root, FPSTR(szisOnText));//
	newState.brigthness= root[FPSTR(szbrightnessText)];
	


	this->AddCommand(newState, WDRefreshAll, src);
	//this->set_state(newState);
	return true;

}
void WeatherDisplayController::loadconfig(JsonObject& json) {
	WeatherDisplay::loadconfig(json);
	loadif(pcs, json, "pcs");
	loadif(prst, json, "prst");
	loadif(pdc, json, "pdc");
	loadif(psclk, json, "psclk");
	loadif(pmosi, json, "pmosi");
	loadif(pmiso, json, "pmiso");
	loadif(pbr, json, "pbr");
	loadif(dac_i2c, json, "dac_i2c");
	int tp = disptype;
	loadif(tp, json, "disptype");
	disptype = (DisplayType)tp;

}
void WeatherDisplayController::getdefaultconfig(JsonObject& json) {

	json[FPSTR(szservice)] = "WeatherDisplayClientController";
	json[FPSTR(szname)] = "WeatherDisplay";
	json["pcs"] = pcs;
	json["prst"] = prst;
	json["pdc"] = pdc;
	json["psclk"] = psclk;
	json["pmosi"] = pmosi;
	json["pmiso"] = pmiso;
	json["pbr"] = pbr;
	json["dac_i2c"] = dac_i2c;

	WeatherDisplay::getdefaultconfig(json);
}
void WeatherDisplayController::on_event(CBaseController* pSender, ControllerEvent evt, uint16_t evData) {
#ifdef WEATHER_GXEPD2
	if (disptype == GxEPD2 && pDisplayEPD2 && evt== SleepStart) {
		DBG_OUTPUT_PORT.println("E ink start hibernate");
		//pDisplayEPD2->hibernate();
	}
#endif
}

void WeatherDisplayController::run() {

	if (this->commands.GetSize() == 0) {
		command newcmd;
		//newcmd.mode = WDRefreshAll;
//		this->read_data();

		//this->commands.Add(newcmd);
		//this->AddCommand(newcmd.state, newcmd.mode, srcSelf);
	}
	command cmd;
	while (commands.Dequeue(&cmd)) {
		bool btime = false;
		bool  bweather = false;
		bool bforecast = false;
		bool bInfo = false;
		bool bAllClear = false;
		bool bsetBrighthess = false;
		if (this->baseprocesscommands(cmd))
			continue;
		WeatherDisplayState newState = get_state();
		
		switch (cmd.mode) {
			case WDSwitchMode:

				dispMode= (DisplayMode)((int)dispMode+1);
				if (dispMode == MaxMode)
					dispMode = MainWether;
				bAllClear = true;
				bInfo = dispMode == Info;
				btime = bforecast = bweather = (dispMode == MainWether);
				break;
			case WDRefreshAll:
				btime = bforecast = bweather = (dispMode == MainWether);
				if (pbr > 0)
					bsetBrighthess = true;
				newState = cmd.state;
				
				break;
			case WDFreeze:
				newState.isOn = false;
				break;
			case WDUnFreeze:
				btime = bforecast = bweather = dispMode == MainWether;
				newState.isOn = true;
				break;
			case WDSetTime:
				if (
					(commands.GetSize() > 0 && commands[0].mode == WDSetTime) ||   //skip to increase perfomance
					(commands.GetSize() > 1  && commands[1].mode == WDSetTime)
					) 
					continue;
				
				newState.now = cmd.state.now;
				btime = dispMode == MainWether;
				break;
			case WDSetCurrentData:
				newState.data = cmd.state.data;
				
				bweather = dispMode == MainWether;
				bInfo=bAllClear= dispMode == Info;
				
				break;
			case WDClearForecastData:
				getforecastdata().RemoveAll();

				break;
			case WDAddForecastData:
				
				getforecastdata().Add(cmd.state.frecord);
				bforecast = dispMode == MainWether;
				break;
			case WDSetBrigthness:
				if (pbr > 0 || dac_i2c>0) {
					bsetBrighthess = true;
				}
				break;
			default:
				break;
		}
		this->set_state(newState);
		
		if (get_state().isOn) {
			if (bAllClear)
				cleardisplay();
			if (bInfo)
				draw_info();
			if (btime && bforecast && bweather) {
				refreshAll();
				
			}
			else  {
				if (btime)
					refreshTime();
				if (bweather)
					draw_weatherdata();
				if (bforecast) {
					clear_forecast();
					draw_forecast();
					
				}
			}
		}
		if(bsetBrighthess)
			this->setBrightness(this->get_state().brigthness);
	}
	WeatherDisplay::run();
}
void WeatherDisplayController::set_state(WeatherDisplayState state) {


	WeatherDisplay::set_state(state);

	//	digitalWrite(pin, (state.isOn ^ this->isinvert) ? HIGH : LOW);

}

void WeatherDisplayController::refreshAll() {
	
	draw_forecast();
	draw_weatherdata();
	draw_time();
}

void WeatherDisplayController::setup(){
  if(disptype==ST7735){
    pDisplay7735 =new  Adafruit_ST7735(pcs, pdc, pmosi, psclk, prst);
	
    pDisplay=static_cast<Adafruit_GFX*>(pDisplay7735);
    pDisplay7735->initR(INITR_BLACKTAB);
  }else if (disptype==ILI9341){
     //Adafruit_ILI9341(int8_t _CS, int8_t _DC, int8_t _MOSI, int8_t _SCLK,  int8_t _RST = -1, int8_t _MISO = -1);
     pDisplay9341 =new  Adafruit_ILI9341(pcs, pdc,pmosi, psclk, prst,pmiso);
      pDisplay=static_cast<Adafruit_GFX*>(pDisplay9341);
      pDisplay9341->begin();
#ifdef ESP32
	  if (gpio_hold_en((gpio_num_t) prst) != ESP_OK) {
		  DBG_OUTPUT_PORT.println("rtc_gpio_hold_en error");
	  }
#endif
	  if (pbr > 0) {
#ifdef ESP8266
		  pinMode(pbr, OUTPUT);
		  digitalWrite(pbr,  LOW);
#endif
#ifdef ESP32
		  ledcSetup(br_channel, BR_FREQ, BR_RESOLUTION);
		  ledcAttachPin(pbr, br_channel);
		  pinMode(pbr, OUTPUT);
		  ledcWrite(br_channel, BRCALC_VAL(0, false));
#endif
		  this->setBrightness(brigthness);
	  }
      //diagnostic();
  }
#ifdef WEATHER_GXEPD2
  else if (disptype == GxEPD2) {
	  pDisplayEPD2 = new EPDTYPE(EPD2TYPENAME(pcs, pdc, prst, 4));
	  pDisplay = static_cast<Adafruit_GFX*>(pDisplayEPD2);
	  dt_color = ST7735_BLACK;
	  bk_color = ST7735_WHITE;
	  forecast_day_color = ST7735_BLACK;
	  forecast_temp_color= ST7735_BLACK;
	  forecast_precip_color = ST7735_BLACK;
	  pDisplayEPD2->init(115200);
	  //pDisplayEPD2->clearScreen();
	  pDisplayEPD2->firstPage();
	  pDisplay->setTextWrap(false);
	  orientation = Hor;
	  tm_text_size = 2;
	  dt_text_size = 1;
	  x_offs_time_text = 150;
	  y_offs_dt_text = 7;
	  icon_palette =(uint16_t*) blackpalette;
	  //pDisplayEPD2->setRotation(1);
	 
  }
#endif

  this->adjustRotation();
  this->recalclayout();
  
  cleardisplay();
 // this->flushDisplay();
  WeatherDisplay::setup();
}
void WeatherDisplayController::clearRect(DispRect_t* pRect) {
	pDisplay->fillRect(pRect->x, pRect->y, pRect->w, pRect->h, bk_color);
}
void WeatherDisplayController::flushDisplay(bool partial) {
#ifdef WEATHER_GXEPD2
	//pDisplayEPD2->refresh();

	if (disptype == GxEPD2 && pDisplayEPD2) {
			pDisplayEPD2->display(partial);
	}
#endif
}
void WeatherDisplayController::setBrightness(uint8_t br) {
	if (pbr > 0) {
#ifdef ESP8266
		analogWrite(pbr, BRCALC_VAL(br, false));
#endif
#ifdef ESP32
		ledcWrite(br_channel, BRCALC_VAL(br, false));
#endif
	}
	if (dac_i2c > 0) {
		Wire.beginTransmission(dac_i2c); //start transmit
		Wire.write(0x40); // Switch  DAC
		Wire.write(br); // value
		Wire.endTransmission(); // end

	}

	brigthness = br;

}
void WeatherDisplayController::cleardisplay() {
	pDisplay->fillRect(0, 0, pDisplay->width(), pDisplay->height(), bk_color);
}
void WeatherDisplayController::draw_info() {
	draw_infoline(10, "Temperature:", String(this->get_state().data.temp) +"C");
	draw_infoline(30, "Humidity:", String(this->get_state().data.humidity) + "%");
	draw_infoline(50, "Pressure:", String(this->get_state().data.pressure) + "hPa");
	draw_infoline(70, "Heap Mem:", String(ESP.getFreeHeap() / 1024) + "kb");
	draw_infoline(90, "WiFi Strength:", String(WiFi.RSSI()) + "dB");
	draw_infoline(110, "IP Address", String(WiFi.localIP().toString()) );
}

void  WeatherDisplayController::draw_infoline(uint16_t y,String label, String text) {
	pDisplay->setTextSize(2);
	pDisplay->setTextColor(ST7735_YELLOW);
	pDisplay->setCursor(10,y);
	pDisplay->println(label);
	pDisplay->setCursor(170,y);
	pDisplay->println(text);
	
}
void WeatherDisplayController::diagnostic(){
  return;
  if (disptype==ILI9341){
      // read diagnostics (optional but can help debug problems)
      uint8_t x = pDisplay9341->readcommand8(ILI9341_RDMODE);
      Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
      x = pDisplay9341->readcommand8(ILI9341_RDMADCTL);
      Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
      x = pDisplay9341->readcommand8(ILI9341_RDPIXFMT);
      Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
      x = pDisplay9341->readcommand8(ILI9341_RDIMGFMT);
      Serial.print("Image Format: 0x"); Serial.println(x, HEX);
      x = pDisplay9341->readcommand8(ILI9341_RDSELFDIAG);
      Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  }
}
void WeatherDisplayController::recalclayout(){
  if(disptype==ST7735){
    if(orientation==Vert){
      y_offs_forecast=80;
      height_forecast=28;
      width_forecast=40;
      max_forecast=3;
      pDisplay->setRotation(0);
    }else{
       y_offs_forecast=50;
       height_forecast=28;
       width_forecast=52;
       max_forecast=3;
      
        pDisplay->setRotation(1);
    }
  }else if(disptype==ILI9341){
      if(orientation==Vert){
      y_offs_forecast=80;
      height_forecast=28;
      width_forecast=40;
      max_forecast=3;

      //pDisplay->setRotation(0);
    }else{
       y_offs_forecast=120;
       height_forecast=40;
       width_forecast=80;
       max_forecast=4;
      fontsize_forecast_temp_h=2;
      isdraw_forecastcondition=true;
      forecast_cond_offset=80;

     main_offset_y_wetaher=50;
	 start_y_weather = 25;
       //spDisplay->setRotation(1);
    }
  }
#ifdef WEATHER_GXEPD2
  else if (disptype == GxEPD2) {
	  y_offs_forecast = 60;
	  height_forecast = 58;
	  width_forecast = 60;
	  max_forecast = 4;
	  main_offset_y_wetaher = 30;
	  start_y_weather = 25;
	  rect_time = DispRect_t(150, 0, pDisplay->width()-150, 25);
	  rect_date = DispRect_t(170, 33, pDisplay->width() - 160, 12);
	  rect_temp = DispRect_t(0, 0, 150, 60);
	  rect_hum= DispRect_t(160, 45, 250, 15);;
	  rect_press= DispRect_t(190, 45, 250, 15);;
	  forecast_cond_offset = 60;
	
  }
#endif
  rect_forecast = DispRect_t(0, y_offs_forecast, pDisplay->width(), pDisplay->height()- y_offs_forecast);
}
void WeatherDisplayController::set_orientation(Orientation o){
  orientation=o;
  recalclayout();
 };
void  WeatherDisplayController::adjustRotation(){
   if(orientation==Vert){
      pDisplay->setRotation(0);
    }else{
	   if (disptype != GxEPD2)
		pDisplay->setRotation(3);
#ifdef WEATHER_GXEPD2
	   if (disptype == GxEPD2) {
		   pDisplay->setRotation(1);
		   pDisplayEPD2->mirror(false);
	   }
#endif

    }
}
void WeatherDisplayController::draw_icon(uint8_t row,uint8_t col,uint16_t x,uint16_t y){
   int16_t w=BMP_WIDTH;
   int16_t h=BMP_HEIGHT;
 
   pDisplay->startWrite();
   for(int16_t j=0; j<h; j++, y++) {
        for(int16_t i=0; i<w; i++ ) {
           // int16_t pixel=pgm_read_word(&icons[row*h*IMAGE_ROWLEN+j*IMAGE_ROWLEN + i+col*w]);
            //if(pixel!=0){
            //  pDisplay->writePixel(x+i, y,pixel);
           // }
        }
    }
    pDisplay->endWrite();
}
void WeatherDisplayController::draw_icon(uint16_t xstart,uint16_t ystart,String icon,const uint16_t palette[]){
   int16_t w=BMP_WIDTH;
   int16_t h=BMP_HEIGHT;
   int16_t offset=6;
    //Serial.println("draw_icon");
   // Serial.println(icon);
   const char* picon=getMiniMeteoconIconFromProgmem(icon);
    uint8_t version = pgm_read_byte(picon);
    uint8_t bmpBitDepth = pgm_read_byte(picon + 1);
    uint8_t pixelsPerByte = 8 / bmpBitDepth;
    
   w=(pgm_read_byte(picon + 2) << 8) | (pgm_read_byte(picon + 3));
   h=(pgm_read_byte(picon + 4) << 8) | (pgm_read_byte(picon + 5));
   int16_t  widthRoundedUp = (w + 7) & ~7;

  uint8_t data;
  uint8_t paletteIndex = 0;

   uint32_t pointer = offset;
  // bitdepth = 8, initialShift = 0
  // bitdepth = 4, initialShift = 4
  // bitdepth = 2, initialShift = 6
  // bitdepth = 1, initialShift = 7
  uint8_t shift = 8 - bmpBitDepth ;
    uint8_t bitCounter = 0;
   pDisplay->startWrite();
   uint16_t bitMask =(1 << bmpBitDepth) - 1;//0xFFFF
  // Serial.println("BitMask");
  // Serial.println(bitMask);
   // bitsPerPixel  2, pixPerByte: 4, 2  4 = 2^2

  for(int16_t y = 0; y < h; y++) {
    for(int16_t x = 0; x < w; x++ ) {

      if (bitCounter == pixelsPerByte  || bitCounter == 0) {
       
        data = pgm_read_byte(picon + pointer);
        pointer++;
        //shift = bitsPerPixel;
        bitCounter = 0;
      }
      shift = 8 - (bitCounter + 1) * bmpBitDepth ;
      paletteIndex = (data >> shift) & bitMask;
      if(paletteIndex!=0){
        pDisplay->writePixel(xstart+x, ystart+y,palette[paletteIndex-1]);
      }

      
      bitCounter++;
    }
    //pointer++;
    bitCounter = 0;

  }
    pDisplay->endWrite();
}

#define SS_WIDTH 32
#define SS_HEIGHT 50
#define SS_DATAOFFS 4

void WeatherDisplayController::writePixel (uint16_t x,uint16_t y, uint16_t color){
  pDisplay->writePixel(x, y, color);
}
void WeatherDisplayController::drawPixel (uint16_t x,uint16_t y, uint16_t color){
  pDisplay->drawPixel(x, y, color);
}
void WeatherDisplayController::drawSSString(uint16_t x, uint16_t y,String str,uint16_t color){
  char* buf=new char[str.length()+1];
  str.toCharArray(buf, str.length()+1); 
  int reduce=0;
  for(int16_t i=0; i<str.length(); i++ ) {
    int charreduce=0;
    if(buf[i]=='.' || buf[i]==':'){
      charreduce=14;
    }else if(buf[i]==' '){
      charreduce=8;
    }

    drawSSchar(x+i*SS_WIDTH-reduce-charreduce,y,buf[i],color);
    charreduce*=2;
    reduce+=charreduce;
    
  }
  
  delete buf;
}
void WeatherDisplayController::drawSSchar(uint16_t x, uint16_t y,uint8_t ch,uint16_t color){
  int16_t w=SS_WIDTH/8;
  int16_t h=SS_HEIGHT;
  uint8_t digit=ch-32;
#ifdef CUT_SEVENSEGMENTSFONT_LASTC
  if (digit > 'C')
	  digit = 0;
#endif
  uint8_t rowlen=w;

  uint8_t offs=SS_DATAOFFS;
  int pX      = 0;

   pDisplay->startWrite();
   for(int16_t j=0; j<h; j++, y++) {
  
        for(int16_t i=0; i<w; i++ ) {
          
             uint8_t pixel8=pgm_read_byte(&SevenSegmentFull[digit*h*rowlen+j*rowlen + i+offs]);
              pX = x + i * 8;
              if (pixel8 & 0x80) drawPixel(pX, y, color);
              if (pixel8 & 0x40) drawPixel(pX + 1, y, color);
              if (pixel8 & 0x20) drawPixel(pX + 2, y, color);
              if (pixel8 & 0x10) drawPixel(pX + 3, y, color);
              if (pixel8 & 0x08) drawPixel(pX + 4, y, color);
              if (pixel8 & 0x04) drawPixel(pX + 5, y, color);
              if (pixel8 & 0x02) drawPixel(pX + 6, y, color);
              if (pixel8 & 0x01) drawPixel(pX + 7, y, color);

             
            }
        }
    
    pDisplay->endWrite();
}
void WeatherDisplayController::clearTime() {

	clearRect(&rect_time);
}
void WeatherDisplayController::clearDate() {
	clearRect(&rect_date);
}
void WeatherDisplayController::refreshTime() {

	

	if (disptype == GxEPD2) {
		pDisplay->fillRect(140, 0, 60, 25,bk_color);
		clearTime();
		clearDate();
		disp_time = this->get_state().now;
		draw_time();
		draw_date(format_date(disp_time), dt_color);
	}
	else
	{
		draw_time(disp_time, bk_color);
		if (format_date(disp_time) != format_date(this->get_state().now)) {
			draw_date(format_date(disp_time), bk_color);
		}
		disp_time = this->get_state().now;
		draw_time();
		draw_date(format_date(disp_time), dt_color);
	}
	this->flushDisplay();

}
void WeatherDisplayController::draw_time() {
	
	draw_time(disp_time, dt_color);
}
void WeatherDisplayController::draw_time(time_t time, uint16_t color){
  if(disptype==ILI9341 || disptype== GxEPD2){
  struct tm * timeinfo = localtime (&time);

  //char time_str[50];
 //sprintf(time_str, "%02d:%02d:%02d\n",timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
  pDisplay->setTextColor(color);
  pDisplay->setTextSize(tm_text_size);

  if (disptype == GxEPD2)
	  pDisplay->setCursor(rect_time.x, rect_time.y+9);
  else
	 pDisplay->setCursor(x_offs_time_text, y_offs_dt_text);

  pDisplay->println(getFormattedTime(time));
  
  }
}
void WeatherDisplayController::draw_date(String date, uint16_t color) {
	pDisplay->setTextSize(dt_text_size);
	pDisplay->setTextColor(color);

	if(disptype== GxEPD2)
		pDisplay->setCursor(rect_date.x, rect_date.y);
	else
		pDisplay->setCursor(1, y_offs_dt_text);
	pDisplay->println(date);
}
String WeatherDisplayController::format_date(time_t time) {
	struct tm * timeinfo = localtime(&time);
	String date = MONTH_NAMES[timeinfo->tm_mon] + " " + String(timeinfo->tm_mday) + " " + String(1900 + timeinfo->tm_year);
	return date;
}
void WeatherDisplayController::draw_weatherdata(){
    uint8_t start_y=1;
	WeatherData wdata = this->get_state().data;
   
if(disptype==ST7735){    
    pDisplay->fillRect(0,start_y_weather,  pDisplay->width(), start_y_weather- y_offs_forecast, ST7735_BLACK);
   this->drawSSString(1,start_y,format_doublestr("%.1f ",wdata.temp),ST7735_YELLOW);

   
//    pDisplay->setFont(&DSEG7_Classic_Bold_20);
    pDisplay->setTextSize(2);
    pDisplay->setTextColor(ST7735_YELLOW);
    pDisplay->setCursor(100, start_y+50);
    pDisplay->println("C");
    pDisplay->setTextSize(1);
 if(orientation==Vert){

   start_y+=75;
    pDisplay->setCursor(5, start_y);
    pDisplay->setTextColor(ST7735_BLUE);
    pDisplay->print(format_doublestr("%.f",wdata.humidity));
    pDisplay->setCursor(80, start_y);
    pDisplay->print('%');
    pDisplay->setTextSize(1);
    pDisplay->setCursor(50, start_y);
    pDisplay->setTextColor(ST7735_RED);
    pDisplay->println(format_doublestr("%.f",wdata.pressure));
 }else{
    pDisplay->setFont(NULL);
    pDisplay->setCursor(pDisplay->width()-30, start_y+10);
    pDisplay->setTextColor(ST7735_BLUE);
    pDisplay->print(format_doublestr("%.f%%",wdata.humidity));
    pDisplay->setCursor(pDisplay->width()-30, start_y+30);
    pDisplay->setTextColor(ST7735_RED);
    pDisplay->println(format_doublestr("%.fhP",wdata.pressure));
 }
}else if(disptype==ILI9341){
   pDisplay->fillRect(0,start_y_weather, pDisplay->width(),  y_offs_forecast- start_y_weather, ST7735_BLACK);
   this->drawSSString(5,main_offset_y_wetaher,format_doublestr("%.1f C ",wdata.temp),ST7735_YELLOW);
      

	if(orientation==Vert){
	   pDisplay->setTextSize(1);

		pDisplay->setCursor(5, main_offset_y_wetaher);
		pDisplay->setTextColor(COLOR_LIGHTBLUE);
		pDisplay->print(format_doublestr("Hum %.f %%",wdata.humidity));

		pDisplay->setTextSize(1);
		pDisplay->setCursor(5, main_offset_y_wetaher+50);
		pDisplay->setTextColor(ST7735_RED);
		pDisplay->println(format_doublestr("Press %.f",wdata.pressure));
	 }else{
	   pDisplay->setTextSize(2);
	   if (wdata.gas_resistance > 0) {
		   pDisplay->setCursor(180, main_offset_y_wetaher);
		   pDisplay->setTextColor(COLOR_LIGHTBLUE);
		   pDisplay->print(format_doublestr("%.f %%", wdata.humidity));
		   pDisplay->setCursor(240, main_offset_y_wetaher);
		   pDisplay->setTextColor(ST7735_CYAN);
		   pDisplay->println(format_doublestr("%.f ", wdata.pressure));
		   pDisplay->setCursor(200, main_offset_y_wetaher + 30);
		   int level = CalculateIAQLevel(wdata.gas);
		   pDisplay->setTextColor(ColorForAirQuality(level));
		   pDisplay->setTextSize(3);
		   pDisplay->println(String((int)wdata.gas));
	   }
	   else {
		   pDisplay->setCursor(180, main_offset_y_wetaher);
		   pDisplay->setTextColor(COLOR_LIGHTBLUE);
		   pDisplay->print(format_doublestr("Hum %.f %%", wdata.humidity));


		   pDisplay->setCursor(180, main_offset_y_wetaher + 30);
		   pDisplay->setTextColor(ST7735_RED);
		   pDisplay->println(format_doublestr("P %.f hPa", wdata.pressure));
	   }
	 }
  
}
#ifdef WEATHER_GXEPD2
else if (disptype == GxEPD2) {
	this->clearRect(&rect_temp);
	pDisplay->setTextWrap(true);
	this->drawSSString(rect_temp.x, rect_temp.y+7, format_doublestr("%.1f C", wdata.temp), ST7735_BLACK);
	pDisplay->setTextWrap(false);
	pDisplay->setTextSize(1);



	//pDisplay->setCursor(180, main_offset_y_wetaher);
	this->clearRect(&rect_hum);
	pDisplay->setCursor(rect_hum.x, rect_hum.y);
	pDisplay->setTextColor(ST7735_BLACK);
	pDisplay->print(format_doublestr("%.f %%", wdata.humidity));


	this->clearRect(&rect_press);
	pDisplay->setCursor(rect_press.x, rect_press.y);
	pDisplay->setTextColor(ST7735_BLACK);
	pDisplay->println(format_doublestr("%.f hPa", wdata.pressure));
}
#endif
this->flushDisplay();
}
void WeatherDisplayController::draw_forecast(){
  
  clear_forecast();
 // Serial.println("Draw forecasts "+String(getforecastdata().GetSize()));
  for(int i=0;i<getforecastdata().GetSize() && i<max_forecast;i++){
      draw_forecast(i,getforecastdata()[i]);
  }
  this->flushDisplay();
}
int WeatherDisplayController::ColorForAirQuality(int level) {
	switch (level) {
	case 0:
		return ST7735_GREEN;
	case 1:
		return ST7735_GREEN;
	case 2:
		return ST7735_YELLOW;
	case 3:
		return ST7735_MAGENTA;
	case 4:
		return ST7735_CYAN;
	case 5:
		return ST7735_RED;

	default:
		return 0xFFFF;
	}
}
void WeatherDisplayController::clear_forecast(){
   pDisplay->fillRect(0, y_offs_forecast, pDisplay->width(), pDisplay->height()-y_offs_forecast, bk_color); 
}
void WeatherDisplayController::draw_forecast(uint8_t idx,ForeceastRecord& rec){
//Serial.println("Draw forecast "+String(idx));
  uint16_t start_y=y_offs_forecast;
  uint16_t start_x=idx*width_forecast;
  //pDisplay->drawFastVLine(start_x, start_y, pDisplay->height()-start_y, ST7735_YELLOW);
  uint16_t daytext_offs = 3;
  uint16_t icon_offs = 10;
  uint16_t temp_offs = 54;
  if (disptype == GxEPD2) {
	  daytext_offs = 1;
	  icon_offs = 5;
	  temp_offs = 50;
}
   pDisplay->setFont(NULL); 
   pDisplay->setCursor(start_x+5, start_y+ daytext_offs);
   pDisplay->setTextColor(forecast_day_color);
   pDisplay->setTextSize(1);
   pDisplay->println(rec.text);
    
  //draw_icon(ICONROW(data.condition),ICONCOL(data.condition),40,start_y+1);
   //Serial.println(rec.icon);

   draw_icon(start_x,start_y+ icon_offs,rec.icon, (const uint16_t*)icon_palette);
   start_y+= temp_offs;
   pDisplay->setCursor(start_x+2, start_y);
   pDisplay->setTextColor(forecast_temp_color);
   pDisplay->setTextSize(fontsize_forecast_temp_h);
   pDisplay->println(format_doublestr("%.0f C",rec.temperature));
   pDisplay->setTextColor(forecast_precip_color);
   pDisplay->setCursor(start_x+(fontsize_forecast_temp_h==1?30:35), start_y);
   pDisplay->println(String(rec.precipChance)+String("%"));
   
    start_y=y_offs_forecast+forecast_cond_offset;
    pDisplay->setTextSize(1);
    pDisplay->setTextColor(dt_color);
    pDisplay->setCursor(start_x+5, start_y);
    pDisplay->println(rec.phraseshort);
}
String WeatherDisplayController::format_doublestr(const char* fmt, double val) {

  String res;
  char buff[50];
  snprintf(buff, sizeof(buff), fmt, val);
  res = buff;
  return res;
}
uint16_t WeatherDisplayController::color8to16(uint8_t color)
{
  uint8_t  blue[] = {0, 11, 21, 31}; // blue 2 to 5 bit colour lookup table
  uint16_t color16 = 0;

  //        =====Green=====     ===============Red==============
  color16  = (color & 0x1C) << 6 | (color & 0xC0) << 5 | (color & 0xE0) << 8;
  //        =====Green=====    =======Blue======
  color16 |= (color & 0x1C) << 3 | blue[color & 0x03];

  return color16;
}
