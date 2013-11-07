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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include "RobinHoodServer.h"
#include "RHLogger.h"
#include "HModuleRoster.h"
#include "StringUtils.h"
#include "VResource.h"

static const char *kDATE_FORMAT = "%a, %d %b %Y %H:%M:%S";
static const char *kDEFAULT_HOST = "Default-Host";

RHServer::RHServer( VHostsList *virtualHosts )
{
	static int32 lastsn = 0;
	sn = atomic_add( &lastsn, 1 );
	
	this->virtualHosts = virtualHosts;
}

RHServer::~RHServer( void )
{
	delete virtualHosts;
}

HTTPHandler *RHServer::NewCopy( void )
{
	HTTPHandler *handler;
	handler = new RHServer( virtualHosts );
	return handler;
}

bool RHServer::MessageReceived( HTTPRequest *request )
{
	if (strlen(request->GetRequestLine()) > 4047)
	{
		log_printf( "%ld ABUSE: Client may be attempting to perform overflow exploit, terminating connection\n", sn );
		HTTPResponse  response;
		response.SetHTMLMessage( 501 ); // Not Implemented
		request->SendReply( &response );
		return true;
	}
	  
	
	// ****
	// Make log entry of request and headers
	// ****
	
	//printf ("request: %s\n", request->GetRequestLine());
	
	log_printf( "%ld Request-Line: %s\n", sn, request->GetRequestLine() );
	
	const char *header;
	for ( int32 i = 0; (header = request->HeaderAt(i)); i++ )
		log_printf( "%ld Header: %s\n", sn, header );

	// ****
	// Setup URI
	// ****
	
	// Setup the broken URI
	brokenURI brURI;
	request->ParseURI( &brURI );
	int32 slength = uri_unescape_str( brURI.path, brURI.path, 4096 );
	
	// ****
	// Set Web Directory based on the "host" name from "virtual hosts" table.
	// ****
	
	VHost			*vhost;
	BPath			webDirectory;
	const char 		*webIndex;
	
	// Get host_name
	// If hostName was not found in URI, set to Host header or default if none
	if( brURI.host[0] == 0 )
	{
		if( !request->FindHeader( kHEAD_HOST, brURI.host, 64 ) )
			strcpy( brURI.host, kDEFAULT_HOST ); // Set to default host
	}
	
	// Find Host in virtual hosts table
	if( (vhost = virtualHosts->FindVHost( brURI.host )) == NULL )
	{
		if( (vhost = virtualHosts->FindVHost( kDEFAULT_HOST )) == NULL )
		{
			// Could not find a default web directory
			HTTPResponse	response;
			response.SetHTMLMessage( 500, "500 No default web directory!" ); // Internal Server Error
			request->SendReply( &response );
			return false;
		}
	}
	webDirectory.SetTo( vhost->GetWebroot() );
	webIndex = ( vhost->GetIndex() );	// this is by default "index.html" if not present in virtual hosts table

	// ****
	// Append index to URI and check for security violations
	// ****
	
	// Append index (default file name) from virtual hosts table if not present in path
	if( ((brURI.path[0] == 0)||(brURI.path[slength-1] == '/'))&&(slength+11<4096) )
		strcat( brURI.path, webIndex );
	// Have they attempted a security violation?
	if( strstr( brURI.path, ".." )||strstr( brURI.path, "./" ) )
	{
		HTTPResponse	response;
		response.SetHTMLMessage( 403 ); // Forbidden
		request->SendReply( &response );
		return true;
	}
	
	// ****
	// Setup PB
	// ****
	
	RequestPB		pb;
	pb.request = request;
	pb.webDirectory = &webDirectory;
	pb.vresources = &vhost->vresources;
	pb.realms = &vhost->realms;
	pb.brURI = &brURI;
	pb.environ = environ;
	pb.authenticate = true;
	pb.sn = sn;
	pb.cookie = NULL;
	
	// ****
	// Call hmodule_roster::HandleRequest() to process the request
	// ****
	
	hmodule_roster->HandleRequest( &pb );
	return !pb.closeConnection;
}

void RHServer::ConnectionOpened( const char *remote_addr )
{
	char			date[256];
	time_t			now;
	tm				*brokentime;
		
	now = time( NULL );
	brTimeLock.Lock();
	brokentime = localtime( &now );
	strftime (date, 256, kDATE_FORMAT, brokentime);
	brTimeLock.Unlock();
	
	log_printf( "%ld Open: %s %s\n", sn, remote_addr, date );
}

void RHServer::ConnectionClosed( status_t status )
{
	char			date[256];
	time_t			now;
	tm				*brokentime;
	
	now = time( NULL );
	brTimeLock.Lock();
	brokentime = localtime( &now );
	strftime (date, 256, kDATE_FORMAT, brokentime);
	brTimeLock.Unlock();
	
	log_printf( "%ld Close: %s\n", sn, date );
}

