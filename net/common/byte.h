#ifndef __SNOX_BYTE_H_INCLUDE_
#define __SNOX_BYTE_H_INCLUDE_

//typedef uint8_t Byte32[32];
//typedef uint8_t Byte4[4];

struct Byte32 {
	unsigned char u8[32];
    Byte32() {
		for( uint32_t i = 0; i < 32; i++ ) {
			u8[i] = 0x00;
		}
	}
};

struct Byte20 {
	unsigned char u8[20];
	Byte20() {
		for( uint32_t i = 0; i < 20; i++ ) {
			u8[i] = 0x00;
		}
	}

};

struct Byte64 {
	unsigned char u8[64];
	Byte64() {
		for( uint32_t i = 0; i < 64; i++ ) {
			u8[i] = 0x00;
		}
	}
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

inline bool operator == (const Byte20& b1, const Byte20& b2) 
{
	for( uint32_t i = 0; i < 20; i++ )
	{
		if( b1.u8[i] != b2.u8[i] ) return false;
	}
	return true;
}

inline bool operator > (const Byte20& b1, const Byte20& b2)
{
	for( uint32_t i =0 ; i < 20; i++ )
	{
		if( b1.u8[i] > b2.u8[i] ) return true;
		if( b1.u8[i] == b2.u8[i] ) continue;
		if( b1.u8[i] < b2.u8[i] ) return false;
	}

	return false;
}

inline bool operator < ( const Byte20& b1, const Byte20& b2)
{
	for( uint32_t i =0 ; i < 20; i++ )
	{
		if( b1.u8[i] < b2.u8[i] ) return true;
		if( b1.u8[i] == b2.u8[i] ) continue;
		if( b1.u8[i] > b2.u8[i] ) return false;
	}
	return false;
}

struct compByte20
{
	bool operator()(const Byte20& lb, const Byte20& rb)
	{
		return lb < rb;
	}
};

inline bool operator == (const Byte64& b1, const Byte64& b2) 
{
	for( uint32_t i = 0; i < 64; i++ )
	{
		if( b1.u8[i] != b2.u8[i] ) return false;
	}
	return true;
}

inline bool operator > (const Byte64& b1, const Byte64& b2)
{
	for( uint32_t i =0 ; i < 64; i++ )
	{
		if( b1.u8[i] > b2.u8[i] ) return true;
		if( b1.u8[i] == b2.u8[i] ) continue;
		if( b1.u8[i] < b2.u8[i] ) return false;
	}

	return false;
}

inline bool operator < ( const Byte64& b1, const Byte64& b2)
{
	for( uint32_t i =0 ; i < 64; i++ )
	{
		if( b1.u8[i] < b2.u8[i] ) return true;
		if( b1.u8[i] == b2.u8[i] ) continue;
		if( b1.u8[i] > b2.u8[i] ) return false;
	}
	return false;
}


struct compByte64 
{
	bool operator()( const Byte64& lb, const Byte64& rb)
	{
		return lb < rb;
	}
};



#endif

