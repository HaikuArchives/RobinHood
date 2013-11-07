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
#include "RequestPB.h"
#include "HTTPMessage.h"
#include "HTTPUtils.h"
#include "StringUtils.h"
#include "DataIOUtils.h"
#include "FieldList.h"

//****
// All Robin Hood add-on modules must implement the two functions 
// declared in "RHModuleInterface.h":
// 		extern "C" __declspec(dllexport) status_t handle_request( RequestPB *pb );
// 		extern "C" __declspec(dllexport) bool can_handle_resource( const char *mimeType, http_method method, int32 *priority );
//****

// Handle a request
status_t handle_request( RequestPB *pb )
{
	// We will supply a content length; there is no reason to force a close
	pb->closeConnection = false;
	
	HTTPResponse	response; // The response message
	BMallocIO		body; // The message body
	char			headBuffer[1024]; // Temporary buffer for creating http messsage headers
	
	// ***
	// Setup Message Body
	// ***
	
	BFile		file;
	BPath		filePath;
	char		fileNameBuf[256];
	const char	*fileName;
	
	if( pb->extras->FindField( "src", fileNameBuf, 256) )
		fileName = fileNameBuf;
	else
		fileName = "404.html";
	
	filePath.SetTo( pb->webDirectory->Path(), fileName );
	
	if( file.SetTo( filePath.Path(), B_READ_ONLY ) == B_OK )
	{
		char		lineBuffer[4096];
		char		command[64];
		const char 	*linePtr, *sPtr, *ePtr;
		
		// Read lines from 404file until EOF is reached.
		while( io_getline( &file, lineBuffer, 4096 ) )
		{
			sPtr = linePtr = lineBuffer;
			
			// Process each command in this line
			while( (sPtr = strstr( sPtr, "<!--#" ) ) )
			{
				// If any text before command, copy to body
				if( sPtr != linePtr )
				{
					*((char *)sPtr) = 0;
					io_printf( &body, "%s", linePtr );
				}
				
				sPtr += 5; // Move to command
				// Find end of command
				if( (ePtr = strstr( sPtr, "-->" )) )
					*((char *)ePtr) = 0; // Terminate string
				
				// Get the command	
				sPtr = get_next_token( command, sPtr, 64 );
				
				if( strcasecmp( command, "path" ) == 0 )
					io_printf( &body, "%s", pb->brURI->path );
				if( strcasecmp( command, "leaf" ) == 0 )
				{
					BPath		path( "/" );
					path.Append( pb->brURI->path );
					io_printf( &body, "%s", path.Leaf() );
				}
				else if( strcasecmp( command, "host" ) == 0 )
					io_printf( &body, "%s", pb->brURI->host );
				else if( strcasecmp( command, "referer" ) == 0 )
				{
					char	referer[4096];
					if( pb->request->FindHeader( "Referer", referer, 4096 ) )
						io_printf( &body, "%s", referer );
					else
						io_printf( &body, "Unspecified Source" );
				}
				else if( strcasecmp( command, "referer-host" ) == 0 )
				{
					char	referer[4096];
					if( pb->request->FindHeader( "Referer", referer, 4096 ) )
					{
						brokenURI		brURI;
						parse_URI( referer, &brURI );	
						io_printf( &body, "%s", brURI.host );
					}
					else
						io_printf( &body, "Unspecified Host" );
				}
				// Advance to end of command
				if( ePtr && (ePtr[3] != 0) )
					sPtr = linePtr = ePtr+3;
				else
				{
					linePtr = sPtr;
					break;
				}
			}
			// Copy to body
			if( *linePtr )
				io_printf( &body, "%s\n", linePtr );
		}
	}
	else // Send default 404 Message
	{
		response.SetHTMLMessage( 404 ); // Not Found
		pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
		pb->request->SendReply( &response );
		return B_OK;
	}
	
	// ***
	// End of Setup Message Body
	// ***
	
	// Set status code
	response.SetStatusLine( 404 );
	
	// Set content type
	response.AddHeader( kHEAD_TYPE, "text/html" );
	
	// Set content length
	response.SetContentLength( body.BufferLength() );
	sprintf( headBuffer, "%s: %ld", kHEAD_LENGTH, int32(response.GetContentLength()) );
	response.AddHeader( headBuffer );
		
	// Only send the body when the method is "GET"
	// "HEAD" requests must NOT contain a body...
	if( pb->request->GetMethod() == METHOD_GET )
		response.SetMessageBody( &body );
	
	// Seek to start of message body
	body.Seek( 0, SEEK_SET );
	
	// Write our status line to the log
	pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
	
	// Send the response message
	pb->request->SendReply( &response );
	
	// return B_OK if a response was sent
	// return B_ERROR if a response could not be sent for some reason
	return B_OK;
}

// This is called to determine if this handler can handle a request.
// The hander which returns true and has the highest priority will be called.
bool can_handle_resource( const char *mimeType, http_method method, int32 *priority )
{
	// We can process the virtual MIME type of "application/x-vnd.RHV.404"
	// 		and we will do so only for 
	if( (strcmp(mimeType, "application/x-vnd.RHV.404")==0)&&
	((method == METHOD_GET)||(method == METHOD_HEAD)) )
	{
		*priority = 10; // If our priority is 5 or less, the RHFileHandler will be used instead
		return true; // We can handle this request.
	}
	return false; // We can't handle this request.
}
