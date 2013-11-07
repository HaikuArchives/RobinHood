// RH Console - A GUI console for the Robin Hood Web Server
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

#include <string.h>
#include "RHMsgFilter.h"
#include "StringUtils.h"

int32 get_line_type( const char *typeName )
{
	if( strcmp( typeName, "Open" ) == 0 )
		return M_OPEN;
	else if( strcmp( typeName, "Close" ) == 0 )
		return M_CLOSE;
	else if( strcmp( typeName, "Request-Line" ) == 0 )
		return M_REQUEST_LINE;
	else if( strcmp( typeName, "Header" ) == 0 )
		return M_HEADER;
	else if( strcmp( typeName, "Status-Line" ) == 0 )
		return M_STATUS_LINE;
	else if( strcmp( typeName, "Sending" ) == 0 )
		return M_SENDING;
	else if( strcmp( typeName, "Sent" ) == 0 )
		return M_SENT;
	else if( strcmp( typeName, "Exec" ) == 0 )
		return M_EXEC;
	else
		return M_OTHER;
}

int32 get_header_type( const char *headerName )
{
	if( strcasecmp( headerName, "User-Agent" ) == 0 )
		return H_USER_AGENT;
	else if( strcasecmp( headerName, "Accept" ) == 0 )
		return H_ACCEPT;
	else if( strcasecmp( headerName, "Host" ) == 0 )
		return H_HOST;
	else if( strcasecmp( headerName, "Authorization" ) == 0 )
		return H_AUTHORIZATION;
	else if( strcasecmp( headerName, "Accept-Language" ) == 0 )
		return H_ACCEPT_LANGUAGE;
	else if( strcasecmp( headerName, "Accept-Encoding" ) == 0 )
		return H_ACCEPT_ENCODING;
	else if( strcasecmp( headerName, "Range" ) == 0 )
		return H_RANGE;
	else if( strcasecmp( headerName, "Via" ) == 0 )
		return H_VIA;
	else if( strcasecmp( headerName, "Referer" ) == 0 )
		return H_REFERER;
	else if( strcasecmp( headerName, "Connection" ) == 0 )
		return H_CONNECTION;
	else if( strcasecmp( headerName, "Extension" ) == 0 )
		return H_EXTENSION;
	else if( strcasecmp( headerName, "From" ) == 0 )
		return H_FROM;
	else if( strncmp( headerName, "UA-", 3 ) == 0 )
		return H_UA;
	else
		return H_OTHER;
}

bool filter_line_type( int32 filterMask, const char *line, int32 *type, const char **textPtr )
{
	const char		*sPtr = line;
	char			token[128];
	int32			ltype;
	
	sPtr = get_next_token( NULL, sPtr, 0 );
	sPtr = get_next_token( token, sPtr, 128, ":" );
	if( textPtr )
		*textPtr = (char *)sPtr;
	ltype = get_line_type( token );
	if( type )
		*type = ltype;
	if( filterMask & ltype )
		return true;
	else
		return false;
}

bool filter_header_type( int32 filterMask, const char *header, int32 *type )
{
	const char		*sPtr = header;
	char			token[128];
	int32			htype;
	
	sPtr = get_next_token( token, sPtr, 128, ":" );
	
	htype = get_header_type( token );
	if( type )
		*type = htype;
	if( filterMask & htype )
		return true;
	else
		return false;
}
