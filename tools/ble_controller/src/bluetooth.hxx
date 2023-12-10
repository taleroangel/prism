#pragma once

#include "lights.hxx"
#include <newton/core.hxx>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2904.h>


#ifdef DEBUG
#include <Logger.h>
#endif

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

				// Swap the bytes (Big Endian to Little Endian)
				{ /* _auxiliaryBytes lifetime block */
					uint8_t _auxiliaryBytes = (rawInstruction & 0xFF00) >> 8UL;

					rawInstruction <<= 8UL;
					rawInstruction &= 0xFF00UL;
					rawInstruction |= (_auxiliaryBytes & 0x00FFUL);
				}

#ifdef DEBUG
				Logger.log<Level::T>(F("Bluetooth"), String{F("Requested: 0x")} + String(rawInstruction, 16));
#endif
				// Parse the instruction
				newton::core::Instruction instruction = newton::core::ParseInstructionU16(rawInstruction);
#ifdef DEBUG
				Logger.log<Level::T>(F("Newton"),
									 String{F("[0x")} + String(instruction.instruction) + String{F(" ; ")} +
										 String(*(uint8_t *)(&instruction.options), 10) + String{F(" ; ")} +
										 String{F("0x")} + String(instruction.value) + String{F("]")});
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
				uint8_t xRegister = Lights::NewtonInterpreter.InterpreterRegisters.X;
				uint8_t yRegister = Lights::NewtonInterpreter.InterpreterRegisters.Y;

				// Concat registers (MSB is X register, LSB is Y register)
				uint16_t byteData = ((yRegister << 8) | xRegister);

				// Set the characteristic value
				characteristic->setValue(byteData);
#ifdef DEBUG
				Logger.log<Level::D>(
					F("Bluetooth"),
					String{F("Requested registers (0x")} + String(byteData, 16) + String{F("): [X -> 0x")} +
						String(xRegister, 16) + String{F("] [Y -> 0x")} + String(yRegister, 16) + String{F("]")});
#endif
			}

		} NewtonCharacteristicCallback;
	}
}
