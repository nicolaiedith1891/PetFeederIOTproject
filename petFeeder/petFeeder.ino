#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi Credentials
#define WIFI_SSID "D03CDialog1-6022"
#define WIFI_PASS "tharu4567"

// Adafruit IO Credentials
#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "36IXTY"
#define MQTT_PASS "aio_HTOG529eFA6B6OdTifUEW3QuKXhS"

// WiFi & MQTT Clients
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

// MQTT Feeds
Adafruit_MQTT_Subscribe onoff = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/onoff");
Adafruit_MQTT_Subscribe scheduled_feed = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/scheduled_feed");
Adafruit_MQTT_Publish feed_count = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/feed_count");
Adafruit_MQTT_Publish feed_timestamp = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/feed_timestamp");
Adafruit_MQTT_Publish food_level = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/food_level");

// Servo Motor
Servo feederServo;
#define SERVO_PIN D4

// Ultrasonic Sensor
#define TRIG_PIN D5
#define ECHO_PIN D6

// Pet Feeder Ready LED
#define LED D2 

// Variables
int feedCounter = 0;
bool scheduledFeeding = false;
unsigned long lastFeedTime = 0;

// NTP Client Setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000); 
int lastResetDay = -1;

void setup() {
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  mqtt.subscribe(&onoff);
  mqtt.subscribe(&scheduled_feed);

  feederServo.attach(SERVO_PIN);
  feederServo.write(0);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED, OUTPUT);
  
  timeClient.begin();
}

void loop() {
  MQTT_connect();
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentDay = timeClient.getDay();
  
  // Reset Feed Count at Midnight
  if (currentHour == 0 && currentMinute == 0 && currentDay != lastResetDay) {
    feedCounter = 0;
    publishFeedCount();
    lastResetDay = currentDay;
    Serial.println("Feed count reset at midnight.");
  }

  // Process MQTT messages
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) {
    if (subscription == &onoff) {
      handleManualFeeding();
    }
    if (subscription == &scheduled_feed) {
      scheduledFeeding = strcmp((char*)scheduled_feed.lastread, "ON") == 0;
      Serial.println(scheduledFeeding ? "Scheduled Feeding Enabled" : "Scheduled Feeding Disabled");
    }
  }
  
  // Scheduled Feeding Every 6 Hours
  if (scheduledFeeding && millis() - lastFeedTime >= 21600000) {
    handleManualFeeding();
    lastFeedTime = millis();
  }

  // Publish Food Level
  publishFoodLevel();

  if (!mqtt.ping()) {
    mqtt.disconnect();
  }
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
    if (--retries == 0) while (1);
  }
  Serial.println("MQTT Connected!");
  digitalWrite(LED, HIGH);
}

void handleManualFeeding() {
  feederServo.write(180);
  delay(2000);
  feederServo.write(0);
  feedCounter++;
  publishFeedCount();
  publishFeedTimestamp();
}

void publishFeedCount() {
  if (!feed_count.publish(feedCounter)) {
    Serial.println("Failed to publish feed count");
  } else {
    Serial.print("Feed Count Published: ");
    Serial.println(feedCounter);
  }
}

void publishFeedTimestamp() {
  String timestamp = timeClient.getFormattedTime();
  if (!feed_timestamp.publish(timestamp.c_str())) {
    Serial.println("Failed to publish feed timestamp");
  } else {
    Serial.print("Feed Timestamp Published: ");
    Serial.println(timestamp);
  }
}

void publishFoodLevel() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2; // Convert to cm
  
  const int MAX_DEPTH = 6;
  const int MIN_DEPTH = 1;

  int foodLevel = map(distance, MIN_DEPTH, MAX_DEPTH, 100, 0);
  
  foodLevel = constrain(foodLevel, 0, 100);

  if (!food_level.publish(foodLevel)) {
    Serial.println("Failed to publish food level");
  } else {
    Serial.print("Food Level Published: ");
    Serial.print(foodLevel);
    Serial.println("%");
  }
}

