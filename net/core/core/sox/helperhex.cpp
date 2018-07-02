
#include "soxhelper.h"
#include <stdlib.h>
#include <stdexcept>

namespace sox
{

std::string hex2bin(const void * _src, size_t len, bool strict)
{
	const char * src = (const char *)_src; 

	std::string bin;
	char b = 0; char x = 0;
	for (; len > 0; ++src, --len)
	{
		char c = tolower(*src);
		if (c >= '0' && c <= '9') c -= '0';
		else if (c >= 'a' && c <= 'f') c -= 'a' - 10;
		else if (strict) throw std::runtime_error("hex2bin: error code");
		else continue;

		switch (x)
		{
		case 0: b = c << 4; x = 1; break;
		case 1: b |= c; x = 0; bin.append(1, b); break;
		}
	}
	if (strict && x != 0) throw std::runtime_error("hex2bin: error len");
	return bin;
}	

} // namespace sox
