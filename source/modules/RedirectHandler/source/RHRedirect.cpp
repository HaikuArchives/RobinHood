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

#include <string.h>
#include <Path.h>
#include <File.h>
#include <stdio.h>
#include "RHModuleInterface.h"
#include "HTTPMessage.h"
#include "FieldList.h"

status_t handle_request( RequestPB *pb )
{
	// We will supply a content length; there is no reason to force a close
	pb->closeConnection = false;
	
	HTTPResponse	response; // The response message
	char			fieldValue[4096]; // Temporary buffer
	bool			temporary = true;
	
	// Check for the type of redirection requested
	if( pb->extras->FindField( "redirect", fieldValue, 4096 ) )
	{
		if( (strcasecmp( "permanent", fieldValue )==0) )
			temporary = false;
	}
	
	// Get the redirect address
	if( !pb->extras->FindField( "href", fieldValue, 4096 ) )
	{
		// If no address, we can't continue
		response.SetHTMLMessage( 501, "Missing redirect address!" ); // Internal Server Error
		pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
		pb->request->SendReply( &response );
		return B_OK;
	}
	
	if( temporary )
		response.SetHTMLMessage( 302 ); // Moved Temporarily
	else
		response.SetHTMLMessage( 301 ); // Moved Permanently
	
	response.AddHeader( kHEAD_LOCATION, fieldValue );
	pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
	pb->request->SendReply( &response );
	return B_OK;
}

// This is called to determine if this handler can handle a request.
// The hander which returns true and has the highest priority will be called.
bool can_handle_resource( const char *mimeType, http_method method, int32 *priority )
{
	// We can process the virtual MIME type of "application/x-vnd.RHV.404"
	if( strcmp(mimeType, "application/x-vnd.RHV.redirect")==0 )
	{
		*priority = 10; // If our priority is 5 or less, the RHFileHandler will be used instead
		return true; // We can handle this request.
	}
	return false; // We can't handle this request.
}
