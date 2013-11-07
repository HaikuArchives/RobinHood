#ifndef VRESOURCE_H
#define VRESOURCE_H

#include <List.h>
#include <StorageDefs.h>
#include "FieldList.h"

class VResource : public BList
{
	public:
		VResource( void );
		virtual ~VResource( void );
		
		void SetMimeType( const char *mimeType );
		void SetReal( bool real );
		void SetAuthenticate( bool authenticate );
		void AddPattern( const char *pattern );
		void AddExtra( const char *fieldName, const char *fieldValue );
		
		bool Match( const char *path, bool real );
		
		inline const char *GetMimeType( void )  { return mimeType; };
		inline bool Authenticate() { return authenticate; };
		inline bool GetReal() { return redirect; };
	
	public:
		FieldList		extras; // Extra fields passed to handler module
		
	protected:
		char mimeType[B_MIME_TYPE_LENGTH];
		bool redirect;
		bool authenticate;
};

class VResList : public BList
{
	public:
		VResList( void );
		virtual ~VResList( void );
		
		void AddVRes( VResource *vres );
		const char *MatchVRes( const char *path, bool real, VResource **vres = NULL );
};

#endif