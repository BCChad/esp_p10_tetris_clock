/* Contains functions created for IoT to allow easy migration to other programs
 *  Bruce Chadbourne 10/27/2018
 */
 
// AdaFruit IoT Manager
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "<your router>"
#define WLAN_PASS       "<your router password>"

/************************* Adafruit.io Setup *********************************/
//  Set up your account at io.adafruit.com
#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "<your username>"
#define AIO_KEY         "<your password>"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/
// Left in if you want to subscribe to anything later
// Setup feed for publishing changes
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
// Adafruit_MQTT_Publish potValue = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/potValue");

// Setup feeds called 'tetrisBrightness' and 'tetrisOnOff' for subscribing to changes from the IO site
Adafruit_MQTT_Subscribe tetrisBrightness = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/tetrisBrightness");
Adafruit_MQTT_Subscribe tetrisOnOff = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/tetrisOnOff");

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;
  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }
  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);  // wait 5 seconds
    retries--;
    if (retries == 0) {
      // basically die and wait for WDT to reset me
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
