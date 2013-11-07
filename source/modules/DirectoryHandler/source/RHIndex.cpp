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
#include <Entry.h>
#include <Directory.h>
#include <Path.h>
#include <stdio.h>
#include <NodeInfo.h>
#include <Mime.h>
#include <Bitmap.h>
#include <TranslatorRoster.h>
#include <TranslatorFormats.h>
#include <BitmapStream.h>
#include <Query.h>
#include <Volume.h>
#include <fs_attr.h>
#include "RHModuleInterface.h"
#include "HTTPMessage.h"
#include "HTTPRealm.h"
#include "HTTPUtils.h"
#include "StringUtils.h"
#include "DataIOUtils.h"
#include "time.h"
#include "RHModuleInit.h"
#include "IconCache.h"

static const char *kBodyHeader = 
"<HTML>\n"
"<HEAD>\n"
"<TITLE>%s</TITLE>\n"
"</HEAD>\n"
"<BODY bgcolor=\"#14458E\" text=\"#FFFFFF\" link=\"#84B5FE\" vlink=\"#84B5FE\" alink=\"#84B5FE\">\n";

static const char *kBodyFooter = 
"</BODY>\n"
"</HTML>\n";

static const char *kPathLine = 
"<CENTER><TABLE align=\"CENTER\" border=0 width=\"95%%\"><TR><TD bgcolor=\"#04357E\" valign=\"TOP\" align=\"CENTER\">"
"<FONT color=\"#FFFFFF\" size=\"+2\">\n"
"http://%s%s\n"
"</FONT></TD></TR></TABLE></CENTER>\n";

static const char *kDirItem = 
"<TR bgcolor=\"#3465AE\">"
"<TD><A href=\"%s\"><IMG src=\"/VBeIcons/%s\" align=left border=0 width=16 height=16>%s</A></TD>"
"<TD align=right>%s</TD><TD>%s</TD>"
"<TD>%s</TD></TR>\n";


static const char *kTableHeader = 
"<CENTER><TABLE border=0 width=\"95%%\" cellspacing=1 cellpadding=3>\n"
"<TR bgcolor=\"#24559E\">"
"<TH align=left>Name</TH>"
"<TH align=right>Size</TH>"
"<TH align=left>Kind</TH>"
"<TH align=left>Modified</TH>"
"</TR>\n";

static const char *kTablePathHeader = 
"<TR bgcolor=\"#24559E\"><TD colspan=4 align=center>http://<A href=\"/index\">%s</A>";

static const char *kTablePathSegment = 
"/<A href=\"%s\">%s</A>";

static const char *kTablePathFooter = 
"</TD></TR>\n";

static const char *kTableFooter =
"</TABLE></CENTER>\n";

bool GetPathSegment( char *dst, const char *path, int32 segment );
void MakeSizeString( char *string, int32 size );
void MIMEtoFile( char *dst, const char *mime );
void FiletoMIME( char *dst, const char *file );

static IconCache *icache;

status_t handle_request( RequestPB *pb )
{
	HTTPResponse	response;
	int32			contentLength;
	char			fieldValue[2048];
	char			headBuffer[1024];
	
	BPath			absPath;
	
	pb->closeConnection = false;
	BMallocIO		body;
	int32			isquery = 1;
	
	// If it's a directory
	if( (strcmp( "application/x-vnd.Be-directory", pb->mimeType  ) == 0)||
	(((isquery = strcmp(pb->mimeType, "application/x-vnd.Be-query"))==0)) )
	{
		absPath.SetTo( pb->webDirectory->Path(), pb->resourcePath->Path()+1 );
		
		// Check Authorization
		if( pb->authenticate &&	!pb->realms->Authenticate( pb->request, &response, pb->brURI->path, absPath.Path(), S_IXOTH ) )
		{
			pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
			return B_OK;
		}


		// fix for not showing contents of webroot when 404 directory request
		// wade majors <guru@startrek.com> Mar-08-2001
		if (!strcmp(absPath.Path(), pb->webDirectory->Path()))
		{
		  // request is for root
		  if (strcmp(pb->request->GetRequestLine(), "GET / ") >= 1)
		  {
		    // client didnt specifically request webroot, reject
		    goto notfound;
		  }
		}
		// end fix
		    
		
		BEntryList	*dir;
		entry_ref	ref;
		BEntry		entry;
		BNode		node;
		BNodeInfo	info;
		BMimeType	mime;
		char		href[2048];
		char		mimeTypeStr[B_MIME_TYPE_LENGTH];
		char		sizeString[256];
		char		modifiedStr[256];
		char		imageName[B_MIME_TYPE_LENGTH];
		off_t		entrySize;
		
		char entryName[B_FILE_NAME_LENGTH];
		const char *div;

	
		// Is it a query?
		if( isquery == 0 )
		{
			BFile		file;
			
			char		queryStr[4096];
			BQuery		*query;
			status_t	status;
			attr_info	attrInfo;
			
			if( ((file.SetTo( absPath.Path(), B_READ_ONLY ) != B_OK ))||
			( (status = file.GetAttrInfo( "_trk/qrystr", &attrInfo )) != B_OK )||
			( (status = file.ReadAttr( "_trk/qrystr", attrInfo.type, 0, queryStr, 4096 )) < 0 ) )
				goto notfound;
			queryStr[attrInfo.size] = 0;
			
			query = new BQuery();
			BVolume		volume;
			node_ref	nref;
			file.GetNodeRef( &nref );
			file.Unset();
			volume.SetTo( nref.device );
			query->SetVolume( &volume );
			query->SetPredicate( queryStr );
			query->Fetch();
			dir = query;
		}
		else
		{
			BDirectory		*myDir;
			myDir = new BDirectory();
			if( myDir->SetTo( absPath.Path() ) != B_OK )
			{
				delete myDir;
				goto notfound;
			}
			dir = myDir;
		}
		
		// ***
		// Create Response Body
		// ***
		
		// Add header
		io_printf( &body, kBodyHeader, pb->resourcePath->Path() );
		
		// Add Working Directory Line
		io_printf( &body, kPathLine, pb->brURI->host, pb->resourcePath->Path() );
		
		// Add Directory Contents
		io_printf( &body, kTableHeader );
		
		if( strcmp( pb->resourcePath->Path(), "/" ) == 0 )
			div = "";
		else
			div = "/";
		while( dir->GetNextEntry( &entry ) != B_ENTRY_NOT_FOUND )
		{
			entry.GetName( entryName );
			status_t	status;
			node.SetTo( &entry );
			info.SetTo( &node );
			
			// Cheap hack for directories without a MIME type...
			if( (status=info.GetType( mimeTypeStr )) != B_OK )
				strcpy( mimeTypeStr, "application/x-vnd.Be-directory" );
			
			// If symlink, follow the link
			if( (strcmp( mimeTypeStr, "application/x-vnd.Be-symlink" ) == 0) )
			{
				entry.GetRef( &ref );
				entry.SetTo( &ref, true );
				node.SetTo( &entry );
				info.SetTo( &node );
				info.GetType( mimeTypeStr );
			}
			if( (strcmp( mimeTypeStr, "application/x-vnd.Be-directory" ) == 0) )
				strcpy( sizeString, "-" );
				
			else
			{
				entry.GetSize( &entrySize );
				MakeSizeString( sizeString, entrySize );
			}
			
			time_t		modTime;
			struct tm 	*brokentime;
			entry.GetModificationTime( &modTime );
			brTimeLock.Lock();
			brokentime = localtime( &modTime );
			strftime( modifiedStr, 256, "%a, %b %d %Y, %H:%M:%S", brokentime );
			brTimeLock.Unlock();
			
			mime.SetTo( mimeTypeStr );
			MIMEtoFile( imageName, mimeTypeStr );
			mime.GetShortDescription( mimeTypeStr );
			sprintf( fieldValue, "%s%s%s", pb->resourcePath->Path(), div, entryName );
			uri_esc_str( href, fieldValue, 2048, false, "/" );
			io_printf( &body, kDirItem, href, imageName, entryName, sizeString, mimeTypeStr, modifiedStr );
		}
		
		// Add Table Path
		BPath		path;
		int32		segment = 1;
		io_printf( &body, kTablePathHeader, pb->brURI->host );
		while( GetPathSegment( fieldValue, pb->resourcePath->Path(), segment ) )
		{
			path.SetTo( fieldValue );
			uri_esc_str( href, path.Path(), 2048, false, "/" );
			
			io_printf( &body, kTablePathSegment, href, path.Leaf() );
			segment++;
		}
		
		io_printf( &body, kTablePathFooter );
		io_printf( &body, kTableFooter );
		
		// Add Footer
		io_printf( &body, kBodyFooter );
		
		// Tell client not to cache generated response
		response.AddHeader( "Cache-Control: no-cache" );
		response.AddHeader( "Pragma: no-cache" );
		
		// Add mime type header
		response.AddHeader( kHEAD_TYPE, "text/html" );
		
		if( isquery == 0 )
			delete ((BQuery *)dir);
		else
			delete ((BDirectory *)dir);
	}
	else if( strcmp( "image/VBeIcon", pb->mimeType  ) == 0 )
	{ // Create virtual Tracker Icon
		BPath				iconPath( "/" );
		char				mimeTypeStr[B_MIME_TYPE_LENGTH];
		iconItem			*iitem;
		
		iconPath.Append( pb->brURI->path );
		FiletoMIME( mimeTypeStr, iconPath.Leaf() );
		
		// Check the cache first
		icache->Lock();
		if( (iitem = icache->FindIcon( mimeTypeStr )) )
		{
			body.Write( iitem->buffer, iitem->size );
			icache->Unlock();
		}
		else
		{
			icache->Unlock();
			char				preferedAppSig[B_MIME_TYPE_LENGTH];
			BMimeType			mime, appMime;
			BBitmap 			iconBM( BRect( 0, 0, 15, 15 ), B_CMAP8 );
			BTranslatorRoster 	*roster = BTranslatorRoster::Default();
			
			mime.SetTo( mimeTypeStr );
			mime.GetPreferredApp( preferedAppSig );
			appMime.SetTo( preferedAppSig );
			
			// First try to get icon from prefered app
			if( appMime.GetIconForType( mimeTypeStr, &iconBM, B_MINI_ICON ) != B_OK )
			{
				// Then try the File Type database
				if( mime.GetIcon( &iconBM, B_MINI_ICON ) != B_OK )
				{
					// If still no luck, use generic icon
					mime.SetTo( "application/octet-stream" );
					if( mime.GetIcon( &iconBM, B_MINI_ICON ) != B_OK )
					{
						// If for some reason it still does not work, return not found
						goto notfound;
					}
				}
			}
			
			// For some reason, the PNG translator chops off the last row and column
			// We will make a copy of the icon with an extra row and column of
			// padding to compensate for this
			BBitmap				*ciconBM = new BBitmap( BRect( 0, 0, 16, 16 ), B_CMAP8 );
			int32				*src , *dst;
			for( int32 y=0; y < 16; y++ )
			{
				src = (int32 *)((char *)iconBM.Bits() + iconBM.BytesPerRow()*y);
				dst = (int32 *)((char *)ciconBM->Bits() + ciconBM->BytesPerRow()*y);
				for( int32 x=0; x < 4; x++ )
					*dst++ = *src++;
			}
			
			// Wrap the bitmap in a BitmapStream object to make acceptable to the translator
			BBitmapStream		*iconStream = new BBitmapStream( ciconBM );
			// Translate the icon to PNG format
			if( roster->Translate( iconStream, NULL, NULL, &body, B_PNG_FORMAT ) != B_OK )
				goto notfound;
			delete iconStream;
			
			// Cache Icon
			icache->Lock();
			icache->AddIcon( mimeTypeStr, (const char *)body.Buffer(), int32(body.BufferLength()) );
			icache->Unlock();
		}
		response.AddHeader( kHEAD_TYPE, "image/png" );
		absPath.SetTo( pb->webDirectory->Path(), pb->brURI->path );
	}
	else
		goto notfound;
	{
		contentLength = body.BufferLength();
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
		
		// Add content length header
		sprintf( headBuffer, "%s: %ld", kHEAD_LENGTH, contentLength );
		response.AddHeader( headBuffer );
		
		// If GET, set body
		if( pb->request->GetMethod() == METHOD_GET )
			response.SetMessageBody( &body );
		body.Seek( 0, SEEK_SET );
			
		response.SetStatusLine( 200 );
		pb->Logprintf( "%ld Sending: %ld %s\n", pb->sn, contentLength, absPath.Path() );
		pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
		int32	size;
		bigtime_t 		startTime, deltaTime;
		startTime = system_time();
		
		size = pb->request->SendReply( &response );
		
		deltaTime = system_time() - startTime;
		int32 		bytesSeconds = (size*1000000)/deltaTime;
		pb->Logprintf( "%ld Sent: %ld bytes %ld ms %ld bytes/second\n", 
			pb->sn, int32(size), int32(deltaTime/1000), bytesSeconds );
		return B_OK;
	}
	
	notfound:
	response.SetHTMLMessage( 404 ); // Not Found
	pb->Logprintf( "%ld Status-Line: %s\n", pb->sn, response.GetStatusLine() );
	pb->request->SendReply( &response );
	return B_OK;
}

bool can_handle_resource( const char *mimeType, http_method method, int32 *priority )
{
	if( ((strcmp(mimeType, "application/x-vnd.Be-directory")==0)||
	(strcmp(mimeType, "image/VBeIcon")==0)||
	(strcmp(mimeType, "application/x-vnd.Be-query")==0))
	&&((method == METHOD_GET)||(method == METHOD_HEAD)) )
	{
		*priority = 10;
		return true;
	}
	return false;
}

bool GetPathSegment( char *dst, const char *path, int32 segment )
{
	int32		n = 0;
	while( *path != 0 )
	{
		if( *path == '/' )
			n++;
		if( n > segment )
		{
			*dst = 0;
			return true;
		}
		*dst++ = *path++;
	}
	*dst = 0;
	if( n == segment )
		return true;
	else
		return false;
}

void MakeSizeString( char *string, int32 size )
{
	if( size < 1000 ) // bytes
		sprintf( string, "%ld bytes", size );
	else if( size < 1000000 ) // KB
		sprintf( string, "%.2f KB", float(size)/1000 );
	else // MB
		sprintf( string, "%.2f MB", float(size)/1000000 );
}

void MIMEtoFile( char *dst, const char *mime )
{
	do
	{
		if( *mime == '/' )
		{
			mime++;
			*dst++ = ',';
		}
		else
			*dst++ = *mime++;
	}while( *mime );
	*dst = 0;
}

void FiletoMIME( char *dst, const char *file )
{
	do
	{
		if( *file == ',' )
		{
			file++;
			*dst++ = '/';
		}
		else
			*dst++ = *file++;
	}while( *file );
	*dst = 0;
}

status_t init_module( void )
{
	icache = new IconCache( 32 );
	return B_OK;
}

void shutdown_module( void )
{
	delete icache;
}