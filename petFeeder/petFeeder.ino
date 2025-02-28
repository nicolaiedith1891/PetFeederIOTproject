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

// Publish feeds to Adafruit IO
Adafruit_MQTT_Publish feed_count = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/feed_count");
Adafruit_MQTT_Publish feed_timestamp = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/feed_timestamp");
Adafruit_MQTT_Publish food_level = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/food_level");

// Servo motor setup
Servo feederServo;
#define SERVO_PIN D4  // Use the correct GPIO pin

// Ultrasonic Sensor Pins
#define TRIG_PIN D5
#define ECHO_PIN D6

// Feed Counter Variables
int feedCounter = 0;

// NTP Client Setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); // GMT +5:30 for Sri Lanka
int lastResetDay = -1;  // Store the last day when the counter was reset

void setup() {
  Serial.begin(9600);

  // Connect to WiFi
  Serial.print("\n\nConnecting Wifi... ");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
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

  // Initialize Ultrasonic Sensor Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
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

  // Measure and Publish Food Level
  int level = getFoodLevel();
  if (!food_level.publish(level)) {
    Serial.println("Failed to publish food level");
  } else {
    Serial.print("Food Level Published: ");
    Serial.println(level);
  }

  // Read from our subscription queue until we run out, or wait up to 5 seconds for an update
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoff) {
      Serial.print("onoff: ");
      Serial.println((char*) onoff.lastread);
      
      // Move servo based on the command
      if (!strcmp((char*) onoff.lastread, "ON")) {
        feederServo.write(180); // Open feeder completely
        delay(2000); // Keep it open for 2 seconds
        feederServo.write(0); // Close feeder

        // Increment feed counter
        feedCounter++;

        // Get the current timestamp
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
  if (!mqtt.ping()) {
    mqtt.disconnect();
  }
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

// Function to measure food level using the ultrasonic sensor
int getFoodLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2; // Convert to cm

  // Assuming max distance (empty) is 20 cm and min (full) is 5 cm
  int level = map(distance, 20, 5, 0, 100);
  level = constrain(level, 0, 100); // Keep within 0-100%
  return level;
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) return;

  Serial.print("Connecting to MQTT... ");
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) while (1);
  }
  Serial.println("MQTT Connected!");
}
