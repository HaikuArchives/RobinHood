#ifndef FILE_IO
#define FILE_IO

#include "LibHTTPBuild.h"

#include <DataIO.h>
#include <stdio.h>

class FileIO : public BDataIO
{
	public:
		FileIO( FILE *stream );
		FileIO( FILE *inStream, FILE *outStream );
		
		virtual ssize_t Read( void *buffer, size_t numBytes );
		virtual ssize_t Write( const void *buffer, size_t numBytes );
	
	protected:
		FILE		*inStream, *outStream;
};

class DesIO : public BDataIO
{
	public:
		DesIO( int fileDes );
		DesIO( int inDes, int outDes );
		
		virtual ssize_t Read( void *buffer, size_t numBytes );
		virtual ssize_t Write( const void *buffer, size_t numBytes );
	
	protected:
		int		inDes, outDes;
};

#endif