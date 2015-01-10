// RH Console - A GUI console for the Robin Hood Web Server
// Copyright (C) 1999-2001
// The Robin Hood Development Team

// This program is free software; you can redistribute it and/or 
// modify it under the terms of the GNU General Public License 
// as published by the Free Software Foundation; either version 2 
// of the License, or (at your option) any later version. 

// This program is distributed in the hope that it will be useful, 
// but WITHOUT ANY WARRANTY; without even the implied warranty of 
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
// GNU General Public License for more details. 

// You should have received a copy of the GNU General Public License 
// along with this program; if not, write to the Free Software 
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <Application.h>
#include <ScrollView.h>
#include <TextView.h>
#include <stdio.h>
#include <time.h>
#include <MenuBar.h>
#include <MenuItem.h>
#include <File.h>
#include <FindDirectory.h>
#include <Directory.h>
#include <Entry.h>
#include <Alert.h>
#include <Path.h>
#include <Roster.h>
#include <StringView.h>
#include <stdarg.h>
#include "RHCWindow.h"
#include "RHMessages.h"
#include "RHMsgFilter.h"
#include "StringUtils.h"

static const float kWindowW = 400;
static const float kWindowH = 350;

const char *kRHSig = RH_APP_SIG;

static const char *kDATE_FORMAT = "%a, %d %b %Y %H:%M:%S";
const char *get_date_string( void );

static const int32 kPrefsVersion = 2;

RHCWindow::RHCWindow( BPoint where, uint32 workspace )
	: BWindow( BRect( where.x, where.y, where.x+kWindowW, where.y+kWindowH ),
		"Robin Hood Web Server - Console", B_TITLED_WINDOW_LOOK, B_NORMAL_WINDOW_FEEL, B_NOT_H_RESIZABLE  )
{
	SetSizeLimits( kWindowW, kWindowW, 50, 1000 );
	
	restart = false;
	openVH = false;
	
	conn = 0;
	highConn = 0;
	
	typeFilter = 0xFFFFFFFF;
	headerFilter = 0xFFFFFFFF;
		
	BPath 			path;
	BFile			prefs;
	
	find_directory( B_USER_SETTINGS_DIRECTORY, &path);
	path.Append( "RHConsolePrefs" );
	if( prefs.SetTo( path.Path(), B_READ_ONLY ) == B_NO_ERROR )
	{
		int32		version;
		uint32		workspace;
		
		prefs.Read( &version, sizeof(version) );
		if( version == kPrefsVersion )
		{
			prefs.Read( &typeFilter, sizeof(typeFilter) );
			prefs.Read( &headerFilter, sizeof(headerFilter) );
			prefs.Read( &workspace, sizeof(workspace) );
			SetWorkspaces( workspace );
		}
	}
	
	
	be_roster->StartWatching( BMessenger( this ), B_REQUEST_LAUNCHED | B_REQUEST_QUIT );
	SetupChildren();
	Show();
	
	WelcomeToRobinHood();		// Welcome message...

	//ConnectToServer();
	if( ConnectToServer() != B_OK )
	{
		rgb_color	rgbColor = {145, 145, 145, 255};
		ConsolePrintfRGB( "# [%s] Log Started. Waiting for server to start...\n\n", rgbColor, get_date_string() );
		serverMenuItem->SetLabel( "Start Server" );
		restartMenuItem->SetEnabled( false );
		killMenuItem->SetEnabled( false );
		SetServerStatus( false );
	}
	else
	{
		rgb_color	rgbColor = {145, 145, 145, 255};
		ConsolePrintfRGB( "# [%s] Log Started. Server is running.\n\n", rgbColor, get_date_string() );
		serverMenuItem->SetLabel( "Stop Server" );
		restartMenuItem->SetEnabled( true );
		killMenuItem->SetEnabled( true );
		SetServerStatus( true );
	}
	SetConnections( 0, 0 );
	
	Show();
}

RHCWindow::~RHCWindow( void )
{
	
}

void RHCWindow::MessageReceived( BMessage *message )
{
	switch( message->what )
	{
		case MSG_ABOUT_RH:
		{
			// "About Robin Hood" alert window
			const char * url = NULL;
			switch((new BAlert(
				"About Robin Hood",
				"Robin Hood Web Server for BeOS\n"
				"version 1.2 (August 20th, 2001)\n\n"
				"Copyright © 1999-2001\n"
				"The Robin Hood Development Team\n\n"
				"Joe Kloss (joek@be.com)\n"
				"Karl-Henrik Henriksson (qwilk@desktopian.org)\n"
				"Daniel Fischer (dan@f3c.com)\n"
				"Wade Majors (wade@ezri.org)\n\n"
				"This program is free software; you can redistribute "
				"it and/or modify it under the terms of the GNU "
				"General Public License as published by the Free "
				"Software Foundation; either version 2 of the License, "
				"or (at your option) any later version.\n\n"
				"This program is distributed in the hope that it will "
				"be useful, but WITHOUT ANY WARRANTY; without even "
				"the implied warranty of MERCHANTABILITY or FITNESS "
				"FOR A PARTICULAR PURPOSE. See the GNU General Public "
				"License for more details.\n\n"
			 	"You should have received a copy of the GNU General "
			 	"Public License along with this program; if not, write "
			 	"to the Free Software Foundation, Inc., 59 Temple Place, "
			 	"Suite 330, Boston, MA 02111-1307, USA.","BeBits Page","OK" ))->Go())
			{
				case 0: url = "http://www.bebits.com/app/2463/";  break;
			}
			if (url) be_roster->Launch("text/html", 1, (char**) &url);
		break;
		}

		case MSG_RH_DOCUMENTATION:
		{
			// Open browser window with documentation index page

			app_info info;
			BEntry entry;
			BPath path; 

			be_app->GetAppInfo( &info );
			entry.SetTo( &info.ref );
			entry.GetParent( &entry );
			entry.GetPath(&path); 
			path.Append( "Robin Hood Docs" );
			const char *url = path.Path();
			be_roster->Launch("text/html", 1, (char**) &url);

		break;
		}

		case MSG_DOWNLOAD_PHP:
		{
			// Open browser window with URL to PHP page at BeBits
			const char * url = "http://www.bebits.com/app/566/";
			be_roster->Launch("text/html", 1, (char**) &url);
		break;
		}

		case MSG_LOG:
		{
			const char *s;
			if( (s = message->FindString( kMSG_LOG_STRING )) )
			{
				int32		type;
				const char	*sPtr;
				
				SetConnections( s );
				
				if( filter_line_type( typeFilter, s, &type, &sPtr ) &&
					((type != M_HEADER)||(filter_header_type( headerFilter, sPtr ))) )
				{
					ConsolePrintf( "%s", s );
				}
			}
			break;
		}
		
		case B_SOME_APP_LAUNCHED:
		{
			const char *s;
			s = message->FindString( "be:signature" );
			
			if( s && (strcmp( s, kRHSig ) == 0) )
			{
				rgb_color	rgbColor = {128, 192, 128, 255};
				ConsolePrintfRGB( "# [%s] Server Started.\n", rgbColor, get_date_string() );
				serverMenuItem->SetLabel( "Stop Server" );
				restartMenuItem->SetEnabled( true );
				killMenuItem->SetEnabled( true );
				ConnectToServer();
				SetServerStatus( true );
				if( openVH )
					PostMessage( MSG_EDIT_VH );
			}
			break;
		}
		case B_SOME_APP_QUIT:
		{
			const char *s;
			s = message->FindString( "be:signature" );
			
			if( s && (strcmp( s, kRHSig ) == 0) )
			{
				SetConnections( 0, highConn );
				conn = 0;
				SetServerStatus( false );
				rgb_color	rgbColor = {224, 128, 128, 255};
				ConsolePrintfRGB( "# [%s] Server Stopped.\n", rgbColor, get_date_string() );
				if( restart )
				{
					be_roster->Launch( kRHSig );
					restart = false;
				}
				else
				{
					serverMenuItem->SetLabel( "Start Server" );
					restartMenuItem->SetEnabled( false );
					killMenuItem->SetEnabled( false );
				}
			}
			break;
		}

		case MSG_RESTART_SERVER:
		{
			status_t		status;
	
			BMessenger		msgr( kRHSig, -1, &status );
			if( status == B_OK )
			{
				restart = true;
				msgr.SendMessage( B_QUIT_REQUESTED );
			}
			break;
		}
		
		case MSG_KILL_SERVER:
		{	
			team_id		team;
			
			if( (team = be_roster->TeamFor( kRHSig )) != B_ERROR )
				kill_team( team );
			break;
		}
		
		case MSG_SERVER_ITEM:
		{	
			status_t		status;
	
			BMessenger		msgr( kRHSig, -1, &status );
			if( status != B_OK )
			{
				app_info info;
				BEntry entry;
				BDirectory dir;
				
				be_app->GetAppInfo( &info );
				entry.SetTo( &info.ref );
				entry.GetParent( &dir );
				entry.SetTo( &dir, "server/rhdaemon" );
				
				// First try to launch by directory/name
				if( entry.Exists() )
				{
					entry_ref ref;
					entry.GetRef( &ref );
					
					be_roster->Launch( &ref );
				}// If the directory/name fails, try to launch by app signature
				else
					be_roster->Launch( kRHSig );
			}
			else
				msgr.SendMessage( B_QUIT_REQUESTED );
			break;
		}
		
		case MSG_CLEAR_CONSOLE:
		{
			SetConnections( conn, 0 );
			highConn = 0;
			logView->Delete( 0, logView->TextLength() );

			BFont		font;
			uint32		fontProperties;
			rgb_color	fontColor = {225, 225, 225, 255};
			logView->GetFontAndColor( &font, &fontProperties );
			logView->SetFontAndColor( &font, B_FONT_ALL, &fontColor );
			break;
		}
		
		case MSG_EDIT_VH:
		{
			if( openVH == true )
			{
				openVH = false;
				snooze( 1000000 ); // Give the daemon time to create the file
			}
			BEntry		vhEntry;
			
			if( vhEntry.SetTo( "/boot/home/config/settings/virtual_hosts" ) == B_OK && vhEntry.Exists() )
			{
				entry_ref	ref;
				vhEntry.GetRef( &ref );
				be_roster->Launch( &ref );
			}
			else
			{
				BAlert		*alert;
				alert = new BAlert( "", "The file \"virtual_hosts\" was not found.\n"
				"Hint: Starting the file server for the first time will create this file for you.",
				"Not Now", "Start Server", NULL, B_WIDTH_AS_USUAL, B_EVEN_SPACING, B_INFO_ALERT );
				if( alert->Go() == 1 )
				{
					be_roster->Launch( kRHSig );
					openVH = true;
				}
			}
			break;
		}
		
		case MSG_TYPE_FILTER:
		{
			BMenuItem	*item;
			
			if( message->FindPointer( "source", (void **)&item ) == B_OK )
			{
				int32		type = get_line_type( item->Label() );
				if( item->IsMarked() )
				{
					item->SetMarked( false );
					typeFilter ^= type;
				}
				else
				{
					item->SetMarked( true );
					typeFilter |= type;
				}
			}
			break;
		}
		case MSG_HEADER_FILTER:
		{
			BMenuItem	*item;
			
			if( message->FindPointer( "source", (void **)&item ) == B_OK )
			{
				int32		type = get_header_type( item->Label() );
				if( item->IsMarked() )
				{
					item->SetMarked( false );
					headerFilter ^= type;
				}
				else
				{
					item->SetMarked( true );
					headerFilter |= type;
				}
			}
			break;
		}
		default:
			BWindow::MessageReceived( message );
			break;
	}
}

bool RHCWindow::QuitRequested( void )
{
	ConsolePrintf( "# [%s] Log Stopped.\n", get_date_string() );
	
	BPath 			path;
	BFile			prefs;
	
	find_directory( B_USER_SETTINGS_DIRECTORY, &path);
	path.Append( "RHConsolePrefs" );
	if( prefs.SetTo( path.Path(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE ) == B_NO_ERROR )
	{
		int32		version = kPrefsVersion;
		uint32		workspace = Workspaces();
		prefs.Write( &version, sizeof(version) );
		prefs.Write( &typeFilter, sizeof(typeFilter) );
		prefs.Write( &headerFilter, sizeof(headerFilter) );
		prefs.Write( &workspace, sizeof(workspace) ); //void SetWorkspaces(uint32 workspaces
	}
	
	be_app->PostMessage( B_QUIT_REQUESTED );
	return true;
}

void RHCWindow::SetupChildren( void )
{
	BView		*mainView = new BView( Bounds(), "main view", B_FOLLOW_ALL, B_WILL_DRAW );
	mainView->SetViewColor( 195, 195, 195 );
	AddChild( mainView );
	
	BRect		frame( 5, 50, kWindowW-19, kWindowH-5 );
	BRect		textRect( 0, 0, kWindowW-25, 0 );
	logView = new BTextView( frame, "", textRect, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS );
	BScrollView *scrollView = new BScrollView( "", logView, 
		B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS, false, true, B_FANCY_BORDER );
	logView->MakeEditable( false );
	logView->SetViewColor( 95, 95, 95 );
	logView->SetStylable( true );
	BFont		font;
	uint32		fontProperties;
	rgb_color	fontColor = {225, 225, 225, 255};
	logView->GetFontAndColor( &font, &fontProperties );
	logView->SetFontAndColor( &font, B_FONT_ALL, &fontColor );

	mainView->AddChild( scrollView );
	
	serverStatusV = new BStringView( BRect( 5, 25, 150, 35 ), "", "" );
	mainView->AddChild( serverStatusV );
	connectionsV = new BStringView( BRect( 5, 37, 150, 47 ), "", "" );
	mainView->AddChild( connectionsV );

	
	BMenuBar	*menuBar;
	BMenu		*menu, *subMenu;
	BMenuItem	*item;
	
	menuBar = new BMenuBar( BRect( 0, 0, kWindowW, 5 ), "" );
	
	// File Menu
	menu = new BMenu( "File" );
	menu->AddItem( new BMenuItem( "Edit Virtual Hosts...", new BMessage( MSG_EDIT_VH ) ) );
	menu->AddItem( new BSeparatorItem() );
	menu->AddItem( new BMenuItem( "Quit", new BMessage( B_QUIT_REQUESTED ) ) );
	menuBar->AddItem( menu );
	
	// Controls Menu
	menu = new BMenu( "Controls" );
	menu->AddItem( serverMenuItem = new BMenuItem( "server", new BMessage( MSG_SERVER_ITEM ) ) );
	menu->AddItem( restartMenuItem = new BMenuItem( "Restart Server", new BMessage( MSG_RESTART_SERVER ) ) );
	menu->AddItem( killMenuItem = new BMenuItem( "Kill Server", new BMessage( MSG_KILL_SERVER ) ) );
	menu->AddItem( new BSeparatorItem() );
	menu->AddItem( new BMenuItem( "Clear Console Log", new BMessage( MSG_CLEAR_CONSOLE ) ) );
	menuBar->AddItem( menu );
	
	// Filter Menu
	menu = new BMenu( "Filter" );
	menu->AddItem( new BMenuItem( "Open", new BMessage( MSG_TYPE_FILTER ) ) );
	menu->AddItem( new BMenuItem( "Close", new BMessage( MSG_TYPE_FILTER ) ) );
	menu->AddItem( new BMenuItem( "Request-Line", new BMessage( MSG_TYPE_FILTER ) ) );
	menu->AddItem( subMenu = new BMenu( "Header" ) );
		subMenu->AddItem( new BMenuItem( "User-Agent", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Accept", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Host", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Authorization", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Accept-Language", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Accept-Encoding", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Range", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Via", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Referer", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Connection", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Extension", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "From", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "UA-*", new BMessage( MSG_HEADER_FILTER ) ) );
		subMenu->AddItem( new BMenuItem( "Other", new BMessage( MSG_HEADER_FILTER ) ) );
		for( int32 i = subMenu->CountItems()-1; i >= 0; i-- )
		{
			if( (item = subMenu->ItemAt( i ))&&(headerFilter & get_header_type( item->Label() )) )
				item->SetMarked( true );
		}
		
	menu->AddItem( new BMenuItem( "Status-Line", new BMessage( MSG_TYPE_FILTER ) ) );
	menu->AddItem( new BMenuItem( "Sending", new BMessage( MSG_TYPE_FILTER ) ) );
	menu->AddItem( new BMenuItem( "Sent", new BMessage( MSG_TYPE_FILTER ) ) );
	menu->AddItem( new BMenuItem( "Exec", new BMessage( MSG_TYPE_FILTER ) ) );
	menu->AddItem( new BMenuItem( "Other", new BMessage( MSG_TYPE_FILTER ) ) );

	int32	type;
	for( int32 i = menu->CountItems()-1; i >= 0; i-- )
	{
		if( (item = menu->ItemAt( i ))&&((type = get_line_type( item->Label() )) != M_HEADER )&&(type & typeFilter) )
			item->SetMarked( true );
	}
	
	menuBar->AddItem( menu );

	// Help Menu
	menu = new BMenu( "Help" );
	menu->AddItem( new BMenuItem( "Documentation...", new BMessage( MSG_RH_DOCUMENTATION ) ) );
	menu->AddItem( new BMenuItem( "Download PHP...", new BMessage( MSG_DOWNLOAD_PHP ) ) );
	menu->AddItem( new BSeparatorItem() );
	menu->AddItem( new BMenuItem( "About Robin Hood...", new BMessage( MSG_ABOUT_RH ) ) );
	menuBar->AddItem( menu );
	
	mainView->AddChild( menuBar );
}

status_t RHCWindow::ConnectToServer( void )
{
	status_t		status;
	
	BMessenger		msgr( kRHSig, -1, &status );
	if( status != B_OK )
		return status;
	BMessage		msg( MSG_ADD_LOG_SERVER );
	msg.AddMessenger( kMSG_LOG_MESSENGER, BMessenger( this ) );
	msgr.SendMessage( &msg );
	return status;
}

void RHCWindow::SetServerStatus( bool running )
{
	Lock();
	if( running )
		serverStatusV->SetText( "Status: Running" );
	else
		serverStatusV->SetText( "Status: Stopped" );
	Unlock();
}

void RHCWindow::SetConnections( int32 current, int32 high )
{
	char	s[256];
	sprintf( s, "Connections: %ld High: %ld", current, high );
	Lock();
	connectionsV->SetText( s );
	Unlock();
}

void RHCWindow::SetConnections( const char *logEntry )
{
	int32 type;
	
	if( filter_line_type( M_OPEN | M_CLOSE, logEntry, &type ) )
	{
		if( type == M_OPEN )
			conn++;
		else
			conn--;
		if( conn < 0 )
			conn = 0;
		if( conn > highConn )
			highConn = conn;
		SetConnections( conn, highConn );
	}
}

int RHCWindow::ConsolePrintf( const char *format, ... )
{
	char		s[4096];
	int 		n;
	va_list 	argList;
	va_start( argList, format );
	n = vsprintf( s, format, argList );
	va_end( argList );
	Lock();
	logView->Insert( logView->TextLength(), s, n );
	logView->ScrollToOffset( logView->TextLength() );
	Unlock();
	return n;
}

int RHCWindow::ConsolePrintfRGB( const char *format, rgb_color rgbColor, ... )
{
	char		s[4096];
	int 		n;
	va_list 	argList;
	va_start( argList, format );
	n = vsprintf( s, format, argList );
	va_end( argList );
	Lock();

	BFont		font;
	uint32		fontProperties;
	rgb_color	standardColor = {225, 225, 225, 255};
	logView->GetFontAndColor( &font, &fontProperties );
	logView->SetFontAndColor( &font, B_FONT_ALL, &rgbColor );
	logView->Insert( logView->TextLength(), s, n );
	logView->ScrollToOffset( logView->TextLength() );
	logView->SetFontAndColor( &font, B_FONT_ALL, &standardColor );

	Unlock();
	return n;
}

void RHCWindow::WelcomeToRobinHood()
{
	Lock();

	static const char	*s1 = "Welcome to the Robin Hood Web Server for ";
	static const char	*s2 = "Be";
	static const char	*s3 = "OS\n";

	BFont		font;
	uint32		fontProperties;
	rgb_color	standardColor = {225, 225, 225, 255};
	rgb_color	rgbColor = {195, 195, 195, 255};
	rgb_color	rgbColor2 = {95, 175, 255, 255};
	rgb_color	rgbColor3 = {255, 135, 135, 255};

	logView->GetFontAndColor( &font, &fontProperties );
	logView->SetFontAndColor( &font, B_FONT_ALL, &rgbColor );
	logView->Insert( logView->TextLength(), s1, 41 );
	logView->ScrollToOffset( logView->TextLength() );
	logView->SetFontAndColor( &font, B_FONT_ALL, &rgbColor2 );
	logView->Insert( logView->TextLength(), s2, 2 );
	logView->ScrollToOffset( logView->TextLength() );
	logView->SetFontAndColor( &font, B_FONT_ALL, &rgbColor3 );
	logView->Insert( logView->TextLength(), s3, 3 );
	logView->ScrollToOffset( logView->TextLength() );
	logView->SetFontAndColor( &font, B_FONT_ALL, &standardColor );

	Unlock();
}

const char *get_date_string( void )
{
	static char 	date[256];
	time_t			now;
	tm				*brokentime;
	
	now = time( NULL );
	brokentime = localtime( &now );
	strftime (date, 256, kDATE_FORMAT, brokentime);
	return date;
}
