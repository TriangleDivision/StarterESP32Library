#include <Arduino.h>
#include <TinyGPSPlus.h>
#include <BluetoothTerminal.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#include "commandProcessor.h"
#include "stateManager.h"
#include "securePreferences.h"
#include "utils.h"

///-----------------------------------------------------------------------------
/// Constants
///-----------------------------------------------------------------------------
const char* PreferencesNamespace = "commontest";
// All keys for preferences must be less than 15 characters or it fails silently
const char* KeySavedSSID = "SSID";
const char* KeySavedSSPwd = "ssidPW";

const char* BluetoothDeviceName = "CommonLibrariesTest";
const unsigned int StartupDelay = 500;

const int versionMajor = 1;
const int versionMinor = 0;
const int versionBuild = 0;

enum class DeviceState {
    Initialize,
    Running
};

///-----------------------------------------------------------------------------
/// Variables
///-----------------------------------------------------------------------------
Preferences preferences;
TinyGPSPlus gps;
HardwareSerial hardwareSerial(1);
BluetoothTerminal bluetoothTerminal;
CommandProcessor commandProcessor;
StateManager<DeviceState> stateManager;

String ssidSaved = "";
String passwordSaved = "";

// Function declarations
void loadPreferences();
void setupBluetooth();
void setupCommands();
void setupGPS();
void setupBluetooth();
void initializeState();
void handleConnectBLE(BLEDevice device);
void handleDisconnectBLE(BLEDevice device);
void handleReceiveBLE(const char *message);
String commandStatus(int argc, char* argv[]);
String commandSetupWifi(int argc, char* argv[]);
String commandSoftReset(int argc, char* argv[]);

extern void setupDisplay();
extern void drawDisplay();

///-----------------------------------------------------------------------------
/// Main Arduino setup/loop handlers
///-----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  stateManager.setDebugMode(true);
  stateManager.registerState(DeviceState::Initialize, onEnterInitialize, nullptr, nullptr);
  stateManager.registerState(DeviceState::Running, nullptr, onUpdateRunning, nullptr);
  stateManager.setState(DeviceState::Initialize);

  delay(StartupDelay);
  loadPreferences();
  setupGPS();
  setupBluetooth();
  setupCommands();
  setupDisplay();
}

void loop() {
  stateManager.update();
  commandProcessor.loop();
  bluetoothTerminal.loop();
  drawDisplay();
}

void loadPreferences() {
  preferences.begin(PreferencesNamespace);
  ssidSaved = SecurePreferences::readDecryptedString(preferences, KeySavedSSID);
  passwordSaved = SecurePreferences::readDecryptedString(preferences, KeySavedSSPwd);
  preferences.end();
}

///-----------------------------------------------------------------------------
/// State handlers
///-----------------------------------------------------------------------------
void onEnterInitialize() {
  Serial.println("Entering init");
  stateManager.setState(DeviceState::Running);
}

void onUpdateInitialize() {
  if (stateManager.getElapsedTime() < 100)
    Serial.println("Updating - only a few times");
}

void onExitInitialize() {
  Serial.println("Exiting init");
}

void onUpdateRunning() {
  if (stateManager.getElapsedTime() < 100)
    Serial.println("Updating - only a few times");
}

///-----------------------------------------------------------------------------
/// GPS handlers
///-----------------------------------------------------------------------------
void setupGPS() {
  hardwareSerial.begin(9600, SERIAL_8N1, A1, A0);  // RX, TX pins
}

bool readGPS() {
    while (hardwareSerial.available() > 0) {
      gps.encode(hardwareSerial.read());
    }

    // Check if the GPS module has a valid time
    if (gps.time.isValid()) {
      // Get the current time from the GPS module
      int newHours = gps.time.hour();
      int newMinutes = gps.time.minute();
      int newSeconds = gps.time.second();
      Serial.printf("Received GPS updated time (UTC-0): %02d:%02d:%02d\n", newHours, newMinutes, newSeconds);
      return true;
    }

    return false;
}

///-----------------------------------------------------------------------------
/// CommandProcessor handlers
///-----------------------------------------------------------------------------
void setupCommands() {
    commandProcessor.registerCommand("wifi", commandSetupWifi, "Connect to wifi network, specify SSID and password", 2, CommandProcessor::TypeString, CommandProcessor::TypeString);
    commandProcessor.registerCommand("status", commandStatus, "Get current device status", 0);
    commandProcessor.registerCommand("reset", commandSoftReset, "Perform soft reset", 0);
}

String commandSetupWifi(int argc, char* argv[]) {
}

String commandStatus(int argc, char* argv[]) {
  char statusInfo[512];
  int rssi = WiFi.RSSI();
  String signalInfo = WiFi.isConnected() ? String("online; signal strength ") + getSignalStrength(rssi) + "(" + rssi + ")" : "offline";

  sprintf(statusInfo, "Version %d.%d.%d\nConnected status = %s\n",
    versionMajor, versionMinor, versionBuild,
    ssidSaved,
    signalInfo.c_str());

  return String(statusInfo);
}

String commandSoftReset(int argc, char* argv[]) {
  float time = 1.0f;

  if (time > 60.0f)
    time = 60.0f;
  if (time < 0)
    time = 0;

  Serial.printf("Soft reset occurring in %2.2f seconds...", time);
  delay((int)(time * 1000.0f));
  esp_restart();

  // Will never get here, but that's ok
  return String("OK");
}

///-----------------------------------------------------------------------------
/// Bluetooth handlers
///-----------------------------------------------------------------------------
void setupBluetooth() {
  bluetoothTerminal.setName(BluetoothDeviceName);
  bluetoothTerminal.onConnect(handleConnectBLE);
  bluetoothTerminal.onDisconnect(handleDisconnectBLE);
  bluetoothTerminal.onReceive(handleReceiveBLE);
  bluetoothTerminal.start();
}

void handleConnectBLE(BLEDevice device) {
  Serial.println("BLE device connected");
}

void handleDisconnectBLE(BLEDevice device) {
  Serial.println("BLE device disconnected");
}

void handleReceiveBLE(const char *message) {
  Serial.printf("BLE message received: %s\n", message);
  if(commandProcessor.processCommand(message)) {
    bluetoothTerminal.send(commandProcessor.getLastResult().c_str());
  } else {
    bluetoothTerminal.send(commandProcessor.getLastStatus().c_str());
  }
}