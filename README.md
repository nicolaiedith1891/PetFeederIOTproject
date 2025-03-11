# 🐾 Smart IoT Pet Feeder

## 📌 Project Overview
The **Smart IoT Pet Feeder** is an **ESP8266-based automated pet feeder** that allows pet owners to **schedule feedings, track food levels, and control feeding remotely** via Adafruit IO and Google Assistant. This ensures your pet is fed on time, even when you're not home.

## 🎯 Features
✅ **Remote Feeding** - Feed your pet via Adafruit IO dashboard or voice commands.  
✅ **Scheduled Feeding** - Automatically dispenses food every **6 hours** when enabled.  
✅ **Food Level Monitoring** - Uses an **ultrasonic sensor** to measure food levels and display them on Adafruit IO.  
✅ **Google Assistant Integration** - Activate feeding with voice commands.  
✅ **Real-time Monitoring** - Displays feed count, last feed timestamp, and food level percentage.  
✅ **Automatic Feed Reset** - Daily reset of feed count at midnight.  

---
## 🛠️ Hardware Requirements

| Component            | Quantity | Description |
|----------------------|----------|-------------|
| **ESP8266 NodeMCU** | 1        | WiFi-enabled microcontroller |
| **Servo Motor**      | 1        | Controls the feeder mechanism |
| **Ultrasonic Sensor (HC-SR04)** | 1 | Measures food level |
| **LEDs (Red, Green)**   | 2        | Display Status |
| **5V Power Supply**  | 1        | Powers the system |
| **Jump Wires**       | Several  | Connects components |
| **Food Container**   | 1        | Stores pet food |

---
## 🔌 Wiring Diagram

![Wiring Diagram](https://github.com/nicolaiedith1891/PetFeederIOTproject/blob/main/Images/Circuit%20pinout.png)

### **ESP8266 Pin Connections**

| Component | ESP8266 Pin |
|-----------|------------|
| LED (Green) | D2 |
| Servo Motor | D4 |
| Trig (Ultrasonic) | D5 |
| Echo (Ultrasonic) | D6 |

---
## 💾 Software & Setup
### **1️⃣ Install Required Libraries**
Ensure you have the following Arduino libraries installed:

- [ESP8266WiFi](https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/src/ESP8266WiFi.h/)
- [Adafruit MQTT](https://github.com/adafruit/Adafruit_MQTT_Library)
- [Servo](https://www.arduino.cc/en/reference/servo)
- [Wire](https://github.com/esp8266/Arduino/blob/master/libraries/Wire/Wire.h)
- [NTPClient](https://github.com/arduino-libraries/NTPClient)

### **2️⃣ Adafruit IO Setup**
1. Create an account on [Adafruit IO](https://io.adafruit.com/).
2. Create **3 feeds**:
   - `feed_count` (Integer) - Tracks the number of feedings.
   - `feed_timestamp` (String) - Stores the last feeding time.
   - `food_level` (Integer) - Displays food level percentage.
   - `scheduled_feed` (Boolean) - Toggles scheduled feeding mode.
3. Copy your **Adafruit IO Key & Username** into your code.

### **3️⃣IFTT Servlet Setup**
- Create a new account on [IFTT Website](https://ifttt.com/explore) with your **Google** Account.
- Go to **Create** -> **If This**, and Chose **Google Assistant** and Select **Activate Scene**.
- Give the Scene name as **pet feeder**.
- Select the **Then That** and Chose **Adafruit** as the output.
- Enter the feilds with your credentials and fill the textspaces as below.
- Save the Scene.
  
![If This](https://github.com/nicolaiedith1891/PetFeederIOTproject/blob/main/Images/If%20This.png)
![Then That](https://github.com/nicolaiedith1891/PetFeederIOTproject/blob/main/Images/Then%20That.png)

### **4️⃣ Setup Google Assistant**
- Download **[Google Home](https://play.google.com/store/apps/details?id=com.google.android.apps.chromecast.app&hl=en)** app from google Play.
- Sign in with the same Google Account used for IFTT.
- Go to Services -> Works With Google.
- Add and Approve the IFTT account.
- (Optional) Create a New **Routine** to customize the voice command.

### **5️⃣ Upload the Code**
- Open **Arduino IDE**.
- Copy and paste the [IoT Pet Feeder Code](https://github.com/nicolaiedith1891/PetFeederIOTproject/blob/main/petFeeder/petFeeder.ino).
- Upload it to your ESP8266 board.

---
## 📡 How It Works
### **🐾 Feeding Mechanism**
- When the **ON** command is sent via **Adafruit IO or Google Assistant**, the servo rotates, dispensing food.
- The **feed count** and **timestamp** are updated in the dashboard.

### **📊 Food Level Monitoring**
- The **HC-SR04 ultrasonic sensor** measures the distance from the lid to the food.
- The system converts this into a **percentage (100% = full, 0% = empty)** and displays it on Adafruit IO.

### **⏳ Scheduled Feeding Mode**
- When enabled, the system will **automatically dispense food every 6 hours**.
- Can be turned **ON/OFF via Adafruit IO**.

---
## 📱 Dashboard & Google Assistant
### **Adafruit IO Dashboard**
![Dashboard](https://github.com/nicolaiedith1891/PetFeederIOTproject/blob/main/Images/Dashboard.png)
- Toggle scheduled feeding mode.
- View live feed count, timestamp, and food level.

### **Google Assistant Voice Commands**
```
"Hey Google, feed my pet."
"Hey Google, feed max." (With custom Routine)
```

---
## 💡 Contributing
Pull requests are welcome! Feel free to improve the system. 😃

---
## 📜 License
This project is **open-source** under the MIT License.

🚀 **Enjoy smart pet feeding!** 🐾

