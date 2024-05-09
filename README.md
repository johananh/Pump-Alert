# Pump Alert System

## Description
This project is designed as a monitoring and alert system for water pumps, which are activated by water level switches to drain excess water and deactivated when water levels are low. It employs an ESP01 microcontroller with a current transformer coil to sample the pumps current. This setup helps identify operational irregularities such as excessive running times or prolonged periods of inactivity, which could signify problems like a disconnected pipe or a malfunctioning pump. Alerts are sent via Telegram.

## Features

- **Current Monitoring**: Uses a current transformer to monitor if pump is on or off.
- **Configurable Alerts**: Users can set maximum run times and downtime through a web interface.
- **Daily Status Updates**: Sends daily "All OK" messages if the system is functioning normally.
- **Telegram Notifications**: Alerts are sent via the Telegram API, allowing for real-time notifications.
- **Web Interface**: Accessible at `pump.local` (if mDNS is enabled) or via the device IP, which is communicated through Telegram after power up.
<img src="Images/webPage.jpg" width="30%" alt="Web Interface Screenshot">

## What You Need

1. A small AC to DC 3.3V module
2. An ESP01 microcontroller
3. A current transformer coil
4. Two diodes, a capacitor, and  two resistors as per the schematics provided below:
<img src="Images/schematics.png" width="50%" alt="Schematics">

## :warning: Warning
**WARNING:** This project involves working with mains voltage, which can be hazardous. It should only be attempted by professionals who have the necessary knowledge and experience in handling high voltage electrical systems. Ensure all safety protocols are followed to prevent any risk of injury or damage.

### Disclaimer
*The information provided in this document is for educational purposes only. Anyone using this information does so at their own risk. The author or publisher of this document assumes no responsibility for any injuries or damage that may occur.*

## Assembly
Assembly is rather non-critical; just make sure your power supply is well away from the low voltage electronics. Here is how I did it:

<img src="Images/circuit.jpg" width="50%" alt="Circuit Setup">
<img src="Images/inBox.jpg" width="50%" alt="Project in Box">

### Software Setup
Follow these steps to configure the software for the project:

1. **Clone the Repository**
   Clone this repository to your local machine using your preferred method.

2. **Create a Telegram Bot**
   Set up a Telegram bot using BotFather. Detailed instructions on creating a Telegram bot and obtaining your Telegram token and chat ID can be found here:
   - [Creating a Telegram Bot with BotFather](https://arabind-meher.medium.com/creating-a-telegram-bot-with-botfather-a-step-by-step-guide-605e954de647)
   - [Finding Your Telegram Chat ID](https://www.wikihow.com/Know-Chat-ID-on-Telegram-on-Android#Finding-Your-Personal-Chat-ID)

3. **Open and Configure the Project**
   Open the project with PlatformIO or your preferred IDE. Insert your WiFi credentials and Telegram bot details in the designated sections of the code:

   ```c++
   // select te pulses input pin:
   
   const int inputPin = 3; //  RX pin used as input ESP01
   //const int inputPin = 5; //  node MCU use D1 (GPIO5) pin as input

   // time is needed for daily OK msg:
   NTPClient timeClient(ntpUDP, "pool.ntp.org", 10800, 600000); // Adjust the second parameter to the offset from UTC in seconds.

    // WiFi credentials
   //const char* ssid = "your_wifi_ssid"; //uncomment and set if you don't want to use WiFi Manager
   //const char* password = "your_wifi_password"; // Same, needed only if you don't want to use WiFi Manager

   // Telegram Bot Token and Chat ID - must be provided
   const String botToken = "your_bot_token"; // Replace with your Telegram bot token
   const String chatId = "your_chat_id"; // Replace with your chat ID

### Install and Build with PlatformIO
If you haven't already, follow these steps to install PlatformIO in Visual Studio Code and build the project:
1. Install the **PlatformIO** extension for Visual Studio Code from the VS Code marketplace.
2. Open the project folder in Visual Studio Code.
3. Build the project by accessing the PlatformIO: Build option from the PlatformIO icon in the activity bar or using the shortcut `Ctrl+Alt+B`.

### Hardware Compatibility
This project was developed using the ESP01 module, but it is compatible with any ESP8266 board. Boards with built-in USB ports are recommended for easier programming.

### Initial WiFi Configuration (if WiFi manager is used)
On the first power-up, configure the device to connect to your local WiFi network by following these steps:
1. Set your mobile device's WiFi to connect to the "Pump Alert AP" access point.
2. Open a web browser and navigate to `192.168.1.4` — the default IP address for the ESP AP.
3. A configuration page should appear. Select your WiFi network and enter the password to connect the device to your local WiFi network. 

## Configuration
- **Access the Web Interface**: Connect to the pump's web interface provided by the ESP01 via WiFi.
- **Set Maximum Run Time and Downtime**: Specify the maximum allowed run time and downtime for the pump.
- **Configure Daily Operational Check**: Set the time for the "Daily Ok" message. Set this to 00:00 will disablr this dayly message.
- **Train Mode**: If "Train Mode" is enabled, a message will be sent each time the pump turns on or off, indicating the duration the pump was in the last state.
- **Display Time Metrics**: The interface displays the maximum on-time and off-time since the last power up.

## Configuration
- **Access the Web Interface**: Connect to the pump's web interface provided by the ESP01 via WiFi.
- **Set Maximum Run Time and Downtime**: Specify the maximum allowed run time and downtime for the pump.
- **Configure Daily Operational Check**: Set the time for the "Daily OK" message. Setting this to 00:00 will disable this daily message.
- **Train Mode**: When "Train Mode" is enabled, a message will be sent each time the pump turns on or off, indicating the duration the pump was in the last state.
- **Display Time Metrics**: The interface displays the maximum on-time and off-time since the last power up.

## Usage
Once installed and configured, the system will monitor the pump's activity. Alerts will be sent through Telegram if:
- The pump runs longer than the set maximum time.
- The pump hasn't activated after the set maximum downtime.
- A daily check (if everything is normal) will be sent at the specified time.
- If "Train Mode" is enabled, a message is sent each time the pump's state changes.
- The LED will blink briefly every 2 seconds when the pump is idle and will stay on continuously when the pump is active.


## Contributing
Contributions to this project are welcome. Please fork this repository, make your changes, and submit a pull request for review.

## License
This project is licensed under custom license - see the [LICENSE](LICENSE) file for details.

