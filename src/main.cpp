#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <EEPROM.h>
#include <ESP8266mDNS.h>
#include <Ticker.h>
#include "TelegramSender.h"
#include "WebPages.h"
#define USE_WIFIMANAGER // uncomment to use WiFi Manger (go to 198.162.4.1 after connecting to device genarated WiFi AP)

const int inputPin = 3; //  RX pin used as input ESP01
//const int inputPin = 5; //  node MCU use D1 (GPIO5) pin as input

#include "..\..\myCredntials.h" // my credtials on a different file
// const char *ssid = "your wifi ssid for hard coded wifi setup";
// const char *password = "your wifi password for hard coded setup";

/*******   Bot token and chatID must be provided !! ******************/
// const String botToken = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"; // your_bot token
// const String chatId = "*********"; // your chat ID

// Define the addresses where each parameter is stored in EEPROM
const int EEPROM_SIZE = 128;
const int ON_TIMEOUT_ADDR = 0;
const int OFF_TIMEOUT_ADDR = ON_TIMEOUT_ADDR + sizeof(int);
const int TRAIN_MODE_ADDR = OFF_TIMEOUT_ADDR + sizeof(int);
const int DAILY_OK_MSG_TIME_ADDR = TRAIN_MODE_ADDR + sizeof(bool);

// Constants
#define OFF false
#define ON true

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 10800, 600000); // 0 for UTC

const int ledPin = 2; // ESP-01S, node MCU blue led, for ESP01 connect external led to GOIO2

volatile int pulseCount = 0;
unsigned long lastTime = 0;         // Last time update
const unsigned long interval = 500; // Interval of 0.5 sec
int lastPinState;
bool pumpState = OFF;
unsigned long pumpOnTimestamp = 0;  // Timestamp when the pump turns on
unsigned long pumpOffTimestamp = 0; // Timestamp when the pump turns off
unsigned long pumpOnDuration = 0;   // Duration pump was on
unsigned long pumpOffDuration = 0;  // Duration pump was off
bool previousPumpState = OFF;
unsigned long maxOnDuration = 0, maxOffDuration = 0;
bool trainMode = false;
unsigned long onTimeout = 0;
unsigned long offTimeout = 0;
unsigned long dailyOkMessageTime = 0; // Time in seconds since midnight

// Global variables for tracking alert times
unsigned long lastOnAlertTime = 0;
unsigned long lastOffAlertTime = 0;
const unsigned long onAlertInterval = 600000; // 10 minutes in milliseconds
const unsigned long offAlertInterval = 600000;

ESP8266WebServer server(80);
TelegramSender bot(botToken, chatId);
Ticker ledTicker;

unsigned long secRTC = 0;
bool oneMinutFlag = false;
String formatTime(unsigned long seconds, String t);

void incRtc()
{
  static unsigned long prvMsec = 0;
  unsigned long ct = millis();
  // Update secRTC if the difference is greater than 1000 milliseconds
  if (ct - prvMsec >= 1000)
  {
    secRTC++; // Increment the seconds count
    prvMsec += 1000; // Update prvMsec to the next target time
    if (secRTC % 60 == 0)
    {
      oneMinutFlag = true;
    }
    if (secRTC % 600 == 0) // sync NPT every 10 minuts
    {
      if (timeClient.update())
      {
        unsigned long newTime = timeClient.getEpochTime();
        if (newTime != 0) // new time is valid ?
        {
          secRTC = newTime;
          Serial.print("Updated RTC to: ");
        }
      }
    }
    Serial.println(formatTime(secRTC, "HH:MM:SS"));
  }
}
void blinkLed()
{
  static bool ledState = false;

  digitalWrite(ledPin, ledState ? HIGH : LOW); // Toggle the LED state

  if (ledState)
  {
    // If the LED was on, turn it off and set the ticker for 600 ms
    ledTicker.attach(2.0, blinkLed);
  }
  else
  {
    // If the LED was off, turn it on and set the ticker for 50 ms
    ledTicker.attach(0.05, blinkLed);
  }

  ledState = !ledState; // Invert the state for next time
}
void saveSettings()
{
  EEPROM.put(ON_TIMEOUT_ADDR, onTimeout);
  EEPROM.put(OFF_TIMEOUT_ADDR, offTimeout);
  EEPROM.put(TRAIN_MODE_ADDR, trainMode);
  EEPROM.put(DAILY_OK_MSG_TIME_ADDR, dailyOkMessageTime);
  EEPROM.commit(); // Commit changes to save them
}

void loadSettings()
{
  EEPROM.get(ON_TIMEOUT_ADDR, onTimeout);
  EEPROM.get(OFF_TIMEOUT_ADDR, offTimeout);
  EEPROM.get(TRAIN_MODE_ADDR, trainMode);
  EEPROM.get(DAILY_OK_MSG_TIME_ADDR, dailyOkMessageTime);
}

String formatTime(unsigned long seconds, String t)
{
  char buf[48] = "Unkonwn time format";
  unsigned long days = seconds / 86400;
  unsigned long hrs = (seconds % 86400) / 3600;
  unsigned long mins = (seconds % 3600) / 60;
  unsigned long secs = seconds % 60;

  if (t == "DD:HH")
  {
    sprintf(buf, "%02lu:%02lu", days, hrs);
  }
  if (t == "MM:SS")
  {
    sprintf(buf, "%02lu:%02lu", mins, secs);
  }
  if (t == "HH:MM:SS")
  {
    sprintf(buf, "%02lu:%02lu:%02lu", hrs, mins, secs);
  }
  if (t == "HH:MM")
  {
    sprintf(buf, "%02lu:%02lu", hrs, mins);
  }
  if (t == "DD:HH:MM:SS")
  {
    sprintf(buf, "%02lu days, %02lu:%02lu:%02lu", days, hrs, mins, secs);
  }

  return String(buf);
}

void handleRoot()
{
  loadSettings();

  String html = INDEX_HTML; // Convert progmem string to String
  html.replace("{onTimeout}", formatTime(onTimeout, "MM:SS"));
  html.replace("{offTimeout}", formatTime(offTimeout, "DD:HH"));
  html.replace("{dailyOkTime}", formatTime(dailyOkMessageTime, "HH:MM"));
  html.replace("{trainModeChecked}", trainMode ? "checked" : "");
  html.replace("{maxOnTime}", formatTime(maxOnDuration, "HH:MM:SS"));
  html.replace("{maxOffTime}", formatTime(maxOffDuration, "DD:HH:MM:SS"));
  server.send(200, "text/html", html);
}

bool isValidTime(const String &timeStr, const String &format)
{
  int p1 = timeStr.indexOf(':');
  bool isValid = true;

  if (format == "MM:SS")
  {
    int mins = timeStr.substring(0, p1).toInt();
    int secs = timeStr.substring(p1 + 1).toInt();
    if (mins < 1 && secs < 1)
      isValid = false;
    if (mins >= 60 || secs >= 60)
      isValid = false;
  }
  if (format == "DD:HH")
  {
    int days = timeStr.substring(0, p1).toInt();
    int hours = timeStr.substring(p1 + 1).toInt();
    if (days == 0 && hours == 0)
      isValid = false;
    if (hours >= 24 || days > 14)
      isValid = false;
  }
  if (format == "HH:MM")
  {
    int hrs = timeStr.substring(0, p1).toInt();
    int mins = timeStr.substring(p1 + 1).toInt();

    if (mins >= 60 || hrs >= 24)
      isValid = false;
  }
  Serial.println(timeStr + " is " + (isValid ? "Valid" : "Not Valid"));
  return isValid;
}

void handleSetParameters()
{
  String onTimeStr = server.arg("onTimeout");
  String offTimeStr = server.arg("offTimeout");
 String dailyOkTimeStr = server.arg("dailyOkTime");
  trainMode = server.arg("trainMode") == "on";

  Serial.println(onTimeStr);
  Serial.println(offTimeStr);
  Serial.println(trainMode);

  if (!isValidTime(onTimeStr, "MM:SS") || !isValidTime(offTimeStr, "DD:HH") || !isValidTime(dailyOkTimeStr, "HH:MM"))
  {
    //    Serial.println("Invalid input detected, sending error response...");
    server.send(400, "text/plain",
                "Input Error\n"
                "Invalid time format:\n"
                "- On Time must be in MM:SS format.\n"
                "- Off Time must be in DD:HH format.\n"
                "- Daily OK Time must be in HH:MM format.\n"
                "- Values cannot be zero or negative.\n"
                "Correct the values and try again.");
    //   Serial.println("Error response sent.");
    return;
  }

  // Parse onTime MM:SS
  int onMinutes = onTimeStr.substring(0, onTimeStr.indexOf(':')).toInt();
  int onSeconds = onTimeStr.substring(onTimeStr.indexOf(':') + 1).toInt();
  // if ontime alert changes we rest max on time duration
  long unsigned int onT = onMinutes * 60 + onSeconds;
  if (onTimeout != onT)
  {
    onTimeout = onT;
    maxOnDuration = 0;
  }

  // Parse offTime DD:HH
  int days = offTimeStr.substring(0, offTimeStr.indexOf(':')).toInt();
  int hours = offTimeStr.substring(offTimeStr.indexOf(':') + 1).toInt();

  // if ontime alert changes we rest max off time duration
  onT = (days * 24 * 3600) + (hours * 3600); // Converting days and hours to seconds
  if (offTimeout != onT)
  {
    offTimeout = onT;
    maxOffDuration = 0;
  }
  // Parse daily OK time HH:MM
  int dailyOkHours = dailyOkTimeStr.substring(0, dailyOkTimeStr.indexOf(':')).toInt();
  int dailyOkMinutes = dailyOkTimeStr.substring(dailyOkTimeStr.indexOf(':') + 1).toInt();
  dailyOkMessageTime = dailyOkHours * 3600 + dailyOkMinutes * 60; // Convert hours and minutes to seconds
  saveSettings();
  // Serial.print("Parsed On Timeout Seconds: ");
  // Serial.println(onTimeout);
  // Serial.print("Parsed Off Timeout Seconds: ");
  // Serial.println(offTimeout);
  server.send(200, "text/plain", "Parameters updated successfully.");
}

void doOneMinute()
{
  if (oneMinutFlag)
  {
    if ((secRTC  % (24 * 60 *60) == dailyOkMessageTime) && (dailyOkMessageTime != 0))
    {
        bot.send("Daily Msg: Pump Alert OK");
    }
    oneMinutFlag = false;
  }
}

IRAM_ATTR void handleInterrupt()
{
  pulseCount++;
}

void setup()
{
  pinMode(ledPin, OUTPUT);    // Setup LED pin as output
  digitalWrite(ledPin, HIGH); // Turn off LED (active LOW on many boards)

  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY); // Start serial communication for debugging
  pinMode(inputPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(inputPin), handleInterrupt, RISING);
  lastPinState = digitalRead(inputPin); // Initialize lastPinState

  EEPROM.begin(EEPROM_SIZE);
  loadSettings();

#ifdef USE_WIFIMANAGER
  WiFiManager wifiManager;
  wifiManager.setTimeout(180); // Set timeout to 180 seconds before giving up
  if (!wifiManager.autoConnect("Pump Alert AP"))
  {
    Serial.println("Failed to connect and hit timeout");
    delay(3000);
    ESP.restart(); // Reset the ESP8266
  }
  Serial.println("WiFi connected via WiFiManager");
#else
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected manually");
#endif
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  if (MDNS.begin("pump"))
  {
    Serial.println("MDNS responder started at pump.local");
  }
  server.on("/", handleRoot);
  server.on("/setParameters", HTTP_POST, handleSetParameters);
  server.begin();
  blinkLed();
  Serial.println("HTTP server started");
  
  String message = "Pump Alert Restarted. IP: " + WiFi.localIP().toString();
  bot.send(message);
  ESP.wdtEnable(5000);
  timeClient.begin();
  if (timeClient.update())
  {
    secRTC = timeClient.getEpochTime();
    Serial.println(formatTime(timeClient.getEpochTime(), "HH:MM:SS"));
  }
  else
    Serial.println("NTP Update time failed");
}

int onAlertCount = 0;  // Counter for ON alerts
int offAlertCount = 0; // Counter for OFF alerts

void loop()
{
  unsigned long currentTime = millis();

  ESP.wdtFeed();
  if (currentTime - lastTime >= interval)
  {
    incRtc();
    doOneMinute();
    lastTime = currentTime;
    pumpState = (pulseCount > 10);

    // digitalWrite(ledPin, pumpState ? LOW : HIGH);
    pulseCount = 0;
    // Handle Pump state change
    if (pumpState != previousPumpState)
    {
      if (pumpState == ON)
      {
        ledTicker.detach();        // Stop blinking when pump is on
        digitalWrite(ledPin, LOW); // Turn LED on
        pumpOnTimestamp = currentTime;
        lastOnAlertTime = 0; // Reset off alert timer when pump turns on
        offAlertCount = 0;
        maxOffDuration = max(maxOffDuration, (currentTime - pumpOffTimestamp) / 1000);
        if (trainMode)
          bot.send("Pump On, was Off for " + formatTime((currentTime - pumpOffTimestamp) / 1000, "DD:HH:MM:SS"));
      }
      else
      { // pump changed to OFF
        blinkLed();
        pumpOffTimestamp = currentTime;
        lastOffAlertTime = 0; // Reset on alert timer when pump turns of
        onAlertCount = 0;
        maxOnDuration = max(maxOnDuration, (currentTime - pumpOnTimestamp) / 1000);
        if (trainMode)
          bot.send("Pump Off, was On for " + formatTime((currentTime - pumpOnTimestamp) / 1000, "HH:MM:SS"));
      }
      previousPumpState = pumpState;
    }
  }

#ifdef DEBUG
  Serial.print("Current Time: ");
  Serial.print(currentTime / 1000);
  if (pumpState)
  {
    Serial.print(" On time: ");
    Serial.print((currentTime - pumpOnTimestamp) / 1000);
    Serial.print(" ontimout: ");
    Serial.println(onTimeout);
  }
  else
  {
    Serial.print(" Off time: ");
    Serial.print((currentTime - pumpOffTimestamp) / 1000);
    Serial.print(" offtimout: ");
    Serial.println(offTimeout);
  }

#endif

  if (pumpState)
  {
    // pump ON
    if ((currentTime - pumpOnTimestamp) / 1000 > onTimeout)
    {
      if ((currentTime - lastOnAlertTime > onAlertInterval || lastOnAlertTime == 0) && onAlertCount < 10)
      {
        bot.send("Alert: Pump is ON for " + formatTime((currentTime - pumpOnTimestamp) / 1000, "HH:MM:SS"));
        lastOnAlertTime = currentTime;
        onAlertCount++; // Increment the ON alert counter
      }
    }
  }
  else
  { // pump Off
    if ((currentTime - pumpOffTimestamp) / 1000 > offTimeout)
    {
      if ((currentTime - lastOffAlertTime > offAlertInterval || lastOffAlertTime == 0) && offAlertCount < 10)
      {
        bot.send("Alert: Pump is OFF for " + formatTime((currentTime - pumpOffTimestamp) / 1000, "DD:HH:MM:SS"));
        lastOffAlertTime = currentTime;
        offAlertCount++; // Increment the OFF alert counter
      }
    }
  }

  // Maintain network and server operations
  server.handleClient();
  MDNS.update();
}
