/* --------- Project Libraries --------- */

#include <setup.hxx>

#ifdef DEBUG
#include <Logger.h>
#endif

/* --------- External Libraries --------- */
#include <FastLED.h>

#include <newton/core.hxx>
#include <newton/drivers/fastled.hxx>
#include <newton/drivers/bluetooth.hxx>

/* --------- Arduino Standard Libraries --------- */
#include <Arduino.h>

/* --------- Preprocessor macros --------- */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* --------- Global Objects --------- */

#include "lights.hxx"
#include "bluetooth.hxx"

/* --------- Setup and Loop --------- */

void setup()
{
#ifdef DEBUG
	/* Set LED to indicate DEBUG mode */
	pinMode(GPIO_NUM_2, OUTPUT);
	digitalWrite(GPIO_NUM_2, HIGH);

	/* Serial Logging */
	Serial.begin(SERIAL_BAUD_RATE);

	Logger.begin(&Serial, Level::ALL);
	Logger.log<Level::I>(F("Setup"), F("Serial speed: " STR(SERIAL_BAUD_RATE)));
#endif

	/* Bluetooth Service setup */
	BLEDevice::init(DEVICE_NAME);
#ifdef DEBUG
	Logger.log<Level::I>(F("Network"), String{F("MAC -> ")} + BLEDevice::getAddress().toString().c_str());
#endif
	Bluetooth::Server = BLEDevice::createServer();
	Bluetooth::Service = Bluetooth::Server->createService(newton::drivers::bluetooth::ServiceUUID);

	Bluetooth::Characteristics::Newton = Bluetooth::Service->createCharacteristic(
		newton::drivers::bluetooth::NewtonCharacteristicUUID,
		BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);

	Bluetooth::Characteristics::NewtonDescriptor.setFormat(BLE2904::FORMAT_UINT16);
	Bluetooth::Characteristics::NewtonDescriptor.setUnit(newton::drivers::bluetooth::NewtonCharacteristicUnit);
	Bluetooth::Characteristics::Newton->addDescriptor(&Bluetooth::Characteristics::NewtonDescriptor);
	Bluetooth::Characteristics::Newton->setCallbacks(&Bluetooth::Callbacks::NewtonCharacteristicCallback);

	/* Bluetooth Advertising setup */
	Bluetooth::Advertising = BLEDevice::getAdvertising();
	Bluetooth::Advertising->addServiceUUID(newton::drivers::bluetooth::ServiceUUID);
	Bluetooth::Advertising->setAppearance(DEVICE_APPEARANCE);
	Bluetooth::Advertising->setScanResponse(true);

	/* FastLED configuration */
	FastLED.setMaxPowerInVoltsAndMilliamps(
		LIGHT_CONFIG_LED_MAX_VOLTS,
		LIGHT_CONFIG_LED_MAX_MAMPS);

	FastLED.addLeds<LIGHT_CONFIG_LED_TYPE, LIGHT_CONFIG_LED_DATA_PIN>(
		Lights::NewtonInterpreter.PublicBuffer, LIGHT_CONFIG_SIZE);

	/* Start Services */
	Bluetooth::Service->start();
	BLEDevice::startAdvertising();

	FastLED.clear();
	FastLED.show();

#ifdef DEBUG
	Logger.log<Level::I>(F("Bluetooth"), F("Device is now discoverable as "
										   "`" DEVICE_NAME "`"));
#endif
}

void loop()
{
	// Check if empty and ignore
	if (Lights::InstructionQueue.empty())
		return;

	// Get the instruction
	auto ExecuteInstruction = Lights::InstructionQueue.front();
	// Pop the instruction
	Lights::InstructionQueue.pop();

#ifdef DEBUG
	Logger.log<Level::D>(
		F("ExecutionLoop"),
		String{F("0x")} + String(newton::core::WriteInstructionToU16(ExecuteInstruction), 16));
#endif
	// Execute the instruction and delay
	uint32_t DelayTime = std::get<uint32_t>(Lights::NewtonInterpreter.Instruction(ExecuteInstruction));
	if (DelayTime > 0)
	{ /* Async delay lifetime block */

#ifdef DEBUG
		Logger.log<Level::I>(
			F("ExecutionTask"),
			String{F("Requested delay of (ms): ")} + String(DelayTime, 10));
#endif
		// Obtain times
		uint32_t _startTime = millis();
		uint32_t _currentTime;

		do // Loop until delay finishes
		{
			yield();
			_currentTime = millis();
		} while ((_currentTime - _startTime) < DelayTime);

#ifdef DEBUG
		Logger.log<Level::D>(F("ExecutionTask"), F("End of delay"));
#endif
	}

	// Show updates on LEDs
	FastLED.show();
}