/* Version IoT_v4  BCChad Feb 23 2019   For Publication 
  Many thanks to Tobias for his baseline version.  I have included these newer features:
  1.  Option to select 12-hr format - set flag in tab "chad_time.h"
  2.  Clock will turn off and on at times "nowNight" and "nowDay" in tab "chad_time.h"
  3.  Display brightness is set in "void setup()" below
  4.  Display can be controlled from smartphone using IO.adafruit; see tab "adafruit.h"
Note 1 - Be sure to add your personal wifi password and AdaFruit Access in tab "adafruit.h"
Note 2 - some diagnostic "Serial.println" statements are commented out but available to get your smartphone connection working
*/
#include <PxMatrix.h>
#define PATTERN4  //for P10 16x32
#include <Ticker.h>
#include <PxMatrix.h>         // Download from: https://github.com/2dom/PxMatrix/, needs https://github.com/adafruit/Adafruit-GFX-Library via library manager
#include <TimeLib.h>
#include <NtpClientLib.h>     // https://github.com/gmag11/NtpClient via library manager
#include <ESP8266WiFi.h>

// WiFi-Manager
#include <DNSServer.h>        // Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> // Local WebServer used to serve the configuration portal
#include <WiFiManager.h>      // https://github.com/tzapu/WiFiManager WiFi Configuration Magic

// Local includes
#include "definitions.h"
#include "colors.h"
#include "numbers.h"
#include "drawing.h"
#include "ntp_time.h"
#include "adafruit.h"
#include "chad_time.h"

void setup()
{
  Serial.begin(115200);
  display.begin(8);  // Was null initially values 4,8, 16,32  (default 8?)
  display.clearDisplay();
  display.setBrightness(8);   // try values 0-255

  // Draw intro while WiFi is connecting
  drawIntro();

  // Connect to WiFi using WiFiManager
  // wifiManager.resetSettings();       // Reset WiFiManager settings, uncomment if needed
  wifiManager.setTimeout(AP_TIMEOUT); // Timeout until config portal is turned off
  if (!wifiManager.autoConnect(WLAN_SSID, WLAN_PASS))
  {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    //reset and try again
    ESP.reset();
    delay(5000);
  }

  init_state = 1;

  display_ticker.attach(0.001, display_updater);
  yield();
  delay(2000);

  // Setup MQTT subscription for tetrisBrightness and OnOff feeds, see adafruit.h
  mqtt.subscribe(&tetrisBrightness);
  mqtt.subscribe(&tetrisOnOff);
}

void loop()
{
  // Initialization state 1 - WiFi is connected, now start NTP
  if (init_state == 1)
  {
    NTP.begin("pool.ntp.org", timeZone, true, minutesTimeZone);  // timeZone set in definitions.h
    NTP.setInterval(63);
    init_state = 2;
  }

  // Initialization state 2 - Clock is now set, init number updater
  if (init_state == 2)
  {
    timer_ticker.attach(0.1, number_updater);
    // Log the current time
    Serial.print(NTP.getTimeDateString());
    Serial.print(" ");
    Serial.print(NTP.isSummerTime() ? "Summer Time. " : "Winter Time. ");
    Serial.print("WiFi is ");
    Serial.print(WiFi.isConnected() ? "connected" : "not connected");
    Serial.print(". ");
    Serial.print("Uptime: ");
    Serial.print(NTP.getUptimeString());
    Serial.print(" since ");
    Serial.println(NTP.getTimeDateString(NTP.getFirstSync()).c_str());
    init_state = 3;
  }

  // Initialization state 3 - Everything ready, check for time change
  if (init_state == 3)
  {
    String str_current_time = getTimeAsString();  // call to lib Ntp_time
    if (twelvehr) {  // "fix" current time from 24 to 12 format; global flag set above
      str_current_time = fixTime(str_current_time);  // declared new function below
    }

    MQTT_connect();   
    Adafruit_MQTT_Subscribe *subscription;
    while ((subscription = mqtt.readSubscription(200))) {
      if (subscription == &tetrisBrightness) {
  //      Serial.print(F("Got tetris Brightness : "));
        uint8_t ledBrightValue = atoi((char *)tetrisBrightness.lastread);
  //      Serial.println(ledBrightValue);
        display.setBrightness(ledBrightValue);
      }
      //    add this for OnOff
      if (subscription == &tetrisOnOff) {
  //      Serial.print(F("Got OnOff command : "));
        int onOff = atoi((char *)tetrisOnOff.lastread);
 //       Serial.println(onOff);
        if (onOff == 1) turnON();
        if (onOff == 0) turnOFF();
        //If 0 turn off, If 1 turn on
      }
    }
    // Time has changed
    if ((str_display_time != str_current_time) && !nighttime)
    {
  //    Serial.print("Time changed: ");
  //    Serial.println(str_current_time);
      updateTime(str_current_time);
      str_display_time = str_current_time;
    }
    nightCheck();    // create routine for checking night threshold
  }
} // end LOOP
