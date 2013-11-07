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

#include "VResource.h"
#include "StringUtils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

VResource::VResource( void )
{
	*mimeType = 0;
	redirect = false;
	authenticate = true;
}

VResource::~VResource( void )
{
	void	*item;
	for( int32 i=0; (item=ItemAt(i)); i++ )
		free( item );
}

bool VResource::Match( const char *path, bool real )
{
	if( real != redirect )
		return false;
	const char *pattern;
	
	// Check each pattern in list until match is found
	for( int32 i=0; (pattern = (const char *)ItemAt(i)); i++ )
	{
		if( match_pattern( pattern, path ) )
			return true;
	}
	return false;
}

void VResource::SetMimeType( const char *mimeType )
{
	strxcpy( this->mimeType, mimeType, B_MIME_TYPE_LENGTH-1 );
}

void VResource::SetReal( bool real )
{
	redirect = real;
}

void VResource::AddPattern( const char *pattern )
{
	char *item;
	
	item = (char *)malloc( strlen( pattern )+1 );
	strcpy( item, pattern );
	AddItem( item );
}

void VResource::AddExtra( const char *fieldName, const char *fieldValue )
{
	extras.AddField( fieldName, fieldValue );
}

VResList::VResList( void )
	: BList( 20 )
{
	
}

VResList::~VResList( void )
{
	void	*item;
	for( int32 i=0; (item=ItemAt(i)); i++ )
		delete item;
}

void VResList::AddVRes( VResource *vres )
{
	AddItem( vres );
}

const char *VResList::MatchVRes( const char *path, bool real, VResource **vres )
{
	VResource		*vRes;
	
	for( int32 i=0; (vRes=(VResource *)ItemAt(i)); i++ )
	{
		if( vRes->Match( path, real ) )
		{
			if( vres )
				*vres = vRes;
			return vRes->GetMimeType();
		}
	}
	return NULL;
}
