#include <tier0/dbg.h>

//-----------------------------------------------------------------------------
// Murmur hash
//-----------------------------------------------------------------------------
uint32 MurmurHash2( const void * key, int len, uint32 seed )
{
	// 'm' and 'r' are mixing constants generated offline.
	// They're not really 'magic', they just happen to work well.

	const uint32 m = 0x5bd1e995;
	const int r = 24;

	// Initialize the hash to a 'random' value

	uint32 h = seed ^ len;

	// Mix 4 bytes at a time into the hash

	const unsigned char * data = (const unsigned char *)key;

	while(len >= 4)
	{
		uint32 k = LittleDWord( *(uint32 *)data );

		k *= m; 
		k ^= k >> r; 
		k *= m; 

		h *= m; 
		h ^= k;

		data += 4;
		len -= 4;
	}

	// Handle the last few bytes of the input array

	switch(len)
	{
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
	};

	// Do a few final mixes of the hash to ensure the last few
	// bytes are well-incorporated.

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}

#define TOLOWERU( c ) ( ( uint32 ) ( ( ( c >= 'A' ) && ( c <= 'Z' ) )? c + 32 : c ) )
uint32 MurmurHash2LowerCase( char const *pString, uint32 nSeed )
{
	int nLen = V_strlen( pString );
	char *p = ( char * ) stackalloc( nLen + 1 );
	for( int i = 0; i < nLen ; i++ )
	{
		p[i] = TOLOWERU( pString[i] );
	}
	return MurmurHash2( p, nLen, nSeed );
}
