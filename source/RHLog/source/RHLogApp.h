#ifndef RHLOG_APP_H
#define RHLOG_APP_H

#include <Application.h>
#include <List.h>
class FieldList;
class RHServer;

struct ServerInfo;
class RHCWindow;

class RHLogApp : public BApplication
{
	public:
		RHLogApp( void );
		virtual ~RHLogApp( void );
		
		virtual void MessageReceived( BMessage *message );
		virtual bool QuitRequested( void );
		virtual void ReadyToRun( void );
		virtual void ArgvReceived( int32 argc, char **argv );
	
	protected:
		status_t ConnectToServer( void );
		const char *GetDateString( void );
	
	protected:
		char			date[256];
		int32			filterMask;
};

#endif