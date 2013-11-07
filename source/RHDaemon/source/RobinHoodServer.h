#ifndef ROBIN_HOOD_SERVER_H
#define ROBIN_HOOD_SERVER_H

#include "HTTPHandler.h"
#include "HTTPMessage.h"

#include "VHosts.h"

class RHServer : public HTTPHandler
{
	public:
		RHServer( VHostsList *virtualHosts );
		virtual ~RHServer( void );
		
		virtual HTTPHandler *NewCopy( void );
		
		virtual bool MessageReceived( HTTPRequest *request );
		
	protected:
		virtual void ConnectionOpened( const char *remote_addr );
		virtual void ConnectionClosed( status_t status );
	
	protected:
		VHostsList 			*virtualHosts;
		int32				sn; // Serial Number
		bigtime_t			startTime;
		int32 				contentLength;
};

class VHostList : public FieldList
{
	
};

#endif
