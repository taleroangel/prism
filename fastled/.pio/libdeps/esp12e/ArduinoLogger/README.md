# ArduinoLogger 📜
Arduino Logging framework, easy Logging for microcontrollers using the Arduino framework

## ⚡️ Usage

1. First include the Logging.h header, once the file is included a global object called ___Logger___ will be available in every file that includes it
```c++
#include <Logger.h>
```

2. Enable the interface you want to use, in this example the Serial interface will be used. once initialized pass it's memory address to the ___Logger___ object via __begin()__ method. you may also specify a ___LoggingLevel___, if not specified the default is __LoggingLevel::ALL__
```c++
Serial.begin(9600);
Logger.begin(&Serial, Level::ALL);
```

3. You are now able to log, specify the logging level via template and put a TAG and a MESSAGE as parameters.
```c++
Logger.log<Level::I>("My Tag", "Hello world!");
```

The resulting log will be:
```
INFO  [My Tag]  Hello world!
```
All printable elements with `Printer class` are allowed:
```c++
Logger.log<Level::D>(F("char *"), "You can print char *");
Logger.log<Level::I>(F("String"), String("Or a String class"));
Logger.log<Level::W>(F("F() or PSTR()"), F("Or a PSTR() macro"));
Logger.log<Level::E>("Hi, i'm a char * tag", F("TAG can also have different types"));
```

## 🌡️ Logging levels

| Level | Name  | Description                                                 |
| ----- | ----- | ----------------------------------------------------------- |
| OFF   | Off   | Turns off logging                                           |
| F     | Fatal | Severe errors that cause premature termination              |
| E     | Error | Other runtime errors or unexpected conditions               |
| W     | Warn  | Other runtime situations that are undesirable or unexpected |
| I     | Info  | Interesting runtime events (startup/shutdown)               |
| D     | Debug | Detailed information on the flow through the system         |
| T     | Trace | Flags to check code execution                               |
| ALL   | All   | Show everything                                             |

Access these levels by using the __Level__ enum

## 🛠️ Macro Options

* __LOGGER_DISABLE_GLOBAL_INSTANCE__: Disable global ___Logger___ instance, you need to create your own ___LoggerClass___ instances
