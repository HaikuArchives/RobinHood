#ifndef RHAPPLICATION_H
#define RHAPPLICATION_H

#include <Application.h>
#include <List.h>
class FieldList;
class RHServer;

struct ServerInfo;
class RHCWindow;

class RHCApp : public BApplication
{
	public:
		RHCApp( void );
		virtual ~RHCApp( void );
		
		virtual void MessageReceived( BMessage *message );
		virtual bool QuitRequested( void );
		virtual void ReadyToRun( void );
	
	protected:
		RHCWindow		*consoleWindow;
};

#endif