#include <lvgl.h>
#include <Ticker.h>
#include <TFT_eSPI.h>


//Sensors
#include <Wire.h>

#include <SparkFunCCS811.h>     
#include <SparkFunBME280.h>     
#include <ClosedCube_HDC1080.h>


#define LVGL_TICK_PERIOD 60

#include <WiFi.h>

#include <SHT1x-ESP.h>


#include <PubSubClient.h>
// Specify data and clock connections and instantiate SHT1x object
#define dataPin  18
#define clockPin 5

// if 3.3v board is used
SHT1x sht1x(dataPin, clockPin, SHT1x::Voltage::DC_3_3v);



#define MQTT_SERVER "192.168.0.2"
#define MQTT_PORT 14419
#define MQTT_CLIENT_ID "ESP32-RoomSensor1"
#define MQTT_PREFIX "ESP32_RoomSensor1_"

const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASS";

WiFiClient espClient;
PubSubClient client(espClient);

Ticker tick; /* timer for interrupt handler */
TFT_eSPI tft = TFT_eSPI(); /* TFT instance */
static lv_disp_buf_t disp_buf;
static lv_color_t buf[LV_HOR_RES_MAX * 10];

lv_obj_t * slider_label;
lv_obj_t * co2_label;
lv_obj_t * pressure_label;
lv_obj_t * tvoc_label;
int screenWidth = 480;
int screenHeight = 320;

//charts
lv_obj_t * chrt_press;
lv_obj_t * chrt_co2;
lv_chart_series_t * ser_press_1;
lv_chart_series_t * ser_co2_1;

//sensors
//#define CCS811_ADDR 0x5B  //Default I2C Address
#define CCS811_ADDR 0x5A    //Alternate I2C Address

CCS811 myCCS811(CCS811_ADDR);
ClosedCube_HDC1080 myHDC1080;
BME280 myBME280;



#if USE_LV_LOG != 0
/* Serial debugging */
void my_print(lv_log_level_t level, const char * file, uint32_t line, const char * dsc)
{

  Serial.printf("%s@%d->%s\r\n", file, line, dsc);
  delay(100);
}
#endif



void reconnect() {
  int i;
  String s;
  
   // server.handleClient();
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = MQTT_CLIENT_ID;
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
   // if (client.connect(clientId.c_str(),"MqttUser","")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      
      for(i=0;i<500;i++)
      {
        //server.handleClient();
        delay(10);
      }
    }

  //}
  //digitalWrite(LED_BUILTIN, HIGH);
  //reconnect_lastrun = millis();
}



/* Display flushing */
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
  uint16_t c;

  tft.startWrite(); /* Start new TFT transaction */
  tft.setAddrWindow(area->x1, area->y1, (area->x2 - area->x1 + 1), (area->y2 - area->y1 + 1)); /* set the working window */
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      c = color_p->full;
      tft.writeColor(c, 1);
      color_p++;
    }
  }
  tft.endWrite(); /* terminate TFT transaction */
  lv_disp_flush_ready(disp); /* tell lvgl that flushing is done */
}

bool my_touchpad_read(lv_indev_drv_t * indev_driver, lv_indev_data_t * data)
{
    uint16_t touchX, touchY;

    bool touched = tft.getTouch(&touchX, &touchY, 600);

    if(!touched)
    {
      return false;
    }

    if(touchX>screenWidth || touchY > screenHeight)
    {
      Serial.println("Y or y outside of expected parameters..");
      Serial.print("y:");
      Serial.print(touchX);
      Serial.print(" x:");
      Serial.print(touchY);
    }
    else
    {

      data->state = touched ? LV_INDEV_STATE_PR : LV_INDEV_STATE_REL; 
  
      /*Save the state and save the pressed coordinate*/
      //if(data->state == LV_INDEV_STATE_PR) touchpad_get_xy(&last_x, &last_y);
     
      /*Set the coordinates (if released use the last pressed coordinates)*/
      data->point.x = touchX;
      data->point.y = touchY;
  
      Serial.print("Data x");
      Serial.println(touchX);
      
      Serial.print("Data y");
      Serial.println(touchY);

    }

    return false; /*Return `false` because we are not buffering and no more data to read*/
}

/* Interrupt driven periodic handler */
static void lv_tick_handler(void)
{

  lv_tick_inc(LVGL_TICK_PERIOD);
}
