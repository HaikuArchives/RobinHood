// RH Log - A command line console for the Robin Hood Web Server
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

#include <stdio.h>
#include <Message.h>
#include <Messenger.h>
#include <Roster.h>
#include <time.h>
#include <string.h>
#include "RHLogApp.h"
#include "RHMessages.h"
#include "StringUtils.h"
#include "RHMsgFilter.h"

const char *kRHSig = RH_APP_SIG;
static const char *kDATE_FORMAT = "%a, %d %b %Y %H:%M:%S";

RHLogApp::RHLogApp( void )
	: BApplication( "application/x-vnd.KS-RHL" )
{
	filterMask = 0xFFFFFFFF;
}

RHLogApp::~RHLogApp( void )
{
	
}

void RHLogApp::MessageReceived( BMessage *message )
{
	switch( message->what )
	{
		case MSG_LOG:
		{
			const char *s;
			s = message->FindString( kMSG_LOG_STRING );
			if( filter_line_type( filterMask, s ) )
				printf( "%s", s );
			break;
		}
		
		case B_SOME_APP_LAUNCHED:
		{
			const char *s;
			s = message->FindString( "be:signature" );
			
			if( s && (strcmp( s, kRHSig ) == 0) )
			{
				printf( "# [%s] Server Started.\n", GetDateString() );
				ConnectToServer();
			}
			break;
		}
		case B_SOME_APP_QUIT:
		{
			const char *s;
			s = message->FindString( "be:signature" );
			
			if( s && (strcmp( s, kRHSig ) == 0) )
			{
				printf( "# [%s] Server Stopped.\n", GetDateString() );
			}
			break;
		}	
		default:
			BApplication::MessageReceived( message );
			break;
	}
}


bool RHLogApp::QuitRequested( void )
{
	printf( "# [%s] Log Stopped.\n", GetDateString() );
	return BApplication::QuitRequested();
}

void RHLogApp::ReadyToRun( void )
{
	be_roster->StartWatching( be_app_messenger, B_REQUEST_LAUNCHED | B_REQUEST_QUIT );
	
	if( ConnectToServer() != B_OK )
		printf( "# [%s] Log Started. Waiting for server to start...\n", GetDateString() );
	else
		printf( "# [%s] Log Started. Server is running.\n", GetDateString() );
}

status_t RHLogApp::ConnectToServer( void )
{
	status_t		status;
	
	BMessenger		msgr( kRHSig, -1, &status );
	if( status != B_OK )
		return status;
	BMessage		msg( MSG_ADD_LOG_SERVER );
	msg.AddMessenger( kMSG_LOG_MESSENGER, be_app_messenger );
	msgr.SendMessage( &msg );
	return status;
}

const char *RHLogApp::GetDateString( void )
{
	time_t			now;
	tm				*brokentime;
	
	now = time( NULL );
	brokentime = localtime( &now );
	strftime (date, 256, kDATE_FORMAT, brokentime);
	return date;
}

void RHLogApp::ArgvReceived( int32 argc, char **argv )
{
	if( argc > 1 )
	{
		for( int32 i=1; i < argc-1; i++ )
		{
			if( strcmp( argv[i], "-m" ) == 0 )
			{
				filterMask = 0;
				if( strchr( argv[i+1], 'o' ) )
					filterMask |= M_OPEN;
				if( strchr( argv[i+1], 'c' ) )
					filterMask |= M_CLOSE;
				if( strchr( argv[i+1], 'r' ) )
					filterMask |= M_REQUEST_LINE;
				if( strchr( argv[i+1], 'h' ) )
					filterMask |= M_HEADER;
				if( strchr( argv[i+1], 't' ) )
					filterMask |= M_STATUS_LINE;
				if( strchr( argv[i+1], 's' ) )
					filterMask |= M_SENDING;
				if( strchr( argv[i+1], 'S' ) )
					filterMask |= M_SENT;
				if( strchr( argv[i+1], 'x' ) )
					filterMask |= M_EXEC;
				if( strchr( argv[i+1], 'O' ) )
					filterMask |= M_OTHER;
				break;
			}
		}
	}
}
