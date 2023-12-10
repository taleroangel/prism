#ifndef __SETUP_HXX__
#define __SETUP_HXX__

/* --------- Bluetooth Configuration --------- */
#define DEVICE_NAME "Prism ESP32 Broker"
#define DEVICE_APPEARANCE 0x0595UL

/* --------- LED Configuration --------- */
#include <FastLED.h>

#define LIGHT_CONFIG_QUEUE_MAX 512UL
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