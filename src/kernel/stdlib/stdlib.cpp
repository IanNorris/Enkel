#include "utilities/termination.h"

extern "C"
{
	void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function)
	{
		AssertionFailed(assertion, function, file, line, 0, 0, 0);
	}
}