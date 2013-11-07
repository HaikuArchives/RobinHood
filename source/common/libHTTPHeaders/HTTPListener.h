#ifndef HTTP_LISTENER_H
#define HTTP_LISTENER_H

#include "LibHTTPBuild.h"

#include "TCP_IO.h"
#include "HTTPHandler.h"

class HTTPListener : public TCP_Listener
{
	public:
		HTTPListener( HTTPHandler *handler, unsigned short port = 80, int acceptance_count = 32 );
		virtual ~HTTPListener( void );
		
		virtual thread_id Run( void );
		virtual void Quit( void );
	
	protected:
		static int32 accept_loop_entry( void *arg );
		int32 AcceptLoop( void );
		
	protected:
		uint16 			port;
		HTTPHandler 	*handler;
		int32			maxConnections;
		thread_id		accept_thread;
		int32			running;
};

class HTTPConnection
{
	public:
		HTTPConnection( HTTPHandler *handler, int32 socket, uint16 acceptPort );
		virtual ~HTTPConnection( void );
		
		virtual thread_id Run( void );
		
	protected:
		static int32 connection_loop_entry( void *arg );
		int32 ConnectionLoop( void );
			
	protected:
		HTTPHandler 	*handler;
		int32			socket;
		uint16			acceptPort;
};

#endif