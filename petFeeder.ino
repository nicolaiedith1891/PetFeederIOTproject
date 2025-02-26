#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define WIFI_SSID "D03CDialog1-6022"
#define WIFI_PASS "tharu4567"

#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "36IXTY"
#define MQTT_PASS "aio_HTOG529eFA6B6OdTifUEW3QuKXhS"

// WiFi & MQTT Clients
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

// Subscribe to "onoff" feed
Adafruit_MQTT_Subscribe onoff = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/onoff");

// Publish feed count and timestamp to Adafruit IO
Adafruit_MQTT_Publish feed_count = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/feed_count");
Adafruit_MQTT_Publish feed_timestamp = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/feed_timestamp");

// Servo motor setup
Servo feederServo;
#define SERVO_PIN D4  // Use the correct GPIO pin

// Variable to track feed activations
int feedCounter = 0;

// NTP Client Setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // GMT +5:30 for Sri Lanka (Adjust if needed)
int lastResetDay = -1;  // Store the last day when the counter was reset

void setup()
{
  Serial.begin(9600);

  // Connect to WiFi
  Serial.print("\n\nConnecting Wifi... ");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println(" Connected!");

  // Subscribe to MQTT feed
  mqtt.subscribe(&onoff);

  // Initialize servo
  feederServo.attach(SERVO_PIN);
  feederServo.write(0); // Start in closed position

  // Start NTP Client
  timeClient.begin();
}

void loop()
{
  MQTT_connect();
  
  // Update NTP time
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentDay = timeClient.getDay();

  // Reset feed count at midnight (00:00)
  if (currentHour == 0 && currentMinute == 0 && currentDay != lastResetDay) {
    feedCounter = 0;
    publishFeedCount();
    lastResetDay = currentDay;
    Serial.println("Feed count reset at midnight.");
  }

  // Read from our subscription queue until we run out, or wait up to 5 seconds for an update
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000)))
  {
    // If a subscription updated...
    if (subscription == &onoff)
    {
      Serial.print("onoff: ");
      Serial.println((char*) onoff.lastread);
      
      // Move servo based on the command
      if (!strcmp((char*) onoff.lastread, "ON"))
      {
        feederServo.write(180); // Open feeder completely
        delay(2000); // Keep it open for 2 seconds
        feederServo.write(0); // Close feeder

        // Increment feed counter
        feedCounter++;

        // Get the current timestamp in a human-readable format
        String timestamp = timeClient.getFormattedTime();
        Serial.print("Timestamp: ");
        Serial.println(timestamp);

        // Publish the new count and timestamp to Adafruit IO
        publishFeedCount();
        publishFeedTimestamp(timestamp);
      }
    }
  }

  // Keep MQTT connection alive
  if (!mqtt.ping())
  {
    mqtt.disconnect();
  }
}

void MQTT_connect() 
{
  int8_t ret;

  // Stop if already connected
  if (mqtt.connected()) 
  {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) // connect() returns 0 when connected
  { 
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) 
       {
         // If failed after retries, enter infinite loop to reset
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

// Function to publish the feed count
void publishFeedCount() {
  if (!feed_count.publish(feedCounter)) {
    Serial.println("Failed to publish feed count");
  } else {
    Serial.print("Feed Count Published: ");
    Serial.println(feedCounter);
  }
}

// Function to publish the feed timestamp
void publishFeedTimestamp(String timestamp) {
  if (!feed_timestamp.publish(timestamp.c_str())) {
    Serial.println("Failed to publish feed timestamp");
  } else {
    Serial.print("Feed Timestamp Published: ");
    Serial.println(timestamp);
  }
}
