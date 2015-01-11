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

#include "HModuleRoster.h"
#include "HModule.h"
#include "HTTPUtils.h"
#include "RHLogger.h"
#include "StringUtils.h"
#include "VResource.h"
#include <Directory.h>
#include <NodeInfo.h>
#include <NodeMonitor.h>
#include <string.h>
#include <stdio.h>
#include <StopWatch.h>

BList HModuleRoster::moduleList;
HModuleRoster *hmodule_roster = NULL;

HModuleRoster::HModuleRoster( void )
{
	watching = false;
}

HModuleRoster::~HModuleRoster( void )
{
	
}

status_t HModuleRoster::StartWatching( BPath *moduleDirectory )
{
	if( watching )
		return B_ERROR;
	printf("Watching %s\n", moduleDirectory->Path());
	BEntry		entry;
	
	if( (entry.SetTo( moduleDirectory->Path() ) == B_OK)&&
		(entry.GetNodeRef( &watchedRef ) == B_OK) )
	{
		watching = true;
		return watch_node( &watchedRef, B_WATCH_DIRECTORY, this );
	}
	else
		return B_ERROR;
}

status_t HModuleRoster::StopWatching( void )
{
	if( !watching )
		return B_ERROR;
	stop_watching( this );
	watching = false;
	return B_OK;
}

// *****
// Respond to changes to the add-on modules directory
// *****

void HModuleRoster::MessageReceived( BMessage *message )
{
	switch( message->what )
	{
		case B_NODE_MONITOR:
		{
			int32		opcode;
			
			opcode = message->FindInt32( "opcode" );
			switch( opcode )
			{
				case B_ENTRY_CREATED:
				{
					entry_ref ref;
					const char *name;
					message->FindInt32( "device", &ref.device );
					message->FindInt64( "directory", &ref.directory );
					message->FindString( "name", &name );
					ref.set_name( name );
					
					BEntry		entry( &ref, true );
					BPath		modulePath;
					entry.GetPath( &modulePath );
					LoadModule( &modulePath );
					break;
				}	
				case B_ENTRY_REMOVED:
				{
					node_ref 	nref;
					message->FindInt32( "device", &nref.device );
					message->FindInt64( "node", &nref.node );
					UnloadModule( &nref );
					break;
				}
				case B_ENTRY_MOVED:
				{
					node_ref dirRef;
					message->FindInt32( "device", &dirRef.device );
					message->FindInt64( "to directory", &dirRef.node );
					
					if( dirRef != watchedRef )
					{
						entry_ref ref;
						const char *name;
						message->FindInt32( "device", &ref.device );
						message->FindInt64( "to directory", &ref.directory );
						message->FindString( "name", &name );
						ref.set_name( name );
						
						BEntry		entry( &ref, true );
						BNode		node( &entry );
						
						node_ref nref;
						node.GetNodeRef( &nref );
						UnloadModule( &nref );
					}
					else
					{
						entry_ref ref;
						const char *name;
						message->FindInt32( "device", &ref.device );
						message->FindInt64( "to directory", &ref.directory );
						message->FindString( "name", &name );
						ref.set_name( name );
						
						BEntry		entry( &ref, true );
						BPath		modulePath;
						entry.GetPath( &modulePath );
						LoadModule( &modulePath );
					}
					break;
				}
			}
			break;
		}
		default:
			BHandler::MessageReceived( message );
			break;
	}
}

status_t HModuleRoster::HandleRequest( RequestPB *pb )
{
	BEntry		entry;
	BNode		node;
	BNodeInfo	info;
	char		mimeType[128], vmimeType[128];
	status_t 	status = B_OK;
	int32		parentCount = 0;
	BPath 		absPath, resourcePath( "/" );
	resourcePath.Append( pb->brURI->path );
	pb->resourcePath = &resourcePath;
	pb->mimeType = mimeType;
	
	
	// fix for "hostname//" request crash
	// wade majors <guru@startrek.com - Mar-09-2001
    if (resourcePath.Path() == NULL)
    {
      resourcePath.SetTo("null");
      pb->resourcePath = &resourcePath;
    }
    //
	
	
	VResource	*vres = NULL;
		
	// *****
	// Look for "real" resource
	// *****
	do
	{
		// Small optimization... if not done, the path normalizer will 
		// be tickled when a resource does not exit
		if( (resourcePath.Path())[1] == 0 )
		{
			status = B_ERROR;
			break;
		}
		absPath.SetTo( pb->webDirectory->Path(), resourcePath.Path()+1 );

		if( (entry.SetTo( absPath.Path(), true ) == B_OK)&&(node.SetTo( &entry ) == B_OK)
		&&(info.SetTo( &node ) == B_OK) )
		{
			const char *resMIME;
			
			// Cheap hack for directories without a MIME type
			if(info.GetType( mimeType ) != B_OK)
				strcpy( mimeType, "application/x-vnd.Be-directory" );
				
			if( (resMIME = pb->vresources->MatchVRes( pb->brURI->path, true, &vres )) )
				strcpy( vmimeType, resMIME );
			else
				strcpy( vmimeType, mimeType );
			break;
		}
		parentCount++;
		
	}while( (status = resourcePath.GetParent( &resourcePath )) == B_OK );
	entry.Unset();
	if( node.InitCheck() )
		node.Unset();
	// *****
	// Look for Virtual Resource if no "real" resource was found.
	// *****
	
	if( (status != B_OK)||((parentCount != 0)&&(strcmp(mimeType, "application/x-vnd.Be-directory") == 0)) )
	{
		const char *resMIME;
		if( (resMIME = pb->vresources->MatchVRes( pb->brURI->path, false, &vres )) )
		{
			strcpy( vmimeType, resMIME );
			strcpy( mimeType, resMIME );
		}
		else
		{
			HTTPResponse	response;
			response.SetHTMLMessage( 404 ); // Not Found
			pb->request->SendReply( &response );
			return B_ERROR;
		}
	}
	
	// *****
	// Find handler module for resource
	// *****
	
	HModule		*module, *prefModule = NULL;
	int32		priority, highestPriority = 0;
	for( int32 i=0; (module = (HModule *)moduleList.ItemAt(i)); i++ )
	{
		if( module->CanHandleResource( vmimeType, pb->request->GetMethod(), &priority )&&
			(priority > highestPriority) )
		{
			highestPriority = priority;
			prefModule = module;
		}
	}
	
	// *****
	// Setup PB
	// *****
	pb->HandleRequest = HModuleRoster::HandleRequest;
	pb->Logprintf = log_printf;
	pb->moduleList = &moduleList;
	if( vres )
	{
		pb->authenticate = vres->Authenticate();
		pb->extras = &vres->extras;
	}
	else
		pb->extras = NULL;
	
	// *****
	// Invoke Handler Module to handle the request
	// *****
	if( highestPriority > 0 )
	{
		status = prefModule->HandleRequest( pb );
		return status;
	}
	else // No handler found... send error
	{
		HTTPResponse	response;
		response.SetHTMLMessage( 501 ); // Not Implemented
		pb->request->SendReply( &response );
		return B_ERROR;
	}
	return B_OK;
}

status_t HModuleRoster::LoadModules( BPath *moduleDirectory )
{
	status_t		status;
	BDirectory		dir;
	
	if( (status = dir.SetTo( moduleDirectory->Path() )) == B_OK )
	{
		BEntry		entry;
		BPath		modulePath;
		
		while( dir.GetNextEntry( &entry, true ) != B_ENTRY_NOT_FOUND )
		{
			entry.GetPath( &modulePath );
			LoadModule( &modulePath );
		}
	}
	else
		return status;
	return B_OK;
}

status_t HModuleRoster::UnloadAllModules( void )
{
	HModule		*module;
	for( int32 i=0; (module = (HModule *)moduleList.ItemAt(i)); i++ )
		delete module;
	moduleList.MakeEmpty();
	return B_OK;
}

status_t HModuleRoster::LoadModule( BPath *modulePath )
{
	BNode		node;
		
	if( node.SetTo( modulePath->Path() ) == B_OK )
	{
		HModule		*module = new HModule;
		if( module->LoadModule( modulePath ) == B_OK )
		{
			moduleList.AddItem( module );
			return B_OK;
		}
		else
		{
			delete module;
			return B_ERROR;
		}
	}
	else
		return B_ERROR;
	return B_OK;
}

status_t HModuleRoster::UnloadModule( const char *moduleName )
{
	HModule		*module;
	for( int32 i=0; (module = (HModule *)moduleList.ItemAt(i)); i++ )
	{
		if( strcmp( moduleName, module->GetName() ) == 0 )
		{
			moduleList.RemoveItem( module );
			delete module;
			return B_OK;
		}
	}
	return B_ERROR;
}

status_t HModuleRoster::UnloadModule( node_ref *nref )
{
	HModule		*module;
	node_ref	moduleRef;
	
	for( int32 i=0; (module = (HModule *)moduleList.ItemAt(i)); i++ )
	{
		module->GetNodeRef( &moduleRef );
		if( moduleRef == *nref )
		{
			moduleList.RemoveItem( module );
			delete module;
			return B_OK;
		}
	}
	return B_ERROR;
}
