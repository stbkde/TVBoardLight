#pragma once
//#define typeof(x) typeof(x)

#include <stdio.h>
#include <typeinfo>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>

#include <Ticker.h>
#include <AsyncMqttClient.h>

#include <NeoPixelBus.h>
#include <NeoPixelBrightnessBus.h> // instead of NeoPixelBus.h
#include <NeoPixelAnimator.h>

//#define FASTLED_ALLOW_INTERRUPTS 0
#define ON true
#define OFF false

// user settings
#include "credentials.h"
#include "colors.h"

//DeviceNameClear.replace('/', '_');

/* RgbColor ist outdated -> switching to RgbwColor */
RgbColor LastRgbColor = RgbColor(0);    // for restoring after switch on
RgbColor CurrentRgbColor = RgbColor(0); // for animations which using current color
RgbColor NextRgbColor = RgbColor(0);    // for setting after animation finished

RgbwColor LastRgbwColor = RgbwColor(0);
RgbwColor CurrentRgbwColor = RgbwColor(0);
RgbwColor NextRgbwColor = RgbwColor(0);

uint8_t LastBrightnessState = 150;
uint8_t CurrentBrightnessState = 150; // for restore after animations finished
uint8_t NextBrightnessState = 150; // same as above !!!!!;

const char coldiv = ',';

/***** include animations ****/

#include "animations.h"

/* fading */// animations
#include "color.h"
#include "brightness.h"

/* waiting */// animations
//#include "animation_fadelr.h"
#include "animation_pulsecolor.h"

bool DeviceState = OFF;
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;

String LastPublishedRgbColor = "";
uint8_t LastPublishedBrightness = 0;

String CurrentRgbColorString = "";

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");

  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  uint16_t subSwitch = mqttClient.subscribe(topicSwitch.c_str(), 2);
  Serial.print("Subscribing to switch at QoS 2, packetId: ");
  Serial.println(subSwitch);

  uint16_t subColor = mqttClient.subscribe(topicColor.c_str(), 2);
  Serial.print("Subscribing to color at QoS 2, packetId: ");
  Serial.println(subColor);

  uint16_t subWaitingForDevices = mqttClient.subscribe(topicWaitingForDevices.c_str(), 2);
  Serial.print("Subscribing to color at QoS 2, packetId: ");
  Serial.println(subWaitingForDevices);

  uint16_t subBrightness = mqttClient.subscribe(topicBrightness.c_str(), 2);
  Serial.print("Subscribing to brightness at QoS 2, packetId: ");
  Serial.println(subBrightness);

  uint16_t subCurrentRgbColor = mqttClient.subscribe(topicCurrentRgbColor.c_str(), 2);
  Serial.print("Subscribing to CurrentRgbColor at QoS 2, packetId: ");
  Serial.println(subCurrentRgbColor);

  uint16_t subCurrentBrightness = mqttClient.subscribe(topicCurrentBrightness.c_str(), 2);
  Serial.print("Subscribing to CurrentBrightness at QoS 2, packetId: ");
  Serial.println(subCurrentBrightness);

  /*mqttClient.publish("zuhause/wohnzimmer/licht/tvschrank/center/color", 0, true, "test 1");
    Serial.println("Publishing at QoS 0");
    uint16_t packetIdPub1 = mqttClient.publish("zuhause/wohnzimmer/licht/tvschrank/center/color", 1, true, "test 2");
    Serial.print("Publishing at QoS 1, packetId: ");
    Serial.println(packetIdPub1);
    uint16_t packetIdPub2 = mqttClient.publish("zuhause/wohnzimmer/licht/tvschrank/center/color", 2, true, "test 3");
    Serial.print("Publishing at QoS 2, packetId: ");
    Serial.println(packetIdPub2);*/
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  String ColorMode = "";  // RGB / RGBW
  uint8_t newR; uint8_t newG; uint8_t newB; uint8_t newW;
  uint16_t newH; uint16_t newS; uint16_t newL;

  Serial.print("MQTT Message received! Topic: ");
  Serial.println(topic);
  Serial.println("Color states before job:");
  logColorStates();

  /*** Topic: Switch 0/1 ***/
  if (String(topic) == topicSwitch)
  {
    Serial.println("Topic: Switch");
    int payint = strtol(payload, NULL, 0);

    if (payint == 1)
    {
      if (DeviceState == OFF)
      {
        DeviceState = ON;

        if (LastRgbColor == RgbColor(0)) { // after hardreset
          Serial.println("LastRgbColor ist black. Setting to colorMorning");
          LastRgbColor = RgbColorMorning;
          LastRgbwColor = RgbwColorMorning;
        }

        Serial.println("Received switch on signal...");

        if (WaitingAnimationRunning) {
          Serial.println("WaitingAnimationRunning == true");

          NextBrightnessState = LastBrightnessState;
          // RGB
          NextRgbColor = LastRgbColor; // initial color
          CurrentRgbColor = LastRgbColor;
          // RGBW
          NextRgbwColor = LastRgbwColor; // initial color
          CurrentRgbwColor = LastRgbwColor;
        }
        else {
          Serial.println("WaitingAnimationRunning == false");

          FadeToBrightness(DefaultBrightnessFadingTime, LastBrightnessState);
          FadeToRgbColor(DefaultFadingTime, LastRgbColor);
          //FadeToRgbwColor(DefaultFadingTime, LastRgbwColor);

          CurrentBrightnessState = LastBrightnessState;
          CurrentRgbColor = LastRgbColor;
          CurrentRgbwColor = LastRgbwColor;
        }
      }
      else {  // (DeviceState == ON)
        // do nothing ...
      }
    }
    else if (payint == 0)
    {
      Serial.println("Received switch off signal...");
      if (DeviceState == ON)
      {
        DeviceState = OFF;
        logColorStates();

        LastBrightnessState = CurrentBrightnessState;
        LastRgbColor = CurrentRgbColor;
        LastRgbwColor = CurrentRgbwColor;
        Serial.println("Switching OFF and setting LastRgbColor = CurrentRgbColor");

        logColorStates();

        if (WaitingAnimationRunning) {
          Serial.println("WaitingAnimationRunning == true");

          //RGB
          NextRgbColor = RgbColor(0); // setting after animation finished
          CurrentRgbColor = RgbColor(0);
          // RGBW
          NextRgbwColor = RgbwColor(0); // setting after animation finished
          CurrentRgbwColor = RgbwColor(0);

          CurrentBrightnessState = strip.GetBrightness();
        }
        else {
          Serial.println("WaitingAnimationRunning == false");
          animations_PixelCount.StopAll();

          // RGB
          FadeToRgbColor(DefaultFadingTime, RgbColor(0));
          NextRgbColor = RgbColor(0);
          CurrentRgbColor = RgbColor(0);
          // RGBW
          //FadeToRgbwColor(DefaultFadingTime, RgbwColor(0));
          NextRgbwColor = RgbwColor(0);
          CurrentRgbwColor = RgbwColor(0);

          CurrentBrightnessState = strip.GetBrightness();
        }
      }
      else {  // (DeviceState == OFF)
        // do nothing ...
      }
    }
    else {
      Serial.println("Unknown switch signal");
    }
  }

  /*
   * ** Topic: Color **
  */  
  
  if (String(topic) == topicColor)
  {
    Serial.println("Topic: Color");

    String value = String((char*)payload);
    ColorMode = value.substring( 0, value.indexOf(coldiv) );
    value = value.substring(value.indexOf(coldiv)+1);
  
    if (ColorMode == "rgb") 
    {
        if (WaitingAnimationRunning)
        {
          Serial.println("WaitingAnimationRunning == true");
          RgbColor ncol = RgbColor(value, coldiv);
          
          if (DeviceState == ON) {
            CurrentRgbColor = ncol;
            NextRgbColor = ncol;
            
            CurrentRgbwColor = RgbwColor(ncol.R, ncol.G, ncol.B, ncol.CalculateBrightness());
            NextRgbwColor = RgbwColor(ncol.R, ncol.G, ncol.B, ncol.CalculateBrightness());
          }
          else {
            LastRgbColor = ncol;
            LastRgbwColor = RgbwColor(ncol.R, ncol.G, ncol.B, ncol.CalculateBrightness());
          }
        }
        else {
          Serial.println("WaitingAnimationRunning == false");
          RgbColor ncol = RgbColor(value, coldiv);
    
          if (DeviceState == ON) {
            animations_PixelCount.StopAll();
            Serial.println(value);
            FadeToRgbColor(DefaultFadingTime, ncol);
          }
          else {
            LastRgbColor = ncol;
            LastRgbwColor = RgbColor(ncol.R, ncol.G, ncol.B);
          }
        }
        logColorStates();
    }
    else if (ColorMode == "rgbw") 
    {
        for (uint16_t pixel = 0; pixel < PixelCount; pixel++)
        {
            Serial.println("Setting RGBW Color...");
            RgbwColor rgbw(value, coldiv);
            strip.SetPixelColor(pixel, RgbColor(value, coldiv));
        }
    }
    else if (ColorMode == "hsb") 
    {
        FadeToRgbColor(DefaultFadingTime, HsbColor(value, coldiv));
    }

  }

  if (String(topic) == topicWaitingForDevices)
  {
    Serial.println("Topic: Waiting for devices");
    int payint = strtol(payload, NULL, 0);

    if (payint == 1)
    {
      Serial.println("Received switch on signal...");
      animations_PixelCount.StopAll();
      animations_2.StopAll();
      animator_wait_pulse.StopAll();

      FadeInFadeOutRinseRepeat();
      //Setup_FadeLR_Animation();
    }
    else if (payint == 0)
    {
      Serial.println("Received switch off signal...");

      animations_2.StopAll();
      animator_wait_pulse.StopAll();

      if (DeviceState == ON)
      {
        FadeToBrightness(DefaultBrightnessFadingTime, 150);
        FadeToRgbColor(DefaultFadingTime, CurrentRgbColor);
      }
      else {
        FadeToRgbColor(DefaultFadingTime, RgbColor(0));
      }

    }
    else {
      Serial.println("Unknown switch signal");
    }
  }

  if (String(topic) == topicBrightness)
  {
    Serial.print("Topic: Brightness=" + String(payload));

    int payint = strtol(payload, NULL, 0);

    if ((payint >= c_MinBrightness) && (payint <= c_MaxBrightness))
    {
      if (WaitingAnimationRunning)
      {
        Serial.println("WaitingAnimationRunning == true");

        if (DeviceState == ON) {
          LastBrightnessState = CurrentBrightnessState;
          NextBrightnessState = payint;
          CurrentBrightnessState = payint;

          FadeToBrightness(DefaultBrightnessFadingTime, payint);
        }
        else {
          LastBrightnessState = payint;
        }
      }
      else {
        Serial.println("WaitingAnimationRunning == false");

        if (DeviceState == ON) {
          LastBrightnessState = CurrentBrightnessState;
          CurrentBrightnessState = payint;
          NextBrightnessState = payint;

          FadeToBrightness(DefaultBrightnessFadingTime, payint);
        }
        else {
          LastBrightnessState = payint;
        }
      }
    }
  }

  // Allways shown
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  payload: ");
  Serial.println(payload);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);


  Serial.println("Color states after job:");
  logColorStates();
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

/******************************** Setup ******************************/

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println();

  while (!Serial); // wait for serial attach

  Serial.println();
  Serial.println("Initializing...");
  Serial.flush();

  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();
  SetRandomSeed();

  Serial.println();
  Serial.println("Running...");

  Serial.println("Setting output pins...");
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.println();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(HOSTNAME.c_str());

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)OTA_PASSWORD);
  // ArduinoOTA.setPassword(OTA_PASSWORD);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void logColorStates() {
  Serial.println("\n*********** Brightness ***********************");
  Serial.println("Last Brightness State: " + String(LastBrightnessState));
  Serial.println("Current Brightness State: " + String(CurrentBrightnessState));
  Serial.println("Next Brightness State: " + String(NextBrightnessState));
  Serial.println();
  Serial.println("\n*********** RGB Values ***********************");
  Serial.println("Last Color: R:" + String(LastRgbColor.R) + ", G:" + String(LastRgbColor.G) + ", B:" + String(LastRgbColor.B));
  Serial.println("Current Color: R:" + String(CurrentRgbColor.R) + ", G:" + String(CurrentRgbColor.G) + ", B:" + String(CurrentRgbColor.B));
  Serial.println("Next Color: R:" + String(NextRgbColor.R) + ", G:" + String(NextRgbColor.G) + ", B:" + String(NextRgbColor.B));
  Serial.println();
  Serial.println("\n*********** Rgbw Values **********************");
  Serial.println("Last Color: R:" + String(LastRgbwColor.R) + ", G:" + String(LastRgbwColor.G) + ", B:" + String(LastRgbwColor.B) + ", W:" + String(LastRgbwColor.W));
  Serial.println("Current Color: R:" + String(CurrentRgbwColor.R) + ", G:" + String(CurrentRgbwColor.G) + ", B:" + String(CurrentRgbwColor.B) + ", W:" + String(LastRgbwColor.W));
  Serial.println("Next Color: R:" + String(NextRgbwColor.R) + ", G:" + String(NextRgbwColor.G) + ", B:" + String(NextRgbwColor.B) + ", W:" + String(NextRgbwColor.W));
  Serial.println();
}


/************************************* loop ************************************/

String Rvalue = "";
String Gvalue = "";
String Bvalue = "";
String RGBdivider = ",";

unsigned long previousMillisAnim = 0;

void loop()
{
  ArduinoOTA.handle();
  //yield();  // Beware: only use if ArduinoOTA doesn't work!

  currentMillis = millis();

  if (currentMillis - previousMillis >= INTERVAL_CHECK_PUBLISH)
  {
    /*
     * Check Colors
     */
    CurrentRgbColorString = CurrentRgbColor.toString(coldiv);

    if (CurrentRgbColorString != LastPublishedRgbColor) {
      //logColorStates();
      Serial.println("topicCurrentRgbColor: Color Publish" + CurrentRgbColorString);
      Serial.println( RgbwColor("99,98,97,96", coldiv).toString(coldiv) );
      Serial.println( RgbColor("99,98,97", coldiv).toString(coldiv) );
      Serial.println( HsbColor("99.12,98.13,97.14", coldiv).toString(coldiv) );
      Serial.println( HslColor("99.12,98.13,97.14", coldiv).toString(coldiv) );
      Serial.println( "RGB->RGBW (1)| rgb:"+CurrentRgbColor.toString(coldiv) );
      Serial.println( "RGB->RGBW (2)| rgbw:"+RgbwColor(CurrentRgbColor).toString(coldiv) );

//      Serial.println( "typeof(CurrentRgbColor) = "+__typeof__(CurrentRgbColor) );
      uint16_t packetIdPub2 = mqttClient.publish(topicCurrentRgbColor.c_str(), 2, true, CurrentRgbColorString.c_str());
      LastPublishedRgbColor = CurrentRgbColorString;
    }

  //// Add publish of brightness and other colors
    
    // save last time the values are checked
    previousMillis = currentMillis;
  }

  if (animations_PixelCount.IsAnimating()) {
    animations_PixelCount.UpdateAnimations();
    strip.Show();
  }

  if (animations_2.IsAnimating()) {
    WaitingAnimationRunning = true;
    animations_2.UpdateAnimations();
    strip.Show();
  }
  else {
    WaitingAnimationRunning = false;
  }


  if (animator_wait_pulse.IsAnimating()) {
    if (DeviceState == OFF && strip.GetBrightness() < 100) {
      FadeToBrightness(DefaultBrightnessFadingTime, 150);
    }

    WaitingAnimationRunning = true;
    animator_wait_pulse.UpdateAnimations();
    strip.Show();
  }
  else {
    WaitingAnimationRunning = false;
  }
}
