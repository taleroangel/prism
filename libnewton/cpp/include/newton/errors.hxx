#ifndef __LIBNEWTON_ERRORS_HXX__
#define __LIBNEWTON_ERRORS_HXX__

#ifndef __cplusplus
#error "This library is not C compatible, use clibnewton instead"
#endif

namespace newton
{
	enum class Error
	{
		INVALID_VARIABLE_CODE,
		EXCEPTION_INSTRUCTION,
	};
}

#endif // _LIBNEWTON_ERRORS_H__
