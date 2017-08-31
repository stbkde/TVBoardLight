/******************* Rooms ***********************************************
 *  
 *  Topic: "HomeName/RoomName/TopicName/DeviceName/..."
*/
const String  HomeName = "zuhause";
const String  RoomName = "wohnzimmer";
const String  TopicName = "licht";
const String  DeviceName = "tvschrank/center";
const String  DeviceNameClear = "tvschrank_center";


/**************** WiFi Access ********************************************/
#define WIFI_SSID "WhyLAN"
#define WIFI_PASSWORD "password"


/**************** ArduinoOTA *********************************************/
const String  HOSTNAME = "esp8266-"+RoomName+"-"+DeviceNameClear;
const char*   OTA_PASSWORD = "zuhause";


/**************** MQTT Access ********************************************/
#define  MQTT_HOST IPAddress(10, 0, 0, 5)
#define  MQTT_PORT 1883

#define  INTERVAL_CHECK_PUBLISH 500


/************* WS2812/ Neopixels *****************************************/
const uint8_t  PixelCount = 30; //30; (-1)
const uint8_t   c_MinBrightness = 20; 
const uint8_t   c_MaxBrightness = 255;


/**************** Colors *************************************************/
// default values if there are no other, by time
const RgbColor    RgbColorMorning(224,153,40);
const RgbColor    RgbColorEvening(28,12,2);

const RgbwColor   RgbwColorMorning(RgbColorMorning);
const RgbwColor   RgbwColorEvening(RgbColorEvening);


/********** Fading Animations ********************************************/
const uint16_t  DefaultFadingTime             = 1000; // ms
const uint16_t  DefaultBrightnessFadingTime   = 2000; // beware: lower values than 2000 ms could freeze the brightness-fading for next time (why?)


/************* Animations-Setup ******************************************/
const RgbColor  CylonEyeColor(221,229,15);


/**************** MQTT Topics ********************************************/

const String  topicSwitch             = HomeName+"/"+RoomName+"/"+TopicName+"/"+DeviceName+"/switch";
const String  topicColor              = HomeName+"/"+RoomName+"/"+TopicName+"/"+DeviceName+"/color";
const String  topicBrightness         = HomeName+"/"+RoomName+"/"+TopicName+"/"+DeviceName+"/brightness";
const String  topicAnimation          = HomeName+"/"+RoomName+"/"+TopicName+"/"+DeviceName+"/animation";

const String  topicWaitingForDevices  = HomeName+"/"+RoomName+"/waitingfordevices";

// next: only state feedback
const String  topicCurrentRgbColor    = HomeName+"/"+RoomName+"/"+TopicName+"/"+DeviceName+"/current_rgbcolor";
const String  topicCurrentBrightness  = HomeName+"/"+RoomName+"/"+TopicName+"/"+DeviceName+"/current_brightness";
