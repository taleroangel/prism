/**
 * @file Logger.h
 * @author Ángel Talero (angelgotalero@outlook.com)
 * @brief Arduino Logging framework, enables logging to multiple outputs
 * @version 0.1
 * @date 2022-11-01
 *
 * @copyright Copyright (c) 2022. Angel D. Talero
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <Arduino.h>

/**
 * @enum Level
 * @brief Logging levels based on Log4J levels + TRACE
 */
enum class Level
{
	OFF,
	/// @brief Fatal: Application cannot continue
	F,
	/// @brief Error: Process failed but application execution can continue
	E,
	/// @brief Warning: Process cannot continue
	W,
	/// @brief Info: Show information to system user
	I,
	/// @brief Debug: Show information to developer
	D,
	/// @brief Trace: Code is being executed correctly
	T,
	ALL,
};

/**
 * @brief Strings with Level names
 *
 */
constexpr const char *LOGGING_LEVEL_NAMES
	[static_cast<uint8_t>(Level::ALL)] PROGMEM = {
		"",
		"FATAL",
		"ERROR",
		"WARN",
		"INFO",
		"DEBUG",
		"TRACE"};

class LoggerClass
{
	/* --------- Variables --------- */
private:
	Print *printer = nullptr;
	Level level = Level::OFF;

	/* --------- Public member functions --------- */
public:
	/**
	 * @brief Attach a system printer (Print class)
	 * ej: &Serial
	 *
	 * @param printer Pointer to printer class
	 * @param level Logging level
	 */
	inline void begin(Print *printer, Level level = Level::ALL)
	{
		this->printer = printer;
		this->level = level;
	}

	LoggerClass() = default;

	/**
	 * @brief Get current logging level
	 * @note Logging level can only be set when begin() is invoked
	 * @return Level Logging level
	 */
	inline Level get_level() const
	{
		return this->level;
	}

	/**
	 * @brief Make a new log
	 * @note OFF and ALL logging levels are not valid
	 * @tparam level Logging level as a template parameter
	 * @tparam TagType Type of the TAG string
	 * @tparam MessageType  Type of the message string
	 * @param tag TAG of the log (Usually a component name or __FILE__ macro)
	 * @param content Content of the log
	 */
	template <Level level, typename TagType, typename MessageType>
	void log(TagType tag, MessageType content);
};

#ifndef LOGGER_DISABLE_GLOBAL_INSTANCE
/**
 * @brief Logging class global instance
 * @note can be disabled by defining LOGGER_DISABLE_GLOBAL_INSTANCE
 */
extern LoggerClass Logger;
#endif

/* --------- Template declarations --------- */
template <Level level, typename TagType, typename MessageType>
void LoggerClass::log(TagType tag, MessageType content)
{
	// Check current logging level
	if (level > this->level ||
		level == Level::OFF ||
		level == Level::ALL)
		return;

	// Print to files
	printer->print(LOGGING_LEVEL_NAMES[static_cast<uint8_t>(level)]);
	printer->print(F("\t["));
	printer->print(tag);
	printer->print(F("]\t"));
	printer->println(content);
	printer->flush();
}

#endif // __LOGGER_H__