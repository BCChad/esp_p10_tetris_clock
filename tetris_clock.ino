// Version BCChad June 19, 2018 - includes two enhancements
//  1. see Line 25 - flag to change to 12-hour time format (function on line 135)
//  2. Line 114 "nightCheck" mode - turn display off during sleeping hours
//  All changes were made to this file, no other .h files affected but good practice would suggest
//  migrating the changes to ntp_time.h
#define PATTERN4
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

bool twelvehr = true;       //  Set to false for 24-hour clock
bool nighttime = true;

// ****************************************************************
// * Setup
// ****************************************************************
void setup()
{
  Serial.begin(115200);

  display.begin(8);  // Was null initially values 4,8, 16,32  (default 8?)
  display.clearDisplay();

  // Draw intro while WiFi is connecting
  drawIntro();

  // Connect to WiFi using WiFiManager
  // wifiManager.resetSettings();       // Reset WiFiManager settings, uncomment if needed
  wifiManager.setTimeout(AP_TIMEOUT); // Timeout until config portal is turned off
  if (!wifiManager.autoConnect(AP_NAME, AP_PASS))
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
  // display.clearDisplay();
}

void loop()
{
  // Initialization state 1
  // WiFi is connected, now start NTP
  if (init_state == 1)
  {
    NTP.begin("pool.ntp.org", timeZone, true, minutesTimeZone);  // timeZone set in definitions.h
    NTP.setInterval(63);
    init_state = 2;
  }

  // Initialization state 2
  // Clock is now set, init number updater
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

  // Initialization state 3
  // Everything ready, check for time change
  if (init_state == 3)
  {
    String str_current_time = getTimeAsString();  // call to lib Ntp_time
    if (twelvehr) {  // "fix" current time from 24 to 12 format; global flag set above
      str_current_time = fixTime(str_current_time);  // declared new function below
    }

    // Time has changed
    if ((str_display_time != str_current_time) && !nighttime)
    {
      Serial.print("Time changed: ");
      Serial.println(str_current_time);
      updateTime(str_current_time);
      str_display_time = str_current_time;
    }
    nightCheck();    // create routine for checking night threshold
    // delay(2000);  // BCC added for debug
  }
}

void nightCheck()  // Sets/clears night flag, turns display on/off
{
  int nowNight = 2200;   // integer value of 24-hour clock, spec is 2200
  int nowDay = 600;
  String str_checktime = getTimeAsString();
  // case 1 - daytime, becoming night
  if (!nighttime && (str_checktime.toInt() > nowNight))  {
    timer_ticker.detach();     //  Is there an easier "display.end"??
    display_ticker.detach();
    display.clearDisplay();
    nighttime = true;
  }
  // case 2 - nighttime - becoming day
  if (nighttime && (str_checktime.toInt() < nowNight) && (str_checktime.toInt() > nowDay))
  {
    display_ticker.attach(0.001, display_updater);
    timer_ticker.attach(0.1, number_updater);
    nighttime = false;
  }
}

String fixTime(String cur_Time) {      // Patch to change 24-hr to 12-hr, called if bool flag set
  // (function by BCChad  05/25/2018)

  String hours = cur_Time.substring(0, 2);  // get hours substr
  cur_Time.remove(0, 2);           // remove it
  int hoursint = hours.toInt();   // change hours substr to integer
  hoursint = hoursint % 12;
  if (hoursint == 0) hoursint = 12;  // 12-hr clock doesn't have "00"
  hours = String(hoursint);  // Convert integer back to string
  if (hoursint < 10) hours = "0" + hours; //  add a leading zero if needed
  String twelveTime;
  return twelveTime = hours + cur_Time;   // Put time string back together
}
