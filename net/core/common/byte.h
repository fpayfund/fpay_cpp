#ifndef __SNOX_BYTE_H_INCLUDE_
#define __SNOX_BYTE_H_INCLUDE_

//typedef uint8_t Byte32[32];
//typedef uint8_t Byte4[4];

struct Byte32 {
	uint8_t u8[32];
};

struct Byte16 {
	uint8_t u8[16];
};

inline bool operator == (const Byte32& b1, const Byte32& b2) 
{
	for( uint32_t i = 0; i < 32; i++ )
	{
		if( b1.u8[i] != b2.u8[i] ) return false;
	}
	return true;
}

inline bool operator > (const Byte32& b1, const Byte32& b2)
{
	for( uint32_t i =0 ; i < 32; i++ )
	{
		if( b1.u8[i] > b2.u8[i] ) return true;
		if( b1.u8[i] == b2.u8[i] ) continue;
		if( b1.u8[i] < b2.u8[i] ) return false;
	}

	return false;
}

inline bool operator < ( const Byte32& b1, const Byte32& b2)
{
	for( uint32_t i =0 ; i < 32; i++ )
	{
		if( b1.u8[i] < b2.u8[i] ) return true;
		if( b1.u8[i] == b2.u8[i] ) continue;
		if( b1.u8[i] > b2.u8[i] ) return false;
	}
	return false;
}


struct compByte32 
{
	bool operator()( const Byte32& lb, const Byte32& rb)
	{
		return lb < rb;
	}
};

inline bool operator == (const Byte16& b1, const Byte16& b2) 
{
	for( uint32_t i = 0; i < 16; i++ )
	{
		if( b1.u8[i] != b2.u8[i] ) return false;
	}
	return true;
}

inline bool operator > (const Byte16& b1, const Byte16& b2)
{
	for( uint32_t i =0 ; i < 16; i++ )
	{
		if( b1.u8[i] > b2.u8[i] ) return true;
		if( b1.u8[i] == b2.u8[i] ) continue;
		if( b1.u8[i] < b2.u8[i] ) return false;
	}

	return false;
}

inline bool operator < ( const Byte16& b1, const Byte16& b2)
{
	for( uint32_t i =0 ; i < 4; i++ )
	{
		if( b1.u8[i] < b2.u8[i] ) return true;
		if( b1.u8[i] == b2.u8[i] ) continue;
		if( b1.u8[i] > b2.u8[i] ) return false;
	}
	return false;
}

struct compByte16
{
	bool operator()(const Byte16& lb, const Byte16& rb)
	{
		return lb < rb;
	}
};
#endif

