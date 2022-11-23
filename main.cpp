
/*
    Simple Clock with NTP controll for WT32-SC01-Plus_ESP32-S3
    Requirements:
    - Development board : WT32-SC01-Plus_ESP32-S3
    - Arduino Library - Display/Touch : LovyanGFX
    - Board selected in Arduino : ESP32S3 Dev Module
*/

#define LGFX_USE_V1         // set to use new version of library
#include <LovyanGFX.hpp>    // main library
#include <NTPClient.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "time.h"

const char* ssid     = "REPLACE_WITH_YOUR_SSID";
const char* password = "REPLACE_WITH_YOUR_PASSWORD";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


class LGFX : public lgfx::LGFX_Device
{

  lgfx::Panel_ST7796  _panel_instance;  // ST7796UI
  lgfx::Bus_Parallel8 _bus_instance;    // MCU8080 8B
  lgfx::Light_PWM     _light_instance;
  lgfx::Touch_FT5x06  _touch_instance;

public:
  LGFX(void)
  {
    {
      auto cfg = _bus_instance.config();
      cfg.freq_write = 20000000;    
      cfg.pin_wr = 47;             
      cfg.pin_rd = -1;             
      cfg.pin_rs = 0;              

      // LCD data interface, 8bit MCU (8080)
      cfg.pin_d0 = 9;              
      cfg.pin_d1 = 46;             
      cfg.pin_d2 = 3;              
      cfg.pin_d3 = 8;              
      cfg.pin_d4 = 18;             
      cfg.pin_d5 = 17;             
      cfg.pin_d6 = 16;             
      cfg.pin_d7 = 15;             

      _bus_instance.config(cfg);   
      _panel_instance.setBus(&_bus_instance);      
    }

    { 
      auto cfg = _panel_instance.config();    

      cfg.pin_cs           =    -1;  
      cfg.pin_rst          =    4;  
      cfg.pin_busy         =    -1; 

      cfg.panel_width      =   320;
      cfg.panel_height     =   480;
      cfg.offset_x         =     0;
      cfg.offset_y         =     0;
      cfg.offset_rotation  =     0;
      cfg.dummy_read_pixel =     8;
      cfg.dummy_read_bits  =     1;
      cfg.readable         =  true;
      cfg.invert           = true;
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       =  true;

      _panel_instance.config(cfg);
    }

    {
      auto cfg = _light_instance.config();    

      cfg.pin_bl = 45;              
      cfg.invert = false;           
      cfg.freq   = 44100;           
      cfg.pwm_channel = 7;          

      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);  
    }

    { 
      auto cfg = _touch_instance.config();

      cfg.x_min      = 0;
      cfg.x_max      = 319;
      cfg.y_min      = 0;  
      cfg.y_max      = 479;
      cfg.pin_int    = 7;  
      cfg.bus_shared = true; 
      cfg.offset_rotation = 0;

      cfg.i2c_port = 1;
      cfg.i2c_addr = 0x38;
      cfg.pin_sda  = 6;   
      cfg.pin_scl  = 5;   
      cfg.freq = 400000;  

      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);  
    }
    setPanel(&_panel_instance); 
  }
};

static LGFX lcd;            // declare display variable

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }  
  lcd.setFont(&Font7);
  lcd.setTextSize(3,3);
  lcd.setCursor(0,20);  
  lcd.println(&timeinfo, "%H:%M");
  lcd.println(&timeinfo, "    %S");
  lcd.setFont(&FreeMono9pt7b);
  lcd.setTextSize(1,1);
  lcd.println(WiFi.localIP().toString());  
}

void setup(void)
{  
  lcd.init();  
  if (lcd.width() < lcd.height()) lcd.setRotation(lcd.getRotation() ^ 1);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  lcd.printf("Connecting to Wifi");
  delay(1000);
  }
  lcd.setFont(&Font7);
  lcd.setCursor(0,0);  
  lcd.println("WiFi connected.");
  lcd.clear();
  lcd.setTextColor(0xFFFFFFU, 0);
  lcd.println(WiFi.localIP().toString());
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  
}

void loop()
{
  printLocalTime();
}
