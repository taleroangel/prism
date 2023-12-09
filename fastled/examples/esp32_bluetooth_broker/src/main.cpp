/* --------- Project Libraries --------- */

#include <setup.hxx>

#ifdef DEBUG
#include <Logger.h>
#endif

/* --------- Arduino Standard Libraries --------- */

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2904.h>

/* --------- C++ Standard Libraries --------- */

/* --------- Preprocessor macros --------- */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* --------- Global Objects --------- */
namespace Bluetooth
{
	BLEAdvertising *Advertising;
	BLEServer *Server;
	BLEService *Service;

	namespace Characteristics
	{
		/* Newton command interpreter */
		BLECharacteristic *Newton;
		BLE2904 NewtonDescriptor;
	};
};

namespace Lights
{
	constexpr uint8_t Size = LIGHT_CONFIG_SIZE;
};

/* --------- Setup and Loop --------- */

void setup()
{
	/* Serial Logging */
#ifdef DEBUG
	Serial.begin(SERIAL_BAUD_RATE);
	Logger.begin(&Serial, Level::ALL);
	Logger.log<Level::T>(F("Setup"), F("Serial speed: " STR(SERIAL_BAUD_RATE)));
#endif

	/* Bluetooth Service setup */
	BLEDevice::init(DEVICE_NAME);
#ifdef DEBUG
	Logger.log<Level::T>(F("Network"), String{F("MAC -> ")} + BLEDevice::getAddress().toString().c_str());
#endif
	Bluetooth::Server = BLEDevice::createServer();
	Bluetooth::Service = Bluetooth::Server->createService(PRISM_SERVICE_UUID);

	Bluetooth::Characteristics::Newton = Bluetooth::Service->createCharacteristic(
		PRISM_NEWTON_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);

	Bluetooth::Characteristics::NewtonDescriptor.setFormat(BLE2904::FORMAT_UINT16);
	Bluetooth::Characteristics::NewtonDescriptor.setUnit(0x2700);
	Bluetooth::Characteristics::Newton->addDescriptor(&Bluetooth::Characteristics::NewtonDescriptor);

	/* Bluetooth Advertising setup */
	Bluetooth::Advertising = BLEDevice::getAdvertising();
	Bluetooth::Advertising->addServiceUUID(PRISM_SERVICE_UUID);
	Bluetooth::Advertising->setAppearance(DEVICE_APPEARANCE);
	Bluetooth::Advertising->setScanResponse(true);

	/* Start Services */
	Bluetooth::Service->start();
	BLEDevice::startAdvertising();

#ifdef DEBUG
	Logger.log<Level::T>(F("Bluetooth"), F("Device is now discoveDevicerable as "
										   "`" DEVICE_NAME "`"));
#endif
}

void loop()
{
	yield();
}