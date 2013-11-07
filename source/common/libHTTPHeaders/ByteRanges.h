#ifndef BYTE_RANGES_H
#define BYTE_RANGES_H

#include "LibHTTPBuild.h"
#include <List.h>

struct ByteRangeSpec
{
	int32		first;
	int32		last;
};

struct FileOffsetSpec
{
	int32		offset;
	int32		size;
};

class ByteRangeSet
{
	public:
		ByteRangeSet( void );
		ByteRangeSet( const char *setString );
		virtual ~ByteRangeSet( void );
		
		void AddByteRange( ByteRangeSpec *range );
		void AddByteRange( int32 first, int32 last );
		void AddByteRange( const char *setString );
		
		char *RangeString( char *buffer );
		char *ContentRangeString( char *buffer, int32 rangeIndex, int32 entityLength );
		int32 ContentLength( int32 entityLength );
		int32 CountRanges( void );
		ByteRangeSpec *GetRange( int32 rangeIndex );
		
		ByteRangeSpec *GetFileOffset( FileOffsetSpec *offset, int32 entityLength, int32 rangeIndex );
		
		void MakeEmpty();
	
	protected:
		BList		rangeSet;
};

#endif