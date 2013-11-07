#ifndef RHAPPLICATION_H
#define RHAPPLICATION_H

#include <Application.h>
#include <List.h>
#include "VHosts.h"
class RHServer;
struct ServerInfo;

class RHApp : public BApplication
{
	public:
		RHApp( void );
		virtual ~RHApp( void );
		
		virtual void MessageReceived( BMessage *message );
		virtual bool QuitRequested( void );
		virtual void ReadyToRun( void );
	
	protected:
		void StartServers( void );
		status_t StartServer( uint16 port, VHostsList *virtualHosts, int32 maxCon = 32 );
		status_t StopServer( int32 index, bool now=false );
		status_t StopAllServers( void );
		int32 CountServers( void );
		ServerInfo *ServerAt( int32 index );
	
	protected:
		BList		serverList;
		BList		logMsgrList;
};

#endif