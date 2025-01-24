#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

// Function to pad a binary string to a specified number of bits
static String padBinary(int value, int bits) {
  String binary = String(value, BIN);
  while (binary.length() < bits) {
    binary = "0" + binary;
  }
  return binary;
}

// Function to get the signal strength description based on RSSI value
static String getSignalStrength(int32_t rssi) {
  if (rssi >= -50) {
    return "excellent";
  } else if (rssi >= -60) {
    return "good";
  } else if (rssi >= -70) {
    return "fair";
  } else if (rssi >= -80) {
    return "weak";
  } else if (rssi >= -90) {
    return "very weak";
  } else {
    return "poor";
  }
}

#endif // UTILS_H
