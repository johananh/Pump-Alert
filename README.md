# Pump Alert System

## Description
This project is a monitoring and alert system for water pumps controlled by water level switches. These pumps are typically activated to drain excess water and turn off when the water level is low. The system uses an ESP01 microcontroller and a current transformer coil to monitor the pump's electrical current, detecting operational anomalies like excessive run times or extended inactivity, which might indicate issues such as a disconnected pipe or a malfunctioning pump.

## Features
- **Current Monitoring**: Uses a current transformer to monitor the electricity usage of the pump.
- **Configurable Alerts**: Users can set maximum run times and downtime through a web interface.
- **Daily Status Updates**: Sends daily "All OK" messages if the system is functioning normally.
- **Telegram Notifications**: Alerts are sent via Telegram API, allowing real-time notifications.

## Installation
### Hardware Setup
1. Connect the ESP01 to the current transformer coil according to the provided schematics.
2. Ensure the ESP01 is powered appropriately and can connect to the internet via WiFi.

### Software Setup
1. Clone this repository to your local machine.
2. Open the project with PlatformIO or your preferred IDE.
3. Insert your WiFi credentials, Telegram bot token, and chatID in the designated sections of the code.
4. Upload the code to the ESP01 using PlatformIO or through the Arduino IDE configured for ESP8266.

## Configuration
- Access the pump's web interface provided by the ESP01 via WiFi.
- Set the maximum allowed run time and downtime for the pump.
- Configure the time for the daily operational check message.

## Usage
Once installed and configured, the system will monitor the pump's activity. Alerts will be sent through Telegram if:
- The pump runs longer than the set maximum time.
- The pump hasn't activated after the set maximum downtime.
- A daily check (if everything is normal) will be sent at the specified time.

## Contributing
Contributions to this project are welcome. Please fork this repository, make your changes, and submit a pull request for review.

## License
This project is licensed under the MIT License - see the [LICENSE.md](LICENSE) file for details.

