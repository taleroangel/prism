#pragma once

#include <FastLED.h>
#include <etl/queue.h>
#include <newton/core.hxx>
#include <newton/drivers/fastled.hxx>

#include <setup.hxx>
#include <cstdint>

namespace Lights
{
	/**
	 * Size of the light strip
	 */
	constexpr uint8_t Size = LIGHT_CONFIG_SIZE;

	/**
	 * Instruction queue
	 */
	etl::queue<newton::core::Instruction,
			   sizeof(newton::core::Instruction) * LIGHT_CONFIG_QUEUE_MAX,
			   etl::memory_model::MEMORY_MODEL_MEDIUM>
		InstructionQueue{};

	/**
	 * Newton FastLED interpreter
	 */
	newton::drivers::fastled::Interpreter<Size> NewtonInterpreter{};

}