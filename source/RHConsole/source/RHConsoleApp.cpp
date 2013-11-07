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

#include <Message.h>
#include <Messenger.h>
#include <Roster.h>
#include "RHConsoleApp.h"
#include "RHCWindow.h"

RHCApp::RHCApp( void )
	: BApplication( "application/x-vnd.KS-RHC" )
{
	
}

RHCApp::~RHCApp( void )
{
	
}

void RHCApp::MessageReceived( BMessage *message )
{
	switch( message->what )
	{
		default:
			BApplication::MessageReceived( message );
			break;
	}
}


bool RHCApp::QuitRequested( void )
{
	return BApplication::QuitRequested();
}

void RHCApp::ReadyToRun( void )
{
	consoleWindow = new RHCWindow( BPoint( 150, 75 ) );
}

