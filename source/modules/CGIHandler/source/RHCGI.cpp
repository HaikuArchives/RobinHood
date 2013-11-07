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
#include "HTTPMessage.h"
#include "HTTPUtils.h"
#include "DataIOPump.h"
#include "BufferIO.h"
#include "DataIOUtils.h"
#include "FILE_IO.h"
#include "Environment.h"
#include "StringUtils.h"
#include "HTTPRealm.h"
#include "RHModuleInterface.h"

status_t handle_request( RequestPB *pb )
{
	HTTPResponse	response;
	pb->closeConnection = false;
	
	const char		*sPtr; // General purpose string pointer
	// Temporary buffers
	int32			fieldSize = 1024;
	char			fieldValue[1024];
	char			headBuffer[1024];
	int32 			contentLength = 0;
	
	// ****
	// Get PATH_INFO and SCRIPT_NAME from path; Setup absPath of CGI
	// ****
	char PATH_INFO[1024];
	char SCRIPT_NAME[256];
	
	// Get SCRIPT_NAME
	strxcpy( SCRIPT_NAME, pb->resourcePath->Path(), 255 );
	
	strxcpy( PATH_INFO, pb->brURI->path+strlen( pb->resourcePath->Path()+1 ), 1023 );
	
	
	// Make absolute CGI path from web-directory and requested path
	BPath		absPath( pb->webDirectory->Path(), pb->resourcePath->Path()+1 );
	
	// ****
	// Make sure CGI exists and Check Permission
	// ****
	
	if( pb->authenticate &&	!pb->realms->Authenticate( pb->request, &response, pb->brURI->path, absPath.Path(), S_IXOTH ) )
	{
		pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
		return B_OK;
	}
	
	// ****
	// Setup meta-variables in new environment
	// ****
	char		params[2048];
	
	// Should we use the CGI script command line?
	// This should be done on a GET or HEAD where the URL query string
	// 		does not contain any unencoded '=' characters.
	if( *pb->brURI->query && 
		((pb->request->GetMethod() == METHOD_GET)||(pb->request->GetMethod() == METHOD_HEAD))&&
		!strchr( pb->brURI->query, '=' ) )
	{
		uri_unescape_str( params, pb->brURI->query, 2048 );
	}
	else
		uri_unescape_str( params, pb->brURI->params, 2048 );
	
	// Environment to be used by the CGI
	Environment 	env( pb->environ );
	
	// AUTH_TYPE
	if( pb->request->FindHeader( kHEAD_AUTHORIZATION, fieldValue, fieldSize ) )
	{
		sPtr = fieldValue;
		sPtr = get_next_token( headBuffer, sPtr, fieldSize );
		env.PutEnv( "AUTH_TYPE", headBuffer );
		if( strcasecmp( headBuffer, "Basic" ) == 0 )
		{
			// REMOTE_USER
			sPtr = get_next_token( headBuffer, sPtr, fieldSize );
			decode_base64( headBuffer, headBuffer, fieldSize );
			sPtr = get_next_token( fieldValue, headBuffer, fieldSize, ":" );
			env.PutEnv( "REMOTE_USER", fieldValue );
		}
	}
	
	// CONTENT_LENGTH
	if( pb->request->FindHeader( kHEAD_LENGTH, fieldValue, fieldSize ) )
		env.PutEnv( "CONTENT_LENGTH", fieldValue );
	
	// CONTENT_TYPE
	if( pb->request->FindHeader( kHEAD_TYPE, fieldValue, fieldSize ) )
		env.PutEnv( "CONTENT_TYPE", fieldValue );
	
	// GATEWAY_INTERFACE
	env.PutEnv( "GATEWAY_INTERFACE", "CGI/1.1" );
	// HTTP_*
	for( int i=0; (sPtr = pb->request->HeaderAt( i )); i++ )
	{
		sPtr = get_next_token( fieldValue, sPtr, fieldSize, ":" );
		sprintf( headBuffer, "HTTP_%s", http_to_cgi_header( fieldValue ) );
		sPtr = get_next_token( fieldValue, sPtr, fieldSize, ":" );
		env.PutEnv( headBuffer, fieldValue );
	}
	
	// PATH_INFO
	env.PutEnv( "PATH_INFO", PATH_INFO );
	
	// PATH_TRANSLATED
	if( *PATH_INFO )
	{
		BPath		pathTrans( pb->webDirectory->Path(), PATH_INFO+1 );
		
		if( pathTrans.Path() )
			env.PutEnv( "PATH_TRANSLATED", pathTrans.Path() );
	}
	
	// QUERY_STRING
	env.PutEnv( "QUERY_STRING", pb->brURI->query );
	
	// REMOTE_ADDR
	env.PutEnv( "REMOTE_ADDR", pb->request->GetRemoteHost() );
	
	// REMOTE_HOST
	// Ya, right... like were going to waste valuable time with a DNS lookup!
	env.PutEnv( "REMOTE_HOST", "" );
	
	// REMOTE_IDENT
	// Ha! Perform an Ident lookup... I don't think so.
	
	// REQUEST_METHOD
	env.PutEnv( "REQUEST_METHOD", http_find_method( pb->request->GetMethod() ) );
	
	// SCRIPT_NAME
	env.PutEnv( "SCRIPT_NAME", SCRIPT_NAME );
	
	// SERVER_NAME
	env.PutEnv( "SERVER_NAME", pb->brURI->host );
	
	// SERVER_PORT
	sprintf( fieldValue, "%u", pb->request->GetPort() );
	env.PutEnv( "SERVER_PORT", fieldValue );
	
	// SERVER_PROTOCOL
	env.PutEnv( "SERVER_PROTOCOL", pb->request->GetVersion() );
	
	// SERVER_SOFTWARE
	env.PutEnv( "SERVER_SOFTWARE", "RobinHood" );
	
	// PWD
	BPath		PWD( absPath );
	PWD.GetParent( &PWD );
	env.PutEnv( "PWD", PWD.Path() );
	
	// ****
	// Create pipes
	// ****
	
	pid_t	pid;
	int 	ipipe[2], opipe[2];
	
	if( pipe(ipipe) < 0 )
	{
		response.SetHTMLMessage( 500, "Pipe creation failed!" );
		pb->request->SendReply( &response );
		return B_OK;
	}
	if( pipe(opipe) < 0 ) 
	{
		close( ipipe[0] );
		close( ipipe[1] );
		response.SetHTMLMessage( 500, "Pipe creation failed!" );
		pb->request->SendReply( &response );
		return B_OK;
	}
	
	// ****
	// Setup args for execve()
	// ****
	
	// Setup command string; copy CGI path and append params
	char command[4096];
	sPtr = strxcpy( command, absPath.Path(), 4095 );
	
	// Disabled because of security risk
	/*
	if( *params && !strpbrk( params, ";&" ) )
	{
		sPtr = strxcpy( (char *)sPtr, " ", command+4095-sPtr );
		strxcpy( (char *)sPtr, params, command+4095-sPtr ); // Append params
	}*/
	
	char *args[4];
	args[0] = "/bin/sh";
	args[1] = "-c";
	args[2] = command;
	args[3] = NULL;
	
	pb->Logprintf( "%ld Exec: %s\n", pb->sn, command );
	
	// ****
	// Start sub-process using fork() dup2() and exec()
	// ****
	
	pid = fork();
	if( pid == (pid_t)0 ) // If we are the child process...
	{
		// Make this process the process group leader
		setpgid( 0, 0 );
		fflush(stdout); // sync stdout... 
		
		// Set pipes to stdin and stdout
		if( dup2( opipe[0], STDIN_FILENO ) < 0 )
			exit( 0 );
		if( dup2( ipipe[1], STDOUT_FILENO ) < 0 )
			exit( 0 );
		// Close unused ends of pipes
		close( opipe[1] );
		close( ipipe[0] );
		
		// Set Current Working Directory to that of script
		chdir( PWD.Path() );
		
		// Execute CGI in this process by means of /bin/sh 
		execve( args[0], args, env.GetEnvironment() );
		exit( 0 ); // If for some reason execve() fails...
	}
	else if( pid < (pid_t)0 ) // Something Bad happened!
	{
		close( opipe[0] );
		close( opipe[1] );
		close( ipipe[0] );
		close( ipipe[1] );
		response.SetHTMLMessage( 500, "Fork failed!" );
		pb->request->SendReply( &response );
		return true;
	}
	// Close unused ends of pipes
	close( opipe[0] );
	close( ipipe[1] );
	
	// ****
	// Talk to CGI
	// ****
	
	bool		persistant = true;
	
	// Defined to make code easier to read
	int			inDes = ipipe[0]; // input file descripter
	int			outDes = opipe[1]; // output file descripter
	
	// Make a BDataIO wrapper for the in and out pipes
	DesIO		pipeIO( inDes, outDes );
	
	// If the request contains a content body, feed it into stdin of the CGI script
	if( pb->request->GetContentLength() > 0 )
		pb->request->SendBody( &pipeIO );
	
	// Buffer the response body for better performance
	response.SetBodyBuffering( true );
	
	// Read first line to detect use of Non-Parsed Header Output
	io_getline( &pipeIO, headBuffer, fieldSize );
	
	// Strip the '\r' character if there is one
	int32	size;
	size = strlen( headBuffer )-1;
	if( headBuffer[size] == '\r' )
		headBuffer[size] = 0;
	
	// Is NPH Output?
	if( strncmp( "HTTP/", headBuffer, 5 ) == 0 )
	{
		DataIOPump		ioPump;
		BufferedIO		bufio( pb->request->GetReplyIO() );
		bufio.DoAllocate();
		
		io_printf( &bufio, "%s\r\n", headBuffer );
		ioPump.StartPump( &pipeIO, &bufio, contentLength );
		bufio.Sync();
		persistant = false;
	}
	else // using Parsed Header Output
	{
		// Add Date header
		time_t			now;
		struct tm 		*brokentime;
		
		now = time( NULL );
		brTimeLock.Lock();
		brokentime = gmtime( &now );
		strftime (fieldValue, 256, kHTTP_DATE, brokentime);
		brTimeLock.Unlock();
		
		response.AddHeader( kHEAD_DATE, fieldValue );
		
		// Add line used to detect NPH as CGI header
		response.AddHeader( headBuffer );
		
		// Receive the CGI headers
		response.ReceiveHeaders( &pipeIO );
			
		// If Location header, don't expect any more headers
		if( (sPtr = response.FindHeader( "Location", fieldValue, fieldSize )) )
		{
			response.SetStatusLine( 302 ); // 302 Moved Temporarily
		}
		else
		{
			if( (sPtr = response.FindHeader( "Status", fieldValue, fieldSize )) )
			{
				response.RemoveHeader( (char *)sPtr ); // Don't forward to client
				response.SetStatusLine( fieldValue );
			}
			else
				response.SetStatusLine( 200 );
		}
		
		// Don't cache the response
		if( !response.FindHeader( "Cache-Control", fieldValue, fieldSize ) )
			response.AddHeader( "Cache-Control: no-cache" );
		if( !response.FindHeader( "Pragma", fieldValue, fieldSize ) )
			response.AddHeader( "Pragma: no-cache" );
		
		// Content-Length header?
		int32		contentLength = 0;
		
		if( (sPtr = response.FindHeader( kHEAD_LENGTH, fieldValue, fieldSize )) )
		{
			contentLength = strtol( fieldValue, (char **)&headBuffer, 10 );
			response.SetContentLength( contentLength );
		}
		else // No content-length provided; close connection on return
		{
			response.AddHeader( "Connection: close" );
			persistant = false;
		}
		
		pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
		if( pb->request->GetMethod() != METHOD_HEAD )
			response.SetMessageBody( &pipeIO );
		pb->request->SendReply( &response );
	}
	
	// Close remaining ends of pipes
	close( ipipe[0] );
	close( opipe[1] );
	pb->closeConnection = !persistant;
	return B_OK;
}

bool can_handle_resource( const char *mimeType, http_method method, int32 *priority )
{
	if( strcmp( mimeType, "application/x-vnd.CGI" ) == 0 )
	{
		*priority = 10;
		return true;
	}
	return false;
}
