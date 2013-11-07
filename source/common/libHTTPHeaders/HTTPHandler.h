#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include "LibHTTPBuild.h"

#include "HTTPMessage.h"

class HTTPHandler
{
	public:
		HTTPHandler(void) {  };
		virtual bool MessageReceived( HTTPRequest *request );
		virtual HTTPHandler *NewCopy( void ) { return new HTTPHandler; };
		
		// Listener Hooks
		virtual void ConnectionOpened( const char *remote_addr ) { };
		virtual void ConnectionClosed( status_t status ) { };
};

#endif