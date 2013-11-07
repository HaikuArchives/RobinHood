// Robin Hood Web Server for BeOS
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
#include <Entry.h>
#include <Roster.h>
#include <File.h>
#include <Path.h>
#include <FindDirectory.h>
#include <stdlib.h>
#include <string.h>
#include "RobinHoodServer.h"
#include "TCP_IO.h"
#include "StringUtils.h"
#include "DataIOUtils.h"
#include "HTTPListener.h"
#include "RHApplication.h"
#include "RHMessages.h"
#include "HModuleRoster.h"

struct ServerInfo
{
	HTTPListener 	*listener;
	RHServer 		*handler;
	VHostsList 		*virtualHosts;
	thread_id		thread;
};

static const char *kDefaultHostFile = 
"# Robin Hood Web Server Virtual Hosts\n"
"# Changes to this file will take effect when the server is restarted.\n"
"# Format:\n"
"# 1*(command ':' 1*field)\";\"\n"
"# command = 'Server' | 'Host' | 'VRes' | 'Realm'\n"
"# field = field-name \"=\" field-value\n"
"# field-name = 1*31( ALPHA | DIGIT | '-' | '_' )\n"
"# field-value = ['\"'] TEXT ['\"']\n"
"# Examples:\n"
"# Server: port=80;\n"
"# Host: host=\"Default-Host\" webroot=\"/boot/home/public_html/\" index=\"main.html\";\n"
"# Host: host=\"www.foo-bar.org\" webroot=\"/boot/home/foobar/\" index=\"index.php\";\n"
"# VRes: pattern=\"*.shtml\" type=\"text/x-server-parsed-html\" real=true;\n"
"# VRes: pattern=\"*index.html\" type=\"application/x-vnd.Be-directory\" real=false;\n"
"# VRes: pattern=\"foo-bar/*\" type=\"application/x-vnd.RHV.redirect\" real=false redirect=temporary href=\"http://www.foo-bar.org/foobar.html\";\n"
"# Realm: name=\"Admin CGI\" pattern=\"cgi-bin/*\" pattern=\"*.cgi\" user=\"Pete\" passwd=\"Thornton\";\n"
"# Realm: name=\"RDA\" pattern=\"*\" user=\"MacGyver\" passwd=\"Jack Dalton\";\n"
"\n"
"Server: port=80;\n"
"\tHost: host=\"Default-Host\" webroot=\"/boot/home/public_html/\" index=\"index.html\";\n"
"\t\tVRes: pattern=\"*.shtml\" type=\"text/x-server-parsed-html\" real=true;\n"
"\t\tVRes: pattern=\"*.php\" type=\"application/x-httpd-php\" real=true;\n"
"\t\tVRes: pattern=\"cgi-bin/*/*\" type=\"application/x-vnd.RHV.File\" real=true;\n"
"\t\tVRes: pattern=\"*.cgi\" pattern=\"cgi-bin/*\" type=\"application/x-vnd.CGI\" real=true;\n"
"#\t\tVRes: pattern=\"*index.html\" pattern=\"index\" type=\"application/x-vnd.Be-directory\" real=false;\n"
"#\t\tVRes: pattern=\"VBeIcons/*\" type=\"image/VBeIcon\" real=false;\n"
"\t\tVRes: pattern=\"*\" type=\"application/x-vnd.RHV.404\" real=false;\n";

RHApp::RHApp( void )
	: BApplication( "application/x-vnd.KS-RH" )
{
	
}

RHApp::~RHApp( void )
{
	
}

void RHApp::MessageReceived( BMessage *message )
{
	switch( message->what )
	{
		case MSG_LOG: // Forward message to all log servers
			{
				BMessenger		*msgr;
				
				
				for( int32 i=0; (msgr=(BMessenger *)logMsgrList.ItemAt(i)); i++ )
				{
					if( msgr->IsValid() )
						msgr->SendMessage( message );
					else // Remove from list if not valid
					{
						logMsgrList.RemoveItem( i );
						i--;
					}
				}
			}
			break;
			
		case MSG_ADD_LOG_SERVER:
			{
				BMessenger		*msgr = new BMessenger;
				if( message->FindMessenger( kMSG_LOG_MESSENGER, msgr ) != B_NO_ERROR )
					delete msgr;
				else
					logMsgrList.AddItem( msgr );
			}
			break;
			
		default:
			BApplication::MessageReceived( message );
			break;
	}
}


bool RHApp::QuitRequested( void )
{
	StopAllServers();
	return BApplication::QuitRequested();
}

void RHApp::ReadyToRun( void )
{
	app_info	info; // info.ref
	BEntry		entry;
	BPath		moduleDir;
	GetAppInfo( &info );
	
	entry.SetTo( &info.ref );
	entry.GetParent( &entry );
	entry.GetPath( &moduleDir );
	moduleDir.Append( "robin_hood_modules/" );
	entry.SetTo( moduleDir.Path() );
	
	if( !entry.Exists() )
	{
		find_directory( B_USER_ADDONS_DIRECTORY, &moduleDir );
		moduleDir.Append( "robin_hood_modules/" );
	}
	
	hmodule_roster = new HModuleRoster;
	hmodule_roster->LoadModules( &moduleDir );
	AddHandler( hmodule_roster );
	hmodule_roster->StartWatching( &moduleDir );
	StartServers();
}

status_t RHApp::StartServer( uint16 port, VHostsList *virtualHosts, int32 maxCon )
{
	ServerInfo 		*info = new ServerInfo();
	info->virtualHosts = virtualHosts;
	info->handler = new RHServer( info->virtualHosts );
	info->listener = new HTTPListener( info->handler, port, maxCon );
	if( (info->thread = info->listener->Run()) < 0 )
	{
		delete info->listener;
		delete info->handler;
		delete info;
		return B_ERROR;
	}
	else
	{
		serverList.AddItem( info );
		return B_OK;
	}
}

status_t RHApp::StopServer( int32 index, bool now )
{
	ServerInfo 		*info;
	
	if( !(info = ServerAt( index )) )
		return B_ERROR;
	info->listener->Quit();
	delete info->listener;
	delete info->handler;
	delete info;
	serverList.RemoveItem( info );
	return B_OK;
}

status_t RHApp::StopAllServers( void )
{
	while( StopServer( 0 ) == B_OK ) {  }
	return B_OK;
}

int32 RHApp::CountServers( void )
{
	return serverList.CountItems();
}

ServerInfo *RHApp::ServerAt( int32 index )
{
	return (ServerInfo *)serverList.ItemAt( index );
}

void RHApp::StartServers( void )
{
	BPath				path;
	VHostsList			*virtualHosts;
	VHost				*vhost = NULL;
	
	// ******
	// Setup servers from virtual hosts file
	// ******
	if( find_directory( B_USER_SETTINGS_DIRECTORY, &path) != B_NO_ERROR )
		return;
	else
	{
		BFile vHostsFile;
		path.Append( "virtual_hosts" );
		
		if( vHostsFile.SetTo( path.Path(), B_READ_ONLY ) != B_NO_ERROR )
		{
			// ******
			// Create default virtual host file if it does not exist
			// ******
			if( vHostsFile.SetTo( path.Path(), B_READ_WRITE | B_CREATE_FILE ) == B_NO_ERROR )
			{
				io_printf( &vHostsFile, "%s", kDefaultHostFile );
				vHostsFile.Seek( 0, SEEK_SET );
			}
			else
				return;
		}
		virtualHosts = new VHostsList();
		
		char		line[4096];
		char		lineType[32];
		char		lineText[1024];
		
		char		fieldName[32];
		char		fieldValue[4096];
		
		int32		servers = 0;
		uint16		port = 80;
		int32		maxConn = 32;
		const char	*sPtr;
		
		// ******
		// Parse Virtual Hosts file
		// ******
		while( io_getline( &vHostsFile, line, 4096 ) )
		{
			// If not a comment or an empty line, interpret the line
			if( (*line != '#')&&(*line != 0) )
			{
				// break line into lineType and lineText
				sPtr = line;
				sPtr = get_next_token( lineType, sPtr, 32, ":" );
				if( *sPtr == 0 )
					continue;
				sPtr = get_next_token( lineText, sPtr+1, 1024, ";", "\"" );
				
				// ******
				// Server
				// ******
				
				if( strcasecmp( "Server", lineType ) == 0 )
				{
					// Start the previous server if there is one
					if( servers++ > 0 )
					{
						StartServer( port, virtualHosts, maxConn );
						virtualHosts = new VHostsList();
						vhost = NULL;
					}
					
					port = 80;
					maxConn = 32;
					sPtr = lineText;
					
					while( (sPtr = get_next_field( sPtr, fieldName, fieldValue )) )
					{
						if( strcasecmp( "port", fieldName ) == 0 )
							port = strtol ( fieldValue, NULL, 10 );
						else if( strcasecmp( "maxcon", fieldName ) == 0 )
							maxConn = strtol ( fieldValue, NULL, 10 );
					}
				}
				// ******
				// VHost
				// ******
				else if( strcasecmp( "Host", lineType ) == 0 )
				{
					vhost = new VHost();
					sPtr = lineText;
					
					while( (sPtr = get_next_field( sPtr, fieldName, fieldValue )) )
					{
						vhost->SetIndex( "index.html" );	//default index file name (used if not set in Virtual Hosts)
						if( strcasecmp( "host", fieldName ) == 0 )
							vhost->SetHost( fieldValue );
						else if( strcasecmp( "webroot", fieldName ) == 0 )
							vhost->SetWebroot( fieldValue );
						else if( strcasecmp( "index", fieldName ) == 0 )
							vhost->SetIndex( fieldValue );
					}
					virtualHosts->AddVHost( vhost );
				}
				// ******
				// VRes
				// ******
				else if( vhost && (strcasecmp( "VRes", lineType ) == 0) )
				{
					VResource		*vres;
					
					vres = new VResource();
					sPtr = lineText;
					
					while( (sPtr = get_next_field( sPtr, fieldName, fieldValue )) )
					{
						if( strcasecmp( "pattern", fieldName ) == 0 )
							vres->AddPattern( fieldValue );
						else if( strcasecmp( "type", fieldName ) == 0 )
							vres->SetMimeType( fieldValue );
						else if( strcasecmp( "real", fieldName ) == 0 )
						{
							if( strcasecmp( "true", fieldValue ) == 0 )
								vres->SetReal( true );
							else
								vres->SetReal( false );
						}
						else
							vres->AddExtra( fieldName, fieldValue );
					}
					vhost->vresources.AddVRes( vres );
				}
				// ******
				// Realm
				// ******
				else if( vhost && (strcasecmp( "Realm", lineType ) == 0) )
				{
					HTTPRealm		*realm;
					
					realm = new HTTPRealm();
					sPtr = lineText;
					
					while( (sPtr = get_next_field( sPtr, fieldName, fieldValue )) )
					{
						if( strcasecmp( "name", fieldName ) == 0 )
							realm->SetName( fieldValue );
						else if( strcasecmp( "pattern", fieldName ) == 0 )
							realm->AddPattern( fieldValue );
						else if( strcasecmp( "user", fieldName ) == 0 )
							realm->SetUser( fieldValue );
						else if( strcasecmp( "passwd", fieldName ) == 0 )
							realm->SetPasswd( fieldValue );
					}
					vhost->realms.AddRealm( realm );
				}
			} // End if( (*hostLine != '#')&&(*hostLine != '\n')&&(*hostLine != '\r') )
		} // End while( io_getline( &vHostsFile, line, 4096 ) )
		
		// Start the last server
		StartServer( port, virtualHosts, maxConn );
	} // End else
	
	// Print Server Config for debugging purposes
	/*
	ServerInfo		*info;
	VResource		*vres;
	const char		*extra, *pattern;
	HTTPRealm		*realm;
				
	for( int32 a=0; (info=(ServerInfo *)serverList.ItemAt(a)); a++ )
	{
		printf( "Server %ld\n", a );
		for( int32 b=0; (vhost=(VHost *)info->virtualHosts->ItemAt(b)); b++ )
		{
			printf( "VHost: %s @ %s\n", vhost->GetHost(), vhost->GetWebroot() );
			for( int32 c=0; (vres=(VResource *)vhost->vresources.ItemAt(c)); c++ )
			{
				printf( "VRes: %s, auth=%d real=%d\n", vres->GetMimeType(), vres->Authenticate(), vres->GetReal() );
				for( int32 d=0; (pattern=(const char *)vres->ItemAt(d)); d++ )
					printf( "VPattern: %s\n", pattern );
				for( int32 d=0; (extra=(const char *)vres->extras.ItemAt(d)); d++ )
					printf( "Extra: %s\n", extra );
			}
			for( int32 c=0; (realm=(HTTPRealm *)vhost->realms.ItemAt(c)); c++ )
			{
				printf( "Realm: %s, user=%s, pass=%s\n", realm->GetName(), realm->GetUser(), realm->GetPasswd() );
				for( int32 d=0; (pattern=(const char *)realm->ItemAt(d)); d++ )
					printf( "RPattern: %s\n", pattern );
			}
		}
	}
	*/
}

