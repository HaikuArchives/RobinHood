#ifndef RHC_WINDOW_H
#define RHC_WINDOW_H

#include <Window.h>
#include <Message.h>

class BAlert;
class BTextView;
class BMenuItem;
class BStringView;

class RHCWindow : public BWindow
{
	public:
		RHCWindow( BPoint where, uint32 workspace = B_CURRENT_WORKSPACE );
		virtual ~RHCWindow( void );
		
		virtual void MessageReceived( BMessage *message );
		virtual bool QuitRequested( void );
	
	protected:
		void SetupChildren( void );
		int ConsolePrintf( const char *format, ... );
		int ConsolePrintfRGB( const char *format, rgb_color rgbColor, ... );
		void WelcomeToRobinHood();
		status_t ConnectToServer( void );
		void SetServerStatus( bool running );
		void SetConnections( int32 current, int32 high );
		void SetConnections( const char *logEntry );

	protected:
		BAlert			*about;
		BTextView		*logView;
		BMenuItem		*serverMenuItem;
		BMenuItem		*restartMenuItem;
		BMenuItem		*killMenuItem;
		int32			typeFilter;
		int32			headerFilter;
		bool			restart;
		bool			openVH;
		
		BStringView		*serverStatusV;
		BStringView		*connectionsV;
		
		int32			conn, highConn;
};

enum {
	MSG_SERVER_ITEM = B_SPECIFIERS_END+1,
	MSG_RESTART_SERVER,
	MSG_CLEAR_CONSOLE,
	MSG_EDIT_VH,
	MSG_TYPE_FILTER,
	MSG_HEADER_FILTER,
	MSG_KILL_SERVER,
	MSG_ABOUT_RH,
	MSG_RH_DOCUMENTATION,
	MSG_DOWNLOAD_PHP
};

#endif
