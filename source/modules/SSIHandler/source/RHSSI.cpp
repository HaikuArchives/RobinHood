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
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdlib.h>
#include <NodeInfo.h>
#include <DataIO.h>
#include <String.h>
#include "DataIOUtils.h"
#include "StringUtils.h"
#include "HTTPMessage.h"
#include "HTTPUtils.h"
#include "DataIOPump.h"
#include "RHModuleInterface.h"
#include "HTTPRealm.h"
#include "Environment.h"

static const char *kErrmsg = "SSI Parse Error";
static const char *kTimefmt = "%a, %d %b %Y %H:%M:%S";
static const char *kSizefmt = "%ld";

status_t invoke_ssi( BDataIO *input, BDataIO *output, BPath *CWD, RequestPB *pb );

status_t handle_request( RequestPB *pb )
{
	// Temporary buffers
	char			fieldValue[1024];
	char			headBuffer[1024];
	
	HTTPResponse	response;
	BFile			theFile;
	BPath			absPath;
	int32			contentLength;
	pb->closeConnection = false;
	
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
		
		BMallocIO		output;
			
		BPath			CWD( absPath );
		CWD.GetParent( &CWD );
		
		invoke_ssi( &theFile, &output, &CWD, pb );
		
		contentLength = output.BufferLength();
		response.SetContentLength( contentLength );
		
		// Add Date header
		time_t			now;
		struct tm 		*brokentime;
		
		now = time( NULL );
		brTimeLock.Lock();
		brokentime = gmtime( &now );
		strftime(fieldValue, 256, kHTTP_DATE, brokentime);
		brTimeLock.Unlock();
		
		response.AddHeader( kHEAD_DATE, fieldValue );
		
		// Tell client not to cache SSI generated response
		response.AddHeader( "Cache-Control: no-cache" );
		response.AddHeader( "Pragma: no-cache" );
		
		// Add content length header
		sprintf( headBuffer, "%s: %ld", kHEAD_LENGTH, contentLength );
		response.AddHeader( headBuffer );
		
		// Add mime type header
		response.AddHeader( kHEAD_TYPE, "text/html" );
		if( pb->request->GetMethod() == METHOD_GET )
			response.SetMessageBody( &output );
		output.Seek( 0, SEEK_SET );
		response.SetStatusLine( statusCode );
		
		pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
		pb->Logprintf( "%ld Sending: %ld %s\n", pb->sn, contentLength, absPath.Path() );
		int32	size;
		bigtime_t 		startTime, deltaTime;
		startTime = system_time();
		
		size = pb->request->SendReply( &response );
		
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

bool can_handle_resource( const char *mimeType, http_method method, int32 *priority )
{
	if( (strcmp( mimeType, "text/x-server-parsed-html" ) == 0)&&((method==METHOD_GET)||(method==METHOD_HEAD)) )
	{
		*priority = 10;
		return true;
	}
	return false;
}

status_t invoke_ssi( BDataIO *input, BDataIO *output, BPath *CWD, RequestPB *pb )
{
	BPath		previousCWD;
	status_t	status = B_NO_ERROR;
	
	char		lineBuffer[4096];
	char		command[64];
	char		tagName[64];
	char		tagValue[4096];
	
	const char	*sPtr, *ePtr;
	
	BString			errmsg( kErrmsg );
	BString			timefmt( kTimefmt );
	BString			sizefmt( kSizefmt );
	
	// Save Current Working Directory
	previousCWD.SetTo( getcwd( lineBuffer, 4096 ) );
	
	// Set Current Working Directory
	chdir( CWD->Path() );
	
	// Read lines until eof
	while( io_getline( input, lineBuffer, 4096 ) )
	{
		// Look for ssi command in line
		if( (sPtr = strstr( lineBuffer, "<!--#" )) )
		{
			BString		trailingSegment;
			
			// If leading segment, copy to output
			if( sPtr != lineBuffer )
			{
				*((char *)sPtr) = 0;
				io_printf( output, "%s", lineBuffer );
			}
			
			sPtr += 5; // Move to command
			if( (ePtr = strstr( sPtr, "-->" )) )
			{
				if( ePtr[3] != 0 )
					trailingSegment.SetTo( ePtr+3 );
				*((char *)ePtr) = 0; // Terminate string
			}
			
			sPtr = get_next_token( command, sPtr, 64 );
			if( strcasecmp( command, "config" ) == 0 )
			{
				// Read tags
				while( (sPtr = get_next_field( sPtr, tagName, tagValue )) )
				{
					if( strcasecmp( tagName, "errmsg" ) == 0 )
						errmsg.SetTo( tagValue );
					else if( strcasecmp( tagName, "timefmt" ) == 0 )
						timefmt.SetTo( tagValue );
					else if( strcasecmp( tagName, "sizefmt" ) == 0 )
						sizefmt.SetTo( tagValue );
				}
			}
			else if( strcasecmp( command, "echo" ) == 0 )
			{
				char		fieldValue[4096];
				struct tm 	*brokentime;
				
				// Read tags
				while( (sPtr = get_next_field( sPtr, tagName, tagValue )) )
				{
					if( strcasecmp( tagName, "var" ) == 0 )
					{
						if( strcmp( tagValue, "DOCUMENT_NAME" ) == 0 )
							io_printf( output, "%s", pb->resourcePath->Leaf() );
						else if( strcmp( tagValue, "DOCUMENT_URI" ) == 0 )
							io_printf( output, "%s", pb->resourcePath->Path() );
						else if( strcmp( tagValue, "QUERY_STRING_UNESCAPED" ) == 0 )
						{
							uri_unescape_str( fieldValue, pb->brURI->query, 4096 );
							io_printf( output, "%s", fieldValue );
						}
						else if( strcmp( tagValue, "DATE_LOCAL" ) == 0 )
						{
							time_t		now;
							now = time( NULL );
							brTimeLock.Lock();
							brokentime = localtime( &now );
							strftime( fieldValue, 256, timefmt.String(), brokentime );
							brTimeLock.Unlock();
							io_printf( output, "%s", fieldValue );
						}
						else if( strcmp( tagValue, "LAST_MODIFIED" ) == 0 )
						{
							time_t		modTime;
							BNode		node;
							BPath		absPath;
							
							absPath.SetTo( pb->webDirectory->Path(), pb->resourcePath->Path()+1 );
							if( node.SetTo( absPath.Path() ) == B_OK )
							{
								node.GetModificationTime( &modTime );
								brTimeLock.Lock();
								brokentime = localtime( &modTime );
								strftime( fieldValue, 256, timefmt.String(), brokentime );
								brTimeLock.Unlock();
								
								io_printf( output, "%s", fieldValue );
							}
						}
						else if( strcmp( tagValue, "DATE_GMT" ) == 0 )
						{
							time_t		now;
							now = time( NULL );
							brTimeLock.Lock();
							brokentime = gmtime( &now );
							strftime( fieldValue, 256, timefmt.String(), brokentime );
							brTimeLock.Unlock();
							io_printf( output, "%s", fieldValue );
						}
					}
					else
						io_printf( output, "%s\n", errmsg.String() );
				}
			}
			else if( strcasecmp( command, "fsize" ) == 0 )
			{
				BPath		includePath;
				// Read tags
				while( (sPtr = get_next_field( sPtr, tagName, tagValue )) )
				{
					if( strcasecmp( tagName, "virtual" ) == 0 )
						includePath.SetTo( pb->webDirectory->Path(), tagValue+1 );
					else if( strcasecmp( tagName, "file" ) == 0 )
						includePath.SetTo( tagValue );
					else
						break;
					BNode		node;
					
					if( node.SetTo( includePath.Path() ) == B_OK )
					{
						off_t		size;
						
						node.GetSize( &size );
						io_printf( output, sizefmt.String(), int32(size) );
					}
					else
						io_printf( output, "%s\n", errmsg.String() );
				}
			}
			else if( strcasecmp( command, "flastmod" ) == 0 )
			{
				BPath		includePath;
				// Read tags
				while( (sPtr = get_next_field( sPtr, tagName, tagValue )) )
				{
					if( strcasecmp( tagName, "virtual" ) == 0 )
						includePath.SetTo( pb->webDirectory->Path(), tagValue+1 );
					else if( strcasecmp( tagName, "file" ) == 0 )
						includePath.SetTo( tagValue );
					else
						break;
					BNode		node;
					
					if( node.SetTo( includePath.Path() ) == B_OK )
					{
						time_t		modTime;
						struct tm 	*brokentime;
						char		fieldValue[2048];
						
						node.GetModificationTime( &modTime );
						brTimeLock.Lock();
						brokentime = localtime( &modTime );
						strftime( fieldValue, 256, timefmt.String(), brokentime );
						brTimeLock.Unlock();
						
						io_printf( output, "%s", fieldValue );
					}
					else
						io_printf( output, "%s\n", errmsg.String() );
				}
			}
			else if( strcasecmp( command, "include" ) == 0 )
			{
				BPath		includePath;
				// Read tags
				while( (sPtr = get_next_field( sPtr, tagName, tagValue )) )
				{
					if( strcasecmp( tagName, "virtual" ) == 0 )
						includePath.SetTo( pb->webDirectory->Path(), tagValue+1 );
					else if( strcasecmp( tagName, "file" ) == 0 )
						includePath.SetTo( tagValue );
					else
						break;
					BFile		includeFile;
					
					if( includeFile.SetTo( includePath.Path(), B_READ_ONLY ) == B_NO_ERROR )
					{
						BNodeInfo	theNode( &includeFile );
						char		mimeType[1024];
						
						theNode.GetType( mimeType );
						// Should we invoke SSI recursively?
						if( strcmp( mimeType, "text/x-server-parsed-html" ) == 0 )
						{
							BPath		NWD( includePath );
							NWD.GetParent( &NWD );
							invoke_ssi( &includeFile, output, &NWD, pb );
						}
						else // Append file
						{
							DataIOPump		ioPump;
							ioPump.StartPump( &includeFile, output );
						}
					}
					else
						io_printf( output, "%s\n", errmsg.String() );
				}
			}
			else if( strcasecmp( command, "exec" ) == 0 )
			{
				bool	html = false;
				// Read tags
				while( (sPtr = get_next_field( sPtr, tagName, tagValue )) )
				{
					if( strcasecmp( tagName, "cmd" ) == 0 )
					{
						FILE	*stream;
						if( (stream = popen ( tagValue, "r" )) )
						{
							while( fgets( lineBuffer, 4096, stream) )
								io_printf( output, "%s", lineBuffer );
							pclose( stream );
						}
						else
							io_printf( output, "%s\n", errmsg.String() );
					}
					else if( strcasecmp( tagName, "html" ) == 0 )
					{
						if( strcasecmp( tagValue, "true" ) == 0 )
							html = true;
						else if( strcasecmp( tagValue, "false" ) == 0 )
							html = false;
					}
					else if( strcasecmp( tagName, "cgi" ) == 0 )
					{
						HTTPRequest		request;
						HTTPResponse	response;
						brokenURI		brURI;
						BMallocIO		replyIO;
						RequestPB 		cgipb;
						
						request.SetRequestLine( "GET", tagValue );
						request.SetReplyIO( &replyIO );
						request.ParseURI( &brURI );
						uri_unescape_str( brURI.path, brURI.path, 2048 );
						
						cgipb.request = &request;
						cgipb.webDirectory = pb->webDirectory;
						cgipb.realms = pb->realms;
						cgipb.vresources = pb->vresources;
						cgipb.brURI = &brURI;
						cgipb.environ = pb->environ;
						cgipb.authenticate = false;
						cgipb.sn = pb->sn;
						cgipb.cookie = NULL;
						
						pb->HandleRequest( &cgipb );
						response.InitMessage();
						replyIO.Seek( 0, SEEK_SET );
						response.ReceiveMessage( &replyIO );
						int32 statusCode = response.GetStatusCode();
						
						if( statusCode == 302 )
						{
							if( response.FindHeader( kHEAD_LOCATION, lineBuffer, 4096 ) )
								io_printf( output, "<A HREF=\"%s\">", lineBuffer );
						}
						else if( (statusCode >= 200)&&(statusCode < 300) )
						{
							if( response.FindHeader( kHEAD_TYPE, lineBuffer, 4096 ) )
							{
								if( strncmp( lineBuffer, "text/", 5 ) == 0 )
								{
									// Should we remove the html header and footer?
									if( html && (strcmp( lineBuffer, "text/html" ) == 0) )
									{
										bool		foundbody = false;
										
										while( io_getline( &replyIO, lineBuffer, 4096 ) )
										{
											// If body not found, look for it
											if( !foundbody && ((sPtr = strstr( lineBuffer, "<BODY" ))||
												(sPtr = strstr( lineBuffer, "<body" ) )) )
											{
												if( (sPtr = strchr( sPtr, '>' ))&&(sPtr[1] != 0) )
													io_printf( output, "%s\n", sPtr );
												foundbody = true;
											}
											// If body found, is this the end of the body?
											else if( foundbody && ((sPtr = strstr( lineBuffer, "</BODY" ))||
												 (sPtr = strstr( lineBuffer, "</body" ) )) )
											{
												*((char *)sPtr) = 0;
												if( *lineBuffer != 0 )
													io_printf( output, "%s\n", lineBuffer );
												break;
											}
											// If body found and it's not the end, copy the line
											else if( foundbody )
												io_printf( output, "%s\n", lineBuffer );
										}
									}
									else // Copy as is
									{
										DataIOPump		ioPump;
										ioPump.StartPump( &replyIO, output );
									}
								}
							}
						}
						else
							io_printf( output, "%s\n", errmsg.String() );
					} // End else if( strcasecmp( tagName, "cgi" ) == 0 )
				} // End while( (sPtr = get_next_field( sPtr, tagName, tagValue )) )
			} // End else if( strcasecmp( command, "exec" ) == 0 )
			if( trailingSegment.Length() )	
				io_printf( output, "%s\n", trailingSegment.String() );
		} // End Look for ssi command in line
		else // no ssi command; just copy the line
			io_printf( output, "%s\n", lineBuffer );
	} // End while( io_getline( input, lineBuffer, 4096 ) )
	
	// Restore previous Current Working Directory
	chdir( previousCWD.Path() );
	return status;
}