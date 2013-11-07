#ifndef HMODULE_H
#define HMODULE_H

#include <OS.h>
#include <image.h>
#include <Node.h>
#include "RequestPB.h"

class HModule
{
	public:
		HModule( void );
		~HModule( void );
		
		status_t LoadModule( BPath *path );
		status_t UnloadModule( void );
		status_t HandleRequest( RequestPB *pb );
		bool CanHandleResource( const char *mimeType, http_method method, int32 *priority );
		
		const char *GetName( void );
		void GetNodeRef( node_ref *nref );
		
	protected:
		char		*name;
		node_ref	moduleRef;
		
		status_t (*handle_request)( RequestPB *pb );
		bool (*can_handle_resource)( const char *mimeType, http_method method, int32 *priority );
		image_id	image;
		
		thread_id	unloadT;
		int32		atomic;
		bool		available;
};

#endif