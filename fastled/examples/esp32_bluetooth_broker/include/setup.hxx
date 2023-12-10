#ifndef __SETUP_HXX__
#define __SETUP_HXX__

/* --------- Bluetooth Configuration --------- */
#define DEVICE_NAME "Prism ESP32 Broker"
#define DEVICE_APPEARANCE 0x0595UL

/* --------- Prism BLE UUID --------- */
#define PRISM_SERVICE_UUID "a7a37338-0c20-4779-b75a-089c6d7a0ac8"
#define PRISM_NEWTON_CHARACTERISTIC_UUID "4e639365-9e62-4d81-8e3d-f0d2bde4ccc6"

/* --------- LED Configuration --------- */
#include <FastLED.h>

#define LIGHT_CONFIG_SIZE 30UL

#define LIGHT_CONFIG_LED_TYPE NEOPIXEL
#define LIGHT_CONFIG_LED_DATA_PIN 4UL

#define LIGHT_CONFIG_LED_MAX_VOLTS 5
#define LIGHT_CONFIG_LED_MAX_MAMPS 850

/* --------- Debugging --------- */
#ifdef DEBUG
#define SERIAL_BAUD_RATE 115200UL
#endif

#endif