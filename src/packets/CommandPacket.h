/******************************************************************
 * Smart Egg Incubator - Command Packet
 * JSON command structure definition
 * 
 * Commands are received on: egg_incubator/command
 * Parsed in IncubatorApp::handleCommand()
 * 
 * Supported commands:
 * 
 * Set target temperature:
 *   {"cmd": "set_temp", "value": 37.8}
 * 
 * Set fan speed (0-100%):
 *   {"cmd": "set_fan_speed", "value": 75}
 * 
 * Trigger manual tray turn:
 *   {"cmd": "turn_tray"}
 * 
 * Force heater OFF:
 *   {"cmd": "heater_off"}
 * 
 * Set heater to AUTO mode:
 *   {"cmd": "heater_auto"}
 * 
 * Reboot ESP32:
 *   {"cmd": "reboot"}
 ******************************************************************/

#ifndef COMMAND_PACKET_H
#define COMMAND_PACKET_H

// Commands are parsed directly in IncubatorApp::handleCommand()
// using ArduinoJson. This header serves as documentation only.

#endif
