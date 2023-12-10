#ifndef __LIBNEWTON_DRIVERS_BLUETOOTH_HXX__
#define __LIBNEWTON_DRIVERS_BLUETOOTH_HXX__

#ifndef __cplusplus
#error "This library is not C compatible, use clibnewton instead"
#endif

#include <cstdint>

namespace newton::drivers::bluetooth
{
	constexpr const char *ServiceUUID = "a7a37338-0c20-4779-b75a-089c6d7a0ac8";
	constexpr const char *NewtonCharacteristicUUID = "4e639365-9e62-4d81-8e3d-f0d2bde4ccc6";
	constexpr uint16_t NewtonCharacteristicUnit = 0x2700UL;
	constexpr uint8_t NewtonCharacteristicFormat = 0x06UL;
}

#endif
