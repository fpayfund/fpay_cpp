
#include "soxhelper.h"

///////////////////////////////////////////////////////////////////////////////
// Base64

namespace sox
{

static const unsigned char *EnBase64 =
	(const unsigned char *)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static unsigned char  DeBase64[] = {
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x3E,0xFF,0xFF,0xFF,0x3F,
	0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,
	0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0xFF,0xFF,0xFF,0xFF,0xFF,
	0xFF,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
	0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,0x30,0x31,0x32,0x33,0xFF,0xFF,0xFF,0xFF,0xFF
};

int Base64Decode(void *_dst, const void *_src, size_t len)
{
	size_t x = len & 0x3;
	if (0 == len || 1 == x) return 0;

	const unsigned char * src = (const unsigned char *)_src;
	unsigned char * dst = (unsigned char *)_dst;

	unsigned long dstlen = 0;
	unsigned long l, m ,n, o;

	if (len > 4)
	{
		for(size_t j = len / 4 - 1; dstlen < j; ++dstlen)
		{
			l = DeBase64[*src++];
			m = DeBase64[*src++];
			n = DeBase64[*src++];
			o = DeBase64[*src++];
			*dst++ = (unsigned char)((l<<2)|(m>>4));
			*dst++ = (unsigned char)((m<<4)|(n>>2));
			*dst++ = (unsigned char)((n<<6)|o);
		}
		dstlen = dstlen * 3;
	}

	l = DeBase64[*src++];
	m = DeBase64[*src++];
	*dst++ = (unsigned char)((l<<2)|(m>>4));
	++dstlen;

	if (0 == x) { x = 2; } else { x -= 2; if (0 == x) return dstlen; }
	if((n = *src++) != '=')
	{
		*dst++ = (unsigned char)((m<<4)|((n = DeBase64[n])>>2));
		++dstlen;
	}
	if (0 == --x) return dstlen;
	if((o = *src++) != '=')
	{
		*dst++ = (unsigned char)((n<<6)|DeBase64[o]);
		++dstlen;
	}

	return dstlen;
}

int Base64Encode(void *_dst, const void *_src, size_t len)
{
	const unsigned char * src = (const unsigned char *)_src;
	unsigned char * dst = (unsigned char *)_dst;
	unsigned char * odst = dst;

	unsigned long l, m, n; 

	for(; len >= 3; len -= 3)
	{
		l = *src++; m = *src++, n = *src++;
		*dst++ = EnBase64[l>>2];
		*dst++ = EnBase64[((l<<4)|(m>>4))&63];
		*dst++ = EnBase64[((m<<2)|(n>>6))&63];
		*dst++ = EnBase64[n&63];
	}

	switch(len)
	{
	case 1:
		l = *src++;
		*dst++ = EnBase64[l>>2];
		*dst++ = EnBase64[(l<<4)&63];
		*dst++ = '=';
		*dst++ = '=';
		break;

	case 2:
		l = *src++, m = *src++;
		*dst++ = EnBase64[l>>2];
		*dst++ = EnBase64[((l<<4)|(m>>4))&63];
		*dst++ = EnBase64[(m<<2)&63];
		*dst++ = '=';
		break;

	}
	return (int)(dst - odst);
}

} // namespace sox

#ifdef TEST
// g++34 -o b64 -DTEST -I../../ helperb64.cpp
#include <iostream>
#define ASSERT_EQUAL(a, b) if (a != b) { std::cout << a << " decode " << b; } 
void test(const std::string & c, const std::string & d)
{
	ASSERT_EQUAL(sox::base64decode(d), c);
}
int main(int argc, char * argv[])
{
	test("", ""); test("a", "YQ=="); test("ab", "YWI="); test("abc", "YWJj");
	test("", ""); test("a", "YQ"); test("ab", "YWI"); test("abc", "YWJj");
	test("", ""); test("a", "YQ=");
	if (argc > 1)
		std::cout << sox::base64encode(argv[1]) << std::endl;
	{
	std::string s("[]'?,.<>=ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
	srandomdev();
	for (int i=0; i<500; ++i)
	{
		std::random_shuffle(s.begin(), s.end());
		for (int j=0; j<s.size(); ++j)
			sox::base64decode(s.substr(random() % s.size()));
	}
	}
	return 0;
}
#endif

