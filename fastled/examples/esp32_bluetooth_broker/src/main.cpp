/* --------- Project Libraries --------- */

#include <setup.hxx>

#ifdef DEBUG
#include <Logger.h>
#endif

/* --------- External Libraries --------- */
#include <FastLED.h>
#include <newton_fastled.hxx>
#include <etl/queue.h>

/* --------- Arduino Standard Libraries --------- */

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2904.h>

/* --------- Preprocessor macros --------- */
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

/* --------- Global Objects --------- */

namespace Lights
{
	/**
	 * Size of the light strip
	 */
	constexpr uint8_t Size = LIGHT_CONFIG_SIZE;

	/**
	 * Instruction queue
	 */
	etl::queue<PrismInstruction,
			   sizeof(PrismInstruction) * LIGHT_CONFIG_QUEUE_MAX,
			   etl::memory_model::MEMORY_MODEL_MEDIUM>
		InstructionQueue{};

	/**
	 * Newton FastLED interpreter
	 */
	NewtonFastLEDInterpreter<Size> NewtonInterpreter{};

}

namespace Bluetooth
{
	BLEAdvertising *Advertising;
	BLEServer *Server;
	BLEService *Service;

	namespace Characteristics
	{
		/* Newton command interpreter bluetooth characteristic */
		BLECharacteristic *Newton;
		BLE2904 NewtonDescriptor;
	}

	namespace Callbacks
	{
		struct : public BLECharacteristicCallbacks
		{
			/**
			 * On command write via bluetooth
			 */
			inline void onWrite(BLECharacteristic *characteristic) override
			{
				// Fetch the data
				uint16_t rawInstruction = *reinterpret_cast<uint16_t *>(characteristic->getData());

#ifdef DEBUG
				Logger.log<Level::T>(F("Bluetooth"), String{F("Requested: 0x")} + String(rawInstruction, 16));
#endif
				// Parse the instruction
				PrismInstruction instruction = Newton_ParseInstructionU16(rawInstruction);
#ifdef DEBUG
				switch (instruction.instruction)
				{
				case PRISM_INSTRUCTION_UPDATE:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("UPDT: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_INSTRUCTION_CLEAR:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("CLR: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_INSTRUCTION_SLEEP:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("SLP: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_INSTRUCTION_SELECT:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("SEL: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_INSTRUCTION_RANGE:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("RAN: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_INSTRUCTION_SET:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("SET: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_INSTRUCTION_FILL:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("FILL: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_INSTRUCTION_BLUR:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("EBLR: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_INSTRUCTION_LOADX:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("LDX: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_INSTRUCTION_LOADY:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("LDY: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_IGNORE_INSTRUCTION:
					Logger.log<Level::T>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("NOP: 0x")} + String(instruction.value, 16));
					break;

				case PRISM_EXCEPTION:
					Logger.log<Level::E>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("Unrecognized instruction: 0x")} + String(instruction.instruction, 16));
					break;

				default:
					Logger.log<Level::W>(F("Newton"),
										 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
											 String(*reinterpret_cast<uint8_t *>(&instruction.options), 10) + String{F("]\t->\t")} +
											 String{F("Unimplemented instruction: 0x")} + String(instruction.instruction, 16));
					break;
				}
#endif
				// Prompt newton interpreter (queue request)
				Lights::InstructionQueue.push(instruction);
			};

			/**
			 * On register read via bluetooth
			 */
			void onRead(BLECharacteristic *characteristic) override
			{
				// Grab the registers
				uint8_t xRegister = Lights::NewtonInterpreter.Registers.X;
				uint8_t yRegister = Lights::NewtonInterpreter.Registers.Y;

				// Concat registers (MSB is X register, LSB is Y register)
				uint16_t byteData = ((xRegister << 8) | yRegister);

				// Set the characteristic value
				characteristic->setValue(byteData);
#ifdef DEBUG
				Logger.log<Level::T>(
					F("Bluetooth"),
					String{F("Requested registers: [X -> 0x")} + String(xRegister, 16) +
						String{F("] [Y -> 0x")} + String(yRegister, 16) + String{F("]")});
#endif
			}

		} NewtonCharacteristicCallback;
	}
}

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
	Bluetooth::Service = Bluetooth::Server->createService(PRISM_SERVICE_UUID);

	Bluetooth::Characteristics::Newton = Bluetooth::Service->createCharacteristic(
		PRISM_NEWTON_CHARACTERISTIC_UUID,
		BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_READ);

	Bluetooth::Characteristics::NewtonDescriptor.setFormat(BLE2904::FORMAT_UINT16);
	Bluetooth::Characteristics::NewtonDescriptor.setUnit(0x2700);
	Bluetooth::Characteristics::Newton->addDescriptor(&Bluetooth::Characteristics::NewtonDescriptor);
	Bluetooth::Characteristics::Newton->setCallbacks(&Bluetooth::Callbacks::NewtonCharacteristicCallback);

	/* Bluetooth Advertising setup */
	Bluetooth::Advertising = BLEDevice::getAdvertising();
	Bluetooth::Advertising->addServiceUUID(PRISM_SERVICE_UUID);
	Bluetooth::Advertising->setAppearance(DEVICE_APPEARANCE);
	Bluetooth::Advertising->setScanResponse(true);

	/* FastLED configuration */
	FastLED.setMaxPowerInVoltsAndMilliamps(
		LIGHT_CONFIG_LED_MAX_VOLTS,
		LIGHT_CONFIG_LED_MAX_MAMPS);

	FastLED.addLeds<LIGHT_CONFIG_LED_TYPE, LIGHT_CONFIG_LED_DATA_PIN>(
		Lights::NewtonInterpreter.LedBuffer, LIGHT_CONFIG_SIZE);

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
		String{F("0x")} + String(Newton_WriteInstructionToU16(ExecuteInstruction), 16));
#endif
	// Execute the instruction and delay
	uint32_t DelayTime = Lights::NewtonInterpreter.Instruction(ExecuteInstruction);
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