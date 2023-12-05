#include <Arduino.h>
#include <Logger.h>

void setup()
{
	// First enable Serial
	Serial.begin(9600);
	delay(2000);

	// Begin logger with Serial
	Logger.begin(&Serial, Level::ALL);

	// Log!
	Logger.log<Level::T>(F(__FILE__), F("Code is being executed correctly!"));
	Logger.log<Level::D>(F(__FILE__), F("Developer secret!"));
	Logger.log<Level::I>(F(__FILE__), F("Hello world!"));
	Logger.log<Level::W>(F(__FILE__), F("This isn't quite right"));
	Logger.log<Level::E>(F(__FILE__), F("Things are not working out"));
	Logger.log<Level::F>(F(__FILE__), F("Something is messed up"));
}

void loop()
{
}