#ifndef BUFFER_IO_H
#define BUFFER_IO_H

#include "LibHTTPBuild.h"
#include <DataIO.h>

class BufferedIO : public BDataIO
{
	public:
		BufferedIO( BDataIO *childIO = NULL, size_t bufferSize = 4096 );
		virtual ~BufferedIO( void );
		status_t DoAllocate( void );
		
		void SetChildIO( BDataIO *childIO );
		
		virtual ssize_t Read(void *buffer, size_t numBytes);
		virtual ssize_t Write(const void *buffer, size_t numBytes);
		int Sync( void );
		
	protected:
		BDataIO			*childIO;
		size_t			bufferSize;
		char			*pbase, *pnext, *pend;
};

#endif