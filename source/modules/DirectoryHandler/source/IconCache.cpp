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
#include "IconCache.h"

IconCache::IconCache( int32 maxItems )
{
	this->maxItems = maxItems;
}

IconCache::~IconCache( void )
{
	iconItem	*item;
	
	for( int32 i=0; (item=(iconItem *)ItemAt(i)); i++ )
	{
		free( item->buffer );
		delete item;
	}
}

void IconCache::AddIcon( const char *name, const char *buffer, int32 size )
{
	iconItem	*item;
	item = new iconItem;
	item->name.SetTo( name );
	item->size = size;
	item->buffer = (char *)malloc( size );
	memcpy( item->buffer, buffer, size );
	if( CountItems() == maxItems )
	{
		iconItem	*discardItem;
		discardItem = (iconItem *)ItemAt( 0 );
		free( discardItem->buffer );
		delete discardItem;
		RemoveItem( int32(0) );
	}
	AddItem( item );
}

iconItem *IconCache::FindIcon( const char *name )
{
	iconItem	*item;
	
	for( int32 i=0; (item=(iconItem *)ItemAt(i)); i++ )
	{
		if( strcmp( name, item->name.String() ) == 0 )
			return item;
	}
	return NULL;
}