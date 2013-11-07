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

#include "HModule.h"
#include <stdlib.h>
#include <string.h>
#include <Entry.h>
#include <Path.h>
#include <stdio.h>

HModule::HModule( void )
{
	image = -1;
	atomic = 0;
	available = false;
	name = NULL;
}

HModule::~HModule( void )
{
	UnloadModule();
}

status_t HModule::LoadModule( BPath *path )
{
	if( image < 0 )
	{
		if( (image = load_add_on( path->Path() )) >= 0 )
		{
			status_t (*init_module)( void );
			
			if( ( get_image_symbol( image, "handle_request", B_SYMBOL_TYPE_TEXT, (void **)&handle_request ) != B_OK)
				||( get_image_symbol( image, "can_handle_resource", B_SYMBOL_TYPE_TEXT, (void **)&can_handle_resource ) != B_OK) )
			{
				unload_add_on( image );
				image = -1;
				return B_ERROR;
			}
			
			// If init_module function, call it
			if( (get_image_symbol( image, "init_module", B_SYMBOL_TYPE_TEXT, (void **)&init_module ) == B_OK)
				&& (init_module() != B_OK) )
			{
				unload_add_on( image );
				image = -1;
				return B_ERROR;
			}
			
			const char *moduleName = path->Leaf();
			name = (char *)malloc( strlen( moduleName )+1 );
			strcpy( name, moduleName );
			
			BEntry entry (path->Path());
			entry.GetNodeRef( &moduleRef );
			
			available = true;
			return B_OK;
		}
		else
			return B_ERROR;
	}
	else
		return B_ERROR;
}

status_t HModule::UnloadModule( void )
{
	if( image >= 0 )
	{
		void (*shutdown_module)( void );
		
		unloadT = find_thread( NULL );
		available = false;
		if( atomic != 0 )
			suspend_thread( unloadT );
		
		// If shutdown_module function, call it
		if( get_image_symbol( image, "shutdown_module", B_SYMBOL_TYPE_TEXT, (void **)&shutdown_module ) == B_OK )
			shutdown_module();
		
		status_t	status;
		status = unload_add_on( image );
		image = -1;
		free( name );
		return status;
	}
	else
		return B_ERROR;
}

status_t HModule::HandleRequest( RequestPB *pb )
{
	if( !available )
		return B_ERROR;
	int32 previous;
	atomic_add( &atomic, 1 );
	
	int32 status;
	status = handle_request( pb );
	
	previous = atomic_add( &atomic, -1 );
	if( (previous == 1)&& !available )
		resume_thread( unloadT );
	return status;
}

bool HModule::CanHandleResource( const char *mimeType, http_method method, int32 *priority )
{
	if( !available )
		return false;
	int32 previous;
	atomic_add( &atomic, 1 );
	
	bool	status;
	status = can_handle_resource( mimeType, method, priority );
	
	previous = atomic_add( &atomic, -1 );
	if( (previous == 1)&& !available )
		resume_thread( unloadT );
	return status;
}

const char *HModule::GetName( void )
{
	return name;
}

void HModule::GetNodeRef( node_ref *nref )
{
	*nref = moduleRef;
}
