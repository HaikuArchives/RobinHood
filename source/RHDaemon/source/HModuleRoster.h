#ifndef HMODULEROSTER_H
#define HMODULEROSTER_H

#include <Handler.h>
#include <List.h>
#include <Node.h>
#include <Path.h>
#include "RequestPB.h"

class HModuleRoster : public BHandler
{
	public:
		HModuleRoster( void );
		virtual ~HModuleRoster( void );
		
		status_t StartWatching( BPath *moduleDirectory );
		status_t StopWatching( void );
		
		virtual void MessageReceived( BMessage *message );
		static status_t HandleRequest( RequestPB *pb );
		
		status_t LoadModules( BPath *moduleDirectory );
		status_t UnloadAllModules( void );
		
		status_t LoadModule( BPath *modulePath );
		status_t UnloadModule( const char *moduleName );
		status_t UnloadModule( node_ref *nref );
		
	protected:
		static BList		moduleList;
		node_ref			watchedRef;
		bool				watching;
		
};

extern HModuleRoster *hmodule_roster;

#endif