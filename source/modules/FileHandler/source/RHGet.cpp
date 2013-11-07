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
#include <stdio.h>
#include <File.h>
#include <Directory.h>
#include <NodeInfo.h>
#include <stdio.h>
#include <Path.h>
#include "HTTPMessage.h"
#include "parsedate.h"
#include "ByteRanges.h"
#include "HTTPRealm.h"
#include "RequestPB.h"
#include "RHModuleInterface.h"
#include "HTTPUtils.h"
#include "StringUtils.h"

bool can_handle_resource( const char *mimeType, http_method method, int32 *priority )
{
	if( (method == METHOD_GET)||(method == METHOD_HEAD) )
	{
		if( strcmp( "application/x-vnd.RHV.File", mimeType ) == 0 )
			*priority = 15;
		else
			*priority = 5;
		return true;
	}
	else
		return false;
}

status_t handle_request( RequestPB *pb )
{
	pb->closeConnection = false;
	HTTPResponse	response;
	int32			fieldSize = 1024;
	char			fieldValue[4096];
	char			headBuffer[1024];
	const char		*sPtr;
	
	// If it's a directory, redirect
	if( strcmp( pb->mimeType, "application/x-vnd.Be-directory") == 0 )
	{
		response.SetHTMLMessage( 302 ); // Moved Temporarily
		sPtr = fieldValue;
		sPtr = strxcpy( (char *)sPtr, pb->brURI->path, 4095-(sPtr-fieldValue) );
		sPtr = strxcpy( (char *)sPtr, "/", 4095-(sPtr-fieldValue) );
		uri_esc_str( pb->brURI->path, fieldValue, 4096, false, "/" );
		strcpy( pb->brURI->scheme, "http" );
		URI_to_string( pb->brURI, fieldValue, 4096, true );
		response.AddHeader( kHEAD_LOCATION, fieldValue );
		pb->request->SendReply( &response );
		pb->closeConnection = false;
		return B_OK;
	}
	// Temporary buffers
	
	BFile			theFile, gzFile;
	BPath			absPath;
	bool			gz = false;
	
	
	// Is gzip allowed? Then see if a gziped file is available.
	if( pb->request->FindHeader( kHEAD_ACCEPT_ENCODING, fieldValue, fieldSize ) )
	{
		if( strstr( fieldValue, "gzip" )||strstr( fieldValue, "x-gzip" ) )
		{
			char	gzPath[2051];
			strcpy( gzPath, pb->brURI->path );
			strcat( gzPath, ".gz" );
			absPath.SetTo( pb->webDirectory->Path(), gzPath );
			
			// Was gz file found? If not, send normal file.
			if( gzFile.SetTo( absPath.Path(), B_READ_ONLY ) == B_NO_ERROR )
				gz = true;
		}
	}
	
	absPath.SetTo( pb->webDirectory->Path(), pb->brURI->path );
	// Check Authorization
	if( pb->authenticate &&	!pb->realms->Authenticate( pb->request, &response, pb->brURI->path, absPath.Path(), S_IROTH ) )
	{
		pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
		return B_OK;
	}
	
	// Was the file found?
	if( theFile.SetTo( absPath.Path(), B_READ_ONLY ) == B_NO_ERROR )
	{
		int32		statusCode = 200;
		
		time_t		modtime;
		theFile.GetModificationTime( &modtime );
		
		// Conditional Get by modifiaction date Header?
		if( pb->request->FindHeader( kHEAD_IF_MODIFIED, fieldValue, fieldSize ) )
		{
			time_t ifmod = parsedate(fieldValue, -1);
			
			if( (ifmod != -1)&&(ifmod >= modtime) )
			{
				response.SetHTMLMessage( 304 ); // Not Modified
				pb->request->SendReply( &response );
				return B_OK;
			}
		}
		
		// Conditional Get by modifiaction date Header?
		if( pb->request->FindHeader( kHEAD_IF_UNMODIFIED, fieldValue, fieldSize ) )
		{
			time_t ifmod = parsedate(fieldValue, -1);
			if( (ifmod != -1)&&(ifmod <= modtime) )
			{
				response.SetHTMLMessage( 412 ); // Pre-condition failed
				pb->request->SendReply( &response );
				return B_OK;
			}
		}
		
		// Get file size
		int32		entityLength, contentLength;
		off_t size;
		
		if( gz )
			gzFile.GetSize( &size );
		else
			theFile.GetSize( &size );
		
		entityLength= size;
		
		
		// Add Date header
		time_t			now;
		struct tm 		*brokentime;
		
		now = time( NULL );
		brTimeLock.Lock();
		brokentime = gmtime( &now );
		strftime(fieldValue, 256, kHTTP_DATE, brokentime);
		brTimeLock.Unlock();
		
		response.AddHeader( kHEAD_DATE, fieldValue );
		
		// Add last modified header
		brTimeLock.Lock();
		brokentime = gmtime (&modtime);
		strftime (fieldValue, fieldSize, kHTTP_DATE, brokentime);
		brTimeLock.Unlock();
		
		response.AddHeader( kHEAD_LAST_MODIFIED, fieldValue );
		
		// Add server name header
		response.AddHeader( kHEAD_SERVER, "RobinHood" );
		
		// Add Accept-Ranges header
		response.AddHeader( "Accept-Ranges: bytes" );
		
		// If "Range:" header, parse range
		ByteRangeSet	rangeSet;
		
		if( pb->request->FindHeader( kHEAD_RANGE, fieldValue, fieldSize ) )
		{
			rangeSet.AddByteRange( fieldValue );
			statusCode = 206;
		}
		
		// Add content encoding header if gzFile
		if( gz )
			response.AddHeader( kHEAD_ENCODING, "gzip" );
		
		if( statusCode == 206 ) // If partial content
		{
			contentLength = rangeSet.ContentLength( entityLength );
			response.SetContentLength( contentLength );
			if( rangeSet.CountRanges() == 1 )
				response.AddHeader( kHEAD_CONTENT_RANGE, 
					rangeSet.ContentRangeString( headBuffer, 0, entityLength ) );
		}
		else // Set content-length to entity-length
			contentLength = entityLength;
	
	
		// Set content-length
		response.SetContentLength( contentLength );
		
		// Add content length header
		sprintf( headBuffer, "%s: %ld", kHEAD_LENGTH, contentLength );
		response.AddHeader( headBuffer );
		
		// Add mime type header
		response.AddHeader( kHEAD_TYPE, pb->mimeType );
		
		// Set message body
		BFile		*bodyFile;
		
		if( pb->request->GetMethod() == METHOD_GET )
		{
			if( gz )
			{
				response.SetMessageBody( &gzFile );
				bodyFile = &gzFile;
			}
			else
			{
				response.SetMessageBody( &theFile );
				bodyFile = &theFile;
			}
		}
		else // Don't send body on HEAD
			bodyFile = NULL;
		
		// Set status line
		response.SetStatusLine( statusCode );
		pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
		pb->Logprintf( "%ld Sending: %ld %s\n", pb->sn, contentLength, absPath.Path() );
		
		bigtime_t 		startTime, deltaTime;
		startTime = system_time();
		if( rangeSet.CountRanges() <= 1 ) // if less <= one byte range
		{
			// If partial content, seek to offset
			if( (statusCode == 206)&&(bodyFile != NULL) )
			{
				FileOffsetSpec		offset;
				if( rangeSet.GetFileOffset( &offset, entityLength, 0 ) )
					bodyFile->Seek( offset.offset, SEEK_SET );
			}
			size = pb->request->SendReply( &response ); // Send standard reply
		}
		else // Send multipart MIME message
		{
			// Not implemented yet... send status 200 OK and full message instead
			statusCode = 200;
			response.SetStatusLine( statusCode );
			size = pb->request->SendReply( &response );
		}
		deltaTime = system_time() - startTime;
		int32 		bytesSeconds = (size*1000000)/deltaTime;
		pb->Logprintf( "%ld Sent: %ld bytes %ld ms %ld bytes/second\n", 
			pb->sn, int32(size), int32(deltaTime/1000), bytesSeconds );
	}
	else
	{
		response.SetHTMLMessage( 404 ); // Not Found
		pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
		pb->request->SendReply( &response );
	}
	return B_OK;
}