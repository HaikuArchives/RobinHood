#ifndef DATA_IO_PUMP_H
#define DATA_IO_PUMP_H

#include "LibHTTPBuild.h"

#include <DataIO.h>
#include <OS.h>
#include <stdlib.h>

class DataIOPump
{
	public:
		DataIOPump( size_t bufferSize = 4096 );
		virtual ~DataIOPump( void );
		
		// Move data from input to output
		// Returns when either EOF is reached, StopPump() is called, or an error occurs.
		status_t StartPump( BDataIO *input, BDataIO *output, ssize_t contentLength = 0 );
		
		// Can be called to stop pump before it reaches EOF
		status_t StopPump( void );
		
		// Returns total bytes moved
		int32 GetTotalBytes( void );
		
		// ReadFunc() gives you a chance to peek at/manipulate the input buffer before it is written to output.
		// if ReadFunc() does not return B_NO_ERROR, the pump is stopped.
		void SetReadCallback( status_t (*ReadFunc)(void *cookie, char *buffer, int32 size), void *cookie );
	
	protected:
		int32			totalBytes;
		size_t			bufferSize;
		
		sem_id			close_sem;
		int32			atomic_open;
		thread_id		pump_thread;
		
		status_t (*ReadFunc)(void *cookie, char *buffer, int32 size);
		void			*cookie;
		bool			running;
};

#endif